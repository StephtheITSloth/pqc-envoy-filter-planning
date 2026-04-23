// PQC HTTP Filter - Implementation
//
// This file implements the PQC HTTP filter logic, including:
// 1. PQC request context extraction from downstream connections
// 2. Request annotation with PQC metadata
// 3. Local policy evaluation for fail-open fallback
// 4. Async external policy-service integration for the Watsonx-ready MVP

#include "src/filters/pqc/pqc_filter.h"

#include "envoy/http/codes.h"
#include "envoy/network/connection.h"
#include "envoy/ssl/connection.h"

#include "source/common/http/header_map_impl.h"
#include "source/common/http/message_impl.h"
#include "source/common/http/utility.h"
#include "source/common/json/json_loader.h"
#include "source/common/protobuf/utility.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

namespace {

constexpr absl::string_view kDefaultPolicyServicePath = "/v1/policy/evaluate";
constexpr std::chrono::milliseconds kDefaultPolicyServiceTimeout{1000};

bool isPqcCipherSuite(absl::string_view cipher) {
  return cipher.find("ML_KEM") != absl::string_view::npos ||
         cipher.find("ML_DSA") != absl::string_view::npos ||
         cipher.find("SLH_DSA") != absl::string_view::npos;
}

std::string normalizeHostLikeValue(absl::string_view value) {
  if (value.empty()) {
    return "";
  }

  // IPv6 authorities are formatted like [::1]:443. Keep the address only.
  if (value.front() == '[') {
    const size_t closing_bracket = value.find(']');
    if (closing_bracket != absl::string_view::npos) {
      return std::string(value.substr(1, closing_bracket - 1));
    }
  }

  const size_t colon = value.find(':');
  if (colon == absl::string_view::npos) {
    return std::string(value);
  }

  return std::string(value.substr(0, colon));
}

absl::string_view policyActionToHeaderValue(pqc::envoy::config::v1::PolicyAction action) {
  switch (action) {
  case pqc::envoy::config::v1::ALLOW:
    return "allow";
  case pqc::envoy::config::v1::DENY:
    return "deny";
  case pqc::envoy::config::v1::FALLBACK:
    return "fallback";
  case pqc::envoy::config::v1::PREFER_HYBRID:
    return "prefer_hybrid";
  case pqc::envoy::config::v1::ACTION_UNSPECIFIED:
  default:
    return "unspecified";
  }
}

absl::string_view modeToPolicyServiceValue(pqc::envoy::config::v1::PqcFilter::Mode mode) {
  switch (mode) {
  case pqc::envoy::config::v1::PqcFilter::CLASSICAL:
    return "classical";
  case pqc::envoy::config::v1::PqcFilter::HYBRID:
    return "hybrid";
  case pqc::envoy::config::v1::PqcFilter::PQC_ONLY:
    return "pqc_only";
  case pqc::envoy::config::v1::PqcFilter::MODE_UNSPECIFIED:
  default:
    return "classical";
  }
}

std::string stringOrDefault(const std::string& value, absl::string_view default_value) {
  return value.empty() ? std::string(default_value) : value;
}

std::string appendFallbackReason(const std::string& reason, absl::string_view failure_reason) {
  if (failure_reason.empty()) {
    return reason;
  }
  if (reason.empty()) {
    return absl::StrCat("Local fallback after policy service failure: ", failure_reason);
  }
  return absl::StrCat(reason, " (local fallback after policy service failure: ", failure_reason,
                      ")");
}

absl::StatusOr<std::string> getJsonStringField(const Json::Object& json,
                                               absl::string_view field_name) {
  auto value_or = json.getValue(std::string(field_name));
  if (!value_or.ok()) {
    return value_or.status();
  }

  const auto* string_value = absl::get_if<std::string>(&value_or.value());
  if (string_value == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("Policy service field '", field_name, "' must be a string"));
  }

  return *string_value;
}

absl::StatusOr<pqc::envoy::config::v1::PolicyAction>
policyActionFromServiceValue(absl::string_view action) {
  if (action == "allow") {
    return pqc::envoy::config::v1::ALLOW;
  }
  if (action == "prefer_hybrid") {
    return pqc::envoy::config::v1::PREFER_HYBRID;
  }
  if (action == "fallback") {
    return pqc::envoy::config::v1::FALLBACK;
  }
  if (action == "deny") {
    return pqc::envoy::config::v1::DENY;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("Unsupported policy service action: ", action));
}

} // namespace

// PqcFilterConfig implementation

PqcFilterConfig::PqcFilterConfig(const pqc::envoy::config::v1::PqcFilter& proto_config)
    : config_(proto_config) {
  // TODO: Validate configuration in later step
  // TODO: Initialize policy engine in later step
}

bool PqcFilterConfig::isEnabled() const {
  // If enabled field is not set, default to true
  if (!config_.has_enabled()) {
    return true;
  }
  return config_.enabled().value();
}

bool PqcFilterConfig::policyServiceFailOpen() const {
  if (!config_.has_policy_service() || !config_.policy_service().has_fail_open()) {
    return true;
  }
  return config_.policy_service().fail_open().value();
}

std::chrono::milliseconds PqcFilterConfig::policyServiceTimeout() const {
  if (!config_.has_policy_service() || !config_.policy_service().has_timeout()) {
    return kDefaultPolicyServiceTimeout;
  }
  return std::chrono::milliseconds(
      DurationUtil::durationToMilliseconds(config_.policy_service().timeout()));
}

std::string PqcFilterConfig::policyServicePath() const {
  if (!config_.has_policy_service() || config_.policy_service().path().empty()) {
    return std::string(kDefaultPolicyServicePath);
  }
  return config_.policy_service().path();
}

std::string PqcFilterConfig::policyServiceAuthority() const {
  if (!config_.has_policy_service()) {
    return "";
  }
  return config_.policy_service().authority();
}

// PqcFilter implementation

PqcFilter::PqcFilter(PqcFilterConfigSharedPtr config, Upstream::ClusterManager* cluster_manager)
    : config_(std::move(config)), cluster_manager_(cluster_manager),
      context_(std::make_shared<PqcContext>()),
      policy_client_(std::make_shared<LocalAgentPolicyClient>(config_->policyConfig())) {
  // TODO: Initialize metrics in observability phase
}

Http::FilterHeadersStatus PqcFilter::decodeHeaders(Http::RequestHeaderMap& headers,
                                                   bool) {
  current_decision_ = absl::nullopt;
  destroyed_ = false;
  clearPendingPolicyRequestState();
  decoding_headers_ = true;
  inline_headers_status_ = absl::nullopt;

  const auto finish_decode = [this](Http::FilterHeadersStatus default_status) {
    decoding_headers_ = false;
    if (inline_headers_status_.has_value()) {
      const auto status = inline_headers_status_.value();
      inline_headers_status_ = absl::nullopt;
      return status;
    }
    return default_status;
  };

  // If filter is disabled, pass through
  if (!config_->isEnabled()) {
    return finish_decode(Http::FilterHeadersStatus::Continue);
  }

  extractPqcContext(headers);

  if (config_->hasPolicyService()) {
    return finish_decode(dispatchPolicyServiceRequest(headers));
  }

  return finish_decode(resolveLocalPolicy(headers));
}

Http::FilterDataStatus PqcFilter::decodeData(Buffer::Instance&, bool) {
  if (waiting_for_policy_) {
    return Http::FilterDataStatus::StopIterationAndBuffer;
  }
  return Http::FilterDataStatus::Continue;
}

Http::FilterTrailersStatus PqcFilter::decodeTrailers(Http::RequestTrailerMap&) {
  if (waiting_for_policy_) {
    return Http::FilterTrailersStatus::StopIteration;
  }
  return Http::FilterTrailersStatus::Continue;
}

void PqcFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

void PqcFilter::onDestroy() {
  destroyed_ = true;
  clearPendingPolicyRequestState();
}

void PqcFilter::onSuccess(const Http::AsyncClient::Request&,
                          Http::ResponseMessagePtr&& response) {
  active_policy_request_ = nullptr;

  if (destroyed_ || pending_headers_ == nullptr) {
    clearPendingPolicyRequestState();
    return;
  }

  auto decision_or = parsePolicyServiceResponse(*response);
  if (!decision_or.ok()) {
    handlePolicyServiceFailure(decision_or.status().message());
    return;
  }

  finalizeAsyncDecision(decision_or.value());
}

void PqcFilter::onFailure(const Http::AsyncClient::Request&,
                          Http::AsyncClient::FailureReason reason) {
  active_policy_request_ = nullptr;

  if (destroyed_ || pending_headers_ == nullptr) {
    clearPendingPolicyRequestState();
    return;
  }

  handlePolicyServiceFailure(
      reason == Http::AsyncClient::FailureReason::Reset
          ? "Policy service request reset"
          : "Policy service request failed");
}

void PqcFilter::extractPqcContext(const Http::RequestHeaderMap& headers) {
  // Recreate the context per request to avoid stale values leaking across retries or re-use.
  context_ = std::make_shared<PqcContext>();

  if (!headers.getMethodValue().empty()) {
    context_->setRequestMethod(headers.getMethodValue());
  }
  if (!headers.getPathValue().empty()) {
    context_->setRequestPath(headers.getPathValue());
  }
  if (!headers.getHostValue().empty()) {
    context_->setAuthority(headers.getHostValue());

    const std::string service_name = normalizeHostLikeValue(headers.getHostValue());
    if (!service_name.empty()) {
      context_->setServiceName(service_name);
    }
  }

  if (!decoder_callbacks_) {
    return;
  }

  // Get downstream connection
  auto connection = decoder_callbacks_->connection();
  if (!connection.has_value()) {
    return;
  }

  if (connection->connectionInfoProvider().remoteAddress()) {
    context_->setClientIp(connection->connectionInfoProvider().remoteAddress()->asString());
  }

  // Check if connection has TLS
  const auto* ssl = connection->ssl();
  if (!ssl) {
    context_->setPqcCapable(false);
    return;
  }

  // Extract cipher suite information
  const std::string cipher = ssl->ciphersuiteString();
  if (!cipher.empty()) {
    context_->setNegotiatedCipher(cipher);
  }

  // Extract TLS version
  const std::string tls_version = ssl->tlsVersion();
  if (!tls_version.empty()) {
    context_->setTlsVersion(tls_version);
  }

  // Extract SNI if available
  if (!ssl->sni().empty()) {
    context_->setSni(ssl->sni());

    if (!context_->serviceName().has_value()) {
      const std::string service_name = normalizeHostLikeValue(ssl->sni());
      if (!service_name.empty()) {
        context_->setServiceName(service_name);
      }
    }
  }

  context_->setPqcCapable(isPqcCipherSuite(cipher));
}

absl::StatusOr<pqc::envoy::config::v1::PolicyDecision> PqcFilter::evaluatePolicyDecision() const {
  if (!policy_client_ || !context_) {
    return absl::InternalError("Policy client is not initialized");
  }

  return policy_client_->evaluate(buildAgentPolicyRequest(*context_, config_->mode()));
}

Http::FilterHeadersStatus PqcFilter::resolveLocalPolicy(Http::RequestHeaderMap& headers,
                                                        absl::string_view fallback_reason) {
  auto decision_or = evaluatePolicyDecision();
  if (decision_or.ok()) {
    current_decision_ = decision_or.value();
    current_decision_->set_reason(
        appendFallbackReason(current_decision_->reason(), fallback_reason));

    const auto apply_status = applyPolicyDecision(current_decision_.value());
    if (apply_status != Http::FilterHeadersStatus::Continue) {
      return apply_status;
    }
  } else {
    current_decision_ = pqc::envoy::config::v1::PolicyDecision();
    current_decision_->set_action(pqc::envoy::config::v1::ALLOW);
    if (fallback_reason.empty()) {
      current_decision_->set_reason("Policy evaluation failed open");
    } else {
      current_decision_->set_reason(
          absl::StrCat("Policy service fallback failed open: ", fallback_reason,
                       "; local policy error: ", decision_or.status().message()));
    }
  }

  annotatePqcHeaders(headers);
  recordMetrics();
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterHeadersStatus PqcFilter::dispatchPolicyServiceRequest(Http::RequestHeaderMap& headers) {
  const auto* policy_service = config_->policyService();
  if (policy_service == nullptr) {
    return resolveLocalPolicy(headers);
  }

  auto fail_with_reason = [&](absl::string_view reason) {
    if (config_->policyServiceFailOpen()) {
      return resolveLocalPolicy(headers, reason);
    }

    if (decoder_callbacks_ != nullptr) {
      decoder_callbacks_->sendLocalReply(Http::Code::ServiceUnavailable,
                                         "PQC policy service unavailable", nullptr,
                                         absl::nullopt, "pqc.policy.unavailable");
    }
    return Http::FilterHeadersStatus::StopIteration;
  };

  if (cluster_manager_ == nullptr) {
    return fail_with_reason("Policy service is configured without a cluster manager");
  }

  auto* thread_local_cluster = cluster_manager_->getThreadLocalCluster(policy_service->cluster());
  if (thread_local_cluster == nullptr) {
    return fail_with_reason(
        absl::StrCat("Policy service cluster is missing: ", policy_service->cluster()));
  }

  pending_headers_ = &headers;
  waiting_for_policy_ = true;

  Http::AsyncClient::RequestOptions options;
  options.setTimeout(config_->policyServiceTimeout()).setSendXff(false);

  const AgentPolicyRequest request_context =
      buildAgentPolicyRequest(*context_, config_->mode());
  active_policy_request_ =
      thread_local_cluster->httpAsyncClient().send(buildPolicyServiceRequest(request_context),
                                                   *this, options);

  if (active_policy_request_ == nullptr && waiting_for_policy_) {
    clearPendingPolicyRequestState();
    return fail_with_reason("Policy service request could not be dispatched");
  }

  if (inline_headers_status_.has_value()) {
    return inline_headers_status_.value();
  }

  return waiting_for_policy_ ? Http::FilterHeadersStatus::StopIteration
                             : Http::FilterHeadersStatus::Continue;
}

Http::RequestMessagePtr
PqcFilter::buildPolicyServiceRequest(const AgentPolicyRequest& request) const {
  ProtobufWkt::Struct payload;
  auto& fields = *payload.mutable_fields();
  fields["service_name"] =
      ValueUtil::stringValue(stringOrDefault(request.service_name, "unknown-service"));
  fields["request_method"] =
      ValueUtil::stringValue(stringOrDefault(request.request_method, "GET"));
  fields["request_path"] = ValueUtil::stringValue(stringOrDefault(request.request_path, "/"));
  fields["mode"] = ValueUtil::stringValue(std::string(modeToPolicyServiceValue(request.mode)));
  fields["pqc_capable"] = ValueUtil::boolValue(request.pqc_capable);
  fields["authority"] = ValueUtil::stringValue(request.authority);
  fields["client_ip"] = ValueUtil::stringValue(request.client_ip);
  fields["sni"] = ValueUtil::stringValue(request.sni);
  fields["tls_version"] = ValueUtil::stringValue(request.tls_version);
  fields["cipher_suite"] = ValueUtil::stringValue(request.cipher_suite);

  const std::string body = MessageUtil::getJsonStringFromMessageOrError(payload, false, true);
  const std::string authority = !config_->policyServiceAuthority().empty()
                                    ? config_->policyServiceAuthority()
                                    : config_->policyService()->cluster();

  Http::RequestHeaderMapPtr request_headers =
      Http::createHeaderMap<Http::RequestHeaderMapImpl>(
          {{Http::Headers::get().Method, "POST"},
           {Http::Headers::get().Host, authority},
           {Http::Headers::get().Path, config_->policyServicePath()},
           {Http::Headers::get().ContentType, "application/json"},
           {Http::Headers::get().ContentLength, std::to_string(body.size())}});

  auto message = std::make_unique<Http::RequestMessageImpl>(std::move(request_headers));
  message->body().add(body);
  return message;
}

absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
PqcFilter::parsePolicyServiceResponse(const Http::ResponseMessage& response) const {
  const auto status = Http::Utility::getResponseStatusOrNullopt(response.headers());
  if (!status.has_value()) {
    return absl::InvalidArgumentError("Policy service response did not include an HTTP status");
  }
  if (status.value() != 200) {
    return absl::UnavailableError(
        absl::StrCat("Policy service returned HTTP ", status.value()));
  }

  const std::string body = response.bodyAsString();
  if (body.empty()) {
    return absl::InvalidArgumentError("Policy service response body was empty");
  }

  auto json_or = Json::Factory::loadFromStringNoThrow(body);
  if (!json_or.ok()) {
    return json_or.status();
  }

  auto action_or = getJsonStringField(*json_or.value(), "action");
  if (!action_or.ok()) {
    return action_or.status();
  }
  auto reason_or = getJsonStringField(*json_or.value(), "reason");
  if (!reason_or.ok()) {
    return reason_or.status();
  }
  auto action_enum_or = policyActionFromServiceValue(action_or.value());
  if (!action_enum_or.ok()) {
    return action_enum_or.status();
  }

  pqc::envoy::config::v1::PolicyDecision decision;
  decision.set_action(action_enum_or.value());
  decision.set_reason(reason_or.value());
  return decision;
}

void PqcFilter::finalizeAsyncDecision(
    const pqc::envoy::config::v1::PolicyDecision& decision) {
  if (pending_headers_ == nullptr) {
    clearPendingPolicyRequestState();
    return;
  }

  current_decision_ = decision;
  const auto apply_status = applyPolicyDecision(current_decision_.value());
  auto* callbacks = decoder_callbacks_;

  if (apply_status == Http::FilterHeadersStatus::Continue) {
    annotatePqcHeaders(*pending_headers_);
  }
  recordMetrics();
  clearPendingPolicyRequestState();

  if (decoding_headers_) {
    inline_headers_status_ = apply_status;
    return;
  }

  if (apply_status == Http::FilterHeadersStatus::Continue && callbacks != nullptr) {
    callbacks->continueDecoding();
  }
}

void PqcFilter::handlePolicyServiceFailure(absl::string_view failure_reason) {
  if (pending_headers_ == nullptr) {
    clearPendingPolicyRequestState();
    return;
  }

  auto* callbacks = decoder_callbacks_;

  if (config_->policyServiceFailOpen()) {
    const auto apply_status = resolveLocalPolicy(*pending_headers_, failure_reason);
    clearPendingPolicyRequestState();

    if (decoding_headers_) {
      inline_headers_status_ = apply_status;
      return;
    }

    if (apply_status == Http::FilterHeadersStatus::Continue && callbacks != nullptr) {
      callbacks->continueDecoding();
    }
    return;
  }

  if (callbacks != nullptr) {
    callbacks->sendLocalReply(Http::Code::ServiceUnavailable,
                              "PQC policy service unavailable", nullptr,
                              absl::nullopt, "pqc.policy.unavailable");
  }
  clearPendingPolicyRequestState();

  if (decoding_headers_) {
    inline_headers_status_ = Http::FilterHeadersStatus::StopIteration;
  }
}

void PqcFilter::clearPendingPolicyRequestState() {
  if (active_policy_request_ != nullptr) {
    active_policy_request_->cancel();
    active_policy_request_ = nullptr;
  }
  pending_headers_ = nullptr;
  waiting_for_policy_ = false;
}

Http::FilterHeadersStatus
PqcFilter::applyPolicyDecision(const pqc::envoy::config::v1::PolicyDecision& decision) {
  switch (decision.action()) {
  case pqc::envoy::config::v1::FALLBACK:
    context_->setFallback(true);
    return Http::FilterHeadersStatus::Continue;
  case pqc::envoy::config::v1::DENY:
    if (decoder_callbacks_ != nullptr) {
      decoder_callbacks_->sendLocalReply(Http::Code::Forbidden,
                                         "PQC policy denied the request", nullptr,
                                         absl::nullopt, "pqc.policy.denied");
    }
    return Http::FilterHeadersStatus::StopIteration;
  case pqc::envoy::config::v1::ALLOW:
  case pqc::envoy::config::v1::PREFER_HYBRID:
  case pqc::envoy::config::v1::ACTION_UNSPECIFIED:
  default:
    return Http::FilterHeadersStatus::Continue;
  }
}

void PqcFilter::annotatePqcHeaders(Http::RequestHeaderMap& headers) {
  if (!context_) {
    return;
  }

  // Add x-pqc-capable header
  headers.addCopy(Http::LowerCaseString("x-pqc-capable"),
                  context_->isPqcCapable() ? "true" : "false");

  // Add x-pqc-cipher header if available
  if (context_->negotiatedCipher().has_value()) {
    headers.addCopy(Http::LowerCaseString("x-pqc-cipher"),
                    context_->negotiatedCipher().value());
  }

  // Add x-pqc-tls-version header if available
  if (context_->tlsVersion().has_value()) {
    headers.addCopy(Http::LowerCaseString("x-pqc-tls-version"),
                    context_->tlsVersion().value());
  }

  if (context_->serviceName().has_value()) {
    headers.addCopy(Http::LowerCaseString("x-pqc-service"), context_->serviceName().value());
  }

  if (current_decision_.has_value()) {
    headers.addCopy(Http::LowerCaseString("x-pqc-decision"),
                    policyActionToHeaderValue(current_decision_->action()));
    if (!current_decision_->reason().empty()) {
      headers.addCopy(Http::LowerCaseString("x-pqc-decision-reason"),
                      current_decision_->reason());
    }
  }

  // Add x-pqc-fallback header if fallback occurred
  if (context_->hasFallback()) {
    headers.addCopy(Http::LowerCaseString("x-pqc-fallback"), "true");
  }

  // Add x-pqc-mode header based on configuration
  const char* mode_str = "unknown";
  switch (config_->mode()) {
  case pqc::envoy::config::v1::PqcFilter::CLASSICAL:
    mode_str = "classical";
    break;
  case pqc::envoy::config::v1::PqcFilter::HYBRID:
    mode_str = "hybrid";
    break;
  case pqc::envoy::config::v1::PqcFilter::PQC_ONLY:
    mode_str = "pqc_only";
    break;
  default:
    break;
  }
  headers.addCopy(Http::LowerCaseString("x-pqc-mode"), mode_str);
}

void PqcFilter::recordMetrics() {
  // TODO: Implement metrics recording
  // Record:
  // - pqc_connections_total
  // - pqc_handshake_duration
  // - pqc_policy_decisions
}

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
