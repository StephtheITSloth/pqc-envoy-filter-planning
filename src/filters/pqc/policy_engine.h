// Policy Engine - Routing Decision Logic
//
// Evaluates policies to determine routing decisions based on PQC context.

#pragma once

#include "src/config/policy.pb.h"
#include "src/filters/pqc/context.h"

#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

/**
 * Policy engine for PQC routing decisions.
 */
class PolicyEngine {
public:
  explicit PolicyEngine(const pqc::envoy::config::v1::PolicyConfig& config);
  
  /**
   * Evaluate policy for a given context and route.
   * 
   * @param context PQC connection context
   * @param route Route pattern
   * @return Policy decision
   */
  absl::StatusOr<pqc::envoy::config::v1::PolicyDecision> 
  evaluate(const PqcContext& context, absl::string_view route) const;

private:
  pqc::envoy::config::v1::PolicyConfig config_;
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
