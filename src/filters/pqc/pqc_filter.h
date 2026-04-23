// PQC HTTP Filter - Header
//
// This file defines the PQC (Post-Quantum Cryptography) HTTP filter that:
// 1. Inspects downstream TLS connections for PQC capability
// 2. Annotates requests with PQC metadata headers
// 3. Routes to PQC-capable upstreams based on configuration
//
// Filter name: envoy.filters.http.pqc
//
// TODO: Add PQC context extraction logic
// TODO: Add policy engine integration
// TODO: Add metrics collection
// TODO: Add distributed tracing support

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "envoy/http/async_client.h"
#include "envoy/http/filter.h"
#include "envoy/server/filter_config.h"
#include "envoy/upstream/cluster_manager.h"

#include "src/config/policy.pb.h"
#include "src/config/pqc_filter.pb.h"
#include "src/filters/pqc/agent_policy_client.h"
#include "src/filters/pqc/context.h"

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

/**
 * Configuration for the PQC HTTP filter.
 * Wraps the proto configuration and provides helper methods.
 */
class PqcFilterConfig {
public:
  explicit PqcFilterConfig(const pqc::envoy::config::v1::PqcFilter& proto_config);

  // Get the raw proto configuration
  const pqc::envoy::config::v1::PqcFilter& config() const { return config_; }

  // Check if the filter is enabled
  bool isEnabled() const;

  // Get the operating mode
  pqc::envoy::config::v1::PqcFilter::Mode mode() const { return config_.mode(); }

  // Get the local MVP policy configuration.
  const pqc::envoy::config::v1::PolicyConfig& policyConfig() const {
    return config_.policy();
  }

  bool hasPolicyService() const { return config_.has_policy_service(); }

  const pqc::envoy::config::v1::PolicyService* policyService() const {
    return config_.has_policy_service() ? &config_.policy_service() : nullptr;
  }

  bool policyServiceFailOpen() const;

  std::chrono::milliseconds policyServiceTimeout() const;

  std::string policyServicePath() const;

  std::string policyServiceAuthority() const;

private:
  pqc::envoy::config::v1::PqcFilter config_;
};

using PqcFilterConfigSharedPtr = std::shared_ptr<PqcFilterConfig>;

/**
 * PQC HTTP filter implementation.
 *
 * This filter:
 * - Extracts PQC capability from downstream TLS connection
 * - Annotates request headers with PQC metadata
 * - Makes routing decisions based on PQC policy
 *
 * The filter operates in the decode path and does not modify response data.
 */
class PqcFilter : public Http::StreamDecoderFilter,
                  public Http::AsyncClient::Callbacks {
public:
  explicit PqcFilter(PqcFilterConfigSharedPtr config,
                     Upstream::ClusterManager* cluster_manager = nullptr);

  // Http::StreamDecoderFilter interface
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers,
                                          bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap& trailers) override;
  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;

  // Filter lifecycle
  void onDestroy() override;

  // Http::AsyncClient::Callbacks interface
  void onSuccess(const Http::AsyncClient::Request& request,
                 Http::ResponseMessagePtr&& response) override;
  void onFailure(const Http::AsyncClient::Request& request,
                 Http::AsyncClient::FailureReason reason) override;
  void onBeforeFinalizeUpstreamSpan(Tracing::Span&,
                                    const Http::ResponseHeaderMap*) override {}

  // Exposes the last extracted decision context for tests and future agent adapters.
  const PqcContext& currentContext() const { return *context_; }
  const absl::optional<pqc::envoy::config::v1::PolicyDecision>& currentDecision() const {
    return current_decision_;
  }

private:
  // Extract PQC context from the downstream connection
  void extractPqcContext(const Http::RequestHeaderMap& headers);

  // Evaluate the per-request policy decision using the local client seam.
  absl::StatusOr<pqc::envoy::config::v1::PolicyDecision> evaluatePolicyDecision() const;

  // Evaluate the local policy and update request headers synchronously.
  Http::FilterHeadersStatus resolveLocalPolicy(Http::RequestHeaderMap& headers,
                                               absl::string_view fallback_reason = "");

  // Start an async policy service request for the current context.
  Http::FilterHeadersStatus dispatchPolicyServiceRequest(Http::RequestHeaderMap& headers);

  // Build an HTTP request to the external policy service.
  Http::RequestMessagePtr buildPolicyServiceRequest(const AgentPolicyRequest& request) const;

  // Parse a policy decision from the external service response.
  absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
  parsePolicyServiceResponse(const Http::ResponseMessage& response) const;

  // Finalize a resolved decision after the async service returns.
  void finalizeAsyncDecision(const pqc::envoy::config::v1::PolicyDecision& decision);

  // Handle an external policy service failure with fail-open or fail-closed behavior.
  void handlePolicyServiceFailure(absl::string_view failure_reason);

  // Reset all state related to an in-flight policy request.
  void clearPendingPolicyRequestState();

  // Apply a policy decision to the current request.
  Http::FilterHeadersStatus applyPolicyDecision(
      const pqc::envoy::config::v1::PolicyDecision& decision);

  // Annotate request headers with PQC metadata
  void annotatePqcHeaders(Http::RequestHeaderMap& headers);

  // Record metrics for this request
  // TODO: Implement in observability phase
  void recordMetrics();

  PqcFilterConfigSharedPtr config_;
  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{nullptr};
  Upstream::ClusterManager* cluster_manager_{nullptr};

  // PQC connection context
  PqcContextSharedPtr context_;

  AgentPolicyClientSharedPtr policy_client_;
  absl::optional<pqc::envoy::config::v1::PolicyDecision> current_decision_;
  Http::AsyncClient::Request* active_policy_request_{nullptr};
  Http::RequestHeaderMap* pending_headers_{nullptr};
  bool waiting_for_policy_{false};
  bool destroyed_{false};
  bool decoding_headers_{false};
  absl::optional<Http::FilterHeadersStatus> inline_headers_status_;

  // TODO: Replace the local client with a Watsonx-backed implementation.
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
