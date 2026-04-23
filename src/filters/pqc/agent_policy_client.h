// Agent Policy Client - MVP policy decision seam
//
// This abstraction keeps the filter wired to a compact policy request/response
// shape so we can start with a local evaluator and later swap in a Watsonx-
// backed decision service without rewriting the filter decode path.

#pragma once

#include <memory>
#include <string>

#include "src/config/policy.pb.h"
#include "src/config/pqc_filter.pb.h"
#include "src/filters/pqc/context.h"
#include "src/filters/pqc/policy_engine.h"

#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

struct AgentPolicyRequest {
  std::string service_name;
  std::string request_method;
  std::string request_path;
  std::string authority;
  std::string client_ip;
  std::string sni;
  std::string tls_version;
  std::string cipher_suite;
  bool pqc_capable{false};
  pqc::envoy::config::v1::PqcFilter::Mode mode{
      pqc::envoy::config::v1::PqcFilter::MODE_UNSPECIFIED};
};

AgentPolicyRequest buildAgentPolicyRequest(
    const PqcContext& context, pqc::envoy::config::v1::PqcFilter::Mode mode);

class AgentPolicyClient {
public:
  virtual ~AgentPolicyClient() = default;

  virtual absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
  evaluate(const AgentPolicyRequest& request) const = 0;
};

using AgentPolicyClientSharedPtr = std::shared_ptr<const AgentPolicyClient>;

class LocalAgentPolicyClient : public AgentPolicyClient {
public:
  explicit LocalAgentPolicyClient(const pqc::envoy::config::v1::PolicyConfig& config);

  absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
  evaluate(const AgentPolicyRequest& request) const override;

private:
  PolicyEngine policy_engine_;
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

