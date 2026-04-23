// Agent Policy Client - Local implementation

#include "src/filters/pqc/agent_policy_client.h"

#include "absl/strings/str_cat.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

namespace {

void maybeCopy(absl::optional<std::string> value, std::string& out) {
  if (value.has_value()) {
    out = value.value();
  }
}

PqcContext buildContext(const AgentPolicyRequest& request) {
  PqcContext context;
  context.setPqcCapable(request.pqc_capable);

  if (!request.cipher_suite.empty()) {
    context.setNegotiatedCipher(request.cipher_suite);
  }
  if (!request.tls_version.empty()) {
    context.setTlsVersion(request.tls_version);
  }
  if (!request.client_ip.empty()) {
    context.setClientIp(request.client_ip);
  }
  if (!request.sni.empty()) {
    context.setSni(request.sni);
  }
  if (!request.request_method.empty()) {
    context.setRequestMethod(request.request_method);
  }
  if (!request.request_path.empty()) {
    context.setRequestPath(request.request_path);
  }
  if (!request.authority.empty()) {
    context.setAuthority(request.authority);
  }
  if (!request.service_name.empty()) {
    context.setServiceName(request.service_name);
  }

  return context;
}

void setModeDefaultDecision(const AgentPolicyRequest& request,
                            pqc::envoy::config::v1::PolicyDecision& decision) {
  switch (request.mode) {
    case pqc::envoy::config::v1::PqcFilter::CLASSICAL:
      decision.set_action(pqc::envoy::config::v1::ALLOW);
      decision.set_reason("Classical mode allows the request");
      break;
    case pqc::envoy::config::v1::PqcFilter::HYBRID:
      if (request.pqc_capable) {
        decision.set_action(pqc::envoy::config::v1::PREFER_HYBRID);
        decision.set_reason("Hybrid mode prefers PQC-capable downstreams");
      } else {
        decision.set_action(pqc::envoy::config::v1::FALLBACK);
        decision.set_reason("Hybrid mode falls back for non-PQC downstreams");
      }
      break;
    case pqc::envoy::config::v1::PqcFilter::PQC_ONLY:
      if (request.pqc_capable) {
        decision.set_action(pqc::envoy::config::v1::ALLOW);
        decision.set_reason("PQC-only mode accepted a PQC-capable downstream");
      } else {
        decision.set_action(pqc::envoy::config::v1::DENY);
        decision.set_reason("PQC-only mode requires a PQC-capable downstream");
      }
      break;
    case pqc::envoy::config::v1::PqcFilter::MODE_UNSPECIFIED:
    default:
      decision.set_action(pqc::envoy::config::v1::ALLOW);
      decision.set_reason("No explicit mode was configured");
      break;
  }
}

void enforceModeGuardrails(const AgentPolicyRequest& request,
                           pqc::envoy::config::v1::PolicyDecision& decision) {
  if (request.mode == pqc::envoy::config::v1::PqcFilter::PQC_ONLY &&
      (!request.pqc_capable ||
       decision.action() == pqc::envoy::config::v1::FALLBACK)) {
    decision.set_action(pqc::envoy::config::v1::DENY);
    decision.set_reason("PQC-only mode rejected a non-PQC decision path");
    return;
  }

  if (request.mode == pqc::envoy::config::v1::PqcFilter::CLASSICAL &&
      decision.action() == pqc::envoy::config::v1::PREFER_HYBRID) {
    decision.set_action(pqc::envoy::config::v1::ALLOW);
    decision.set_reason("Classical mode downgraded hybrid preference to allow");
  }
}

} // namespace

AgentPolicyRequest buildAgentPolicyRequest(
    const PqcContext& context, pqc::envoy::config::v1::PqcFilter::Mode mode) {
  AgentPolicyRequest request;
  request.pqc_capable = context.isPqcCapable();
  request.mode = mode;

  maybeCopy(context.serviceName(), request.service_name);
  maybeCopy(context.requestMethod(), request.request_method);
  maybeCopy(context.requestPath(), request.request_path);
  maybeCopy(context.authority(), request.authority);
  maybeCopy(context.clientIp(), request.client_ip);
  maybeCopy(context.sni(), request.sni);
  maybeCopy(context.tlsVersion(), request.tls_version);
  maybeCopy(context.negotiatedCipher(), request.cipher_suite);

  return request;
}

LocalAgentPolicyClient::LocalAgentPolicyClient(
    const pqc::envoy::config::v1::PolicyConfig& config)
    : policy_engine_(config) {}

absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
LocalAgentPolicyClient::evaluate(const AgentPolicyRequest& request) const {
  PqcContext context = buildContext(request);
  auto decision_or = policy_engine_.evaluate(context, request.request_path);
  if (!decision_or.ok()) {
    return decision_or.status();
  }

  auto decision = decision_or.value();
  if (decision.action() == pqc::envoy::config::v1::ACTION_UNSPECIFIED) {
    setModeDefaultDecision(request, decision);
  } else {
    enforceModeGuardrails(request, decision);
  }

  if (decision.reason().empty()) {
    decision.set_reason(absl::StrCat("Evaluated policy for route ", request.request_path));
  }

  return decision;
}

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

