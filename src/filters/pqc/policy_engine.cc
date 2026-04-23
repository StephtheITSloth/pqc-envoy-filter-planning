// Policy Engine Implementation

#include "src/filters/pqc/policy_engine.h"

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

namespace {

bool matchesRoutePattern(absl::string_view pattern, absl::string_view route) {
  if (pattern.empty()) {
    return false;
  }
  if (pattern == "*" || pattern == "/*") {
    return true;
  }

  const size_t wildcard = pattern.find('*');
  if (wildcard == absl::string_view::npos) {
    return pattern == route;
  }

  return wildcard == 0 || absl::StartsWith(route, pattern.substr(0, wildcard));
}

bool containsService(const google::protobuf::RepeatedPtrField<std::string>& services,
                     absl::string_view service_name) {
  for (const auto& service : services) {
    if (service == service_name) {
      return true;
    }
  }
  return false;
}

pqc::envoy::config::v1::PolicyDecision makeDecision(
    pqc::envoy::config::v1::PolicyAction action, absl::string_view reason,
    absl::optional<absl::string_view> matched_route = absl::nullopt) {
  pqc::envoy::config::v1::PolicyDecision decision;
  decision.set_action(action);
  decision.set_reason(std::string(reason));
  if (matched_route.has_value()) {
    decision.set_matched_route(std::string(matched_route.value()));
  }
  return decision;
}

} // namespace

PolicyEngine::PolicyEngine(const pqc::envoy::config::v1::PolicyConfig& config)
    : config_(config) {}

absl::StatusOr<pqc::envoy::config::v1::PolicyDecision>
PolicyEngine::evaluate(const PqcContext& context, absl::string_view route) const {
  const pqc::envoy::config::v1::RoutePolicy* best_match = nullptr;
  for (const auto& route_policy : config_.route_policies()) {
    if (!matchesRoutePattern(route_policy.route_pattern(), route)) {
      continue;
    }

    if (best_match == nullptr || route_policy.priority() > best_match->priority()) {
      best_match = &route_policy;
    }
  }

  if (best_match != nullptr) {
    const bool require_pqc =
        best_match->has_require_pqc() && best_match->require_pqc().value();
    const bool allow_fallback =
        best_match->has_allow_fallback() && best_match->allow_fallback().value();

    if (require_pqc && !context.isPqcCapable()) {
      return makeDecision(
          allow_fallback ? pqc::envoy::config::v1::FALLBACK
                         : pqc::envoy::config::v1::DENY,
          allow_fallback ? "Matched route requires PQC but allows fallback"
                         : "Matched route requires PQC",
          best_match->route_pattern());
    }

    if (best_match->action() != pqc::envoy::config::v1::ACTION_UNSPECIFIED) {
      return makeDecision(best_match->action(), "Matched explicit route policy",
                          best_match->route_pattern());
    }

    if (require_pqc && context.isPqcCapable()) {
      return makeDecision(pqc::envoy::config::v1::PREFER_HYBRID,
                          "Matched route requires PQC and downstream supports it",
                          best_match->route_pattern());
    }
  }

  if (context.serviceName().has_value()) {
    const absl::string_view service_name = context.serviceName().value();
    const bool require_pqc =
        containsService(config_.require_pqc_services(), service_name);
    const bool allow_fallback =
        containsService(config_.allow_fallback_services(), service_name);

    if (require_pqc) {
      if (context.isPqcCapable()) {
        return makeDecision(pqc::envoy::config::v1::PREFER_HYBRID,
                            absl::StrCat("Service ", service_name,
                                         " requires PQC and downstream supports it"));
      }

      if (allow_fallback) {
        return makeDecision(pqc::envoy::config::v1::FALLBACK,
                            absl::StrCat("Service ", service_name,
                                         " requires PQC but fallback is allowed"));
      }

      return makeDecision(pqc::envoy::config::v1::DENY,
                          absl::StrCat("Service ", service_name, " requires PQC"));
    }

    if (allow_fallback && !context.isPqcCapable()) {
      return makeDecision(pqc::envoy::config::v1::FALLBACK,
                          absl::StrCat("Service ", service_name,
                                       " allows fallback for classical clients"));
    }
  }

  if (config_.default_action() != pqc::envoy::config::v1::ACTION_UNSPECIFIED) {
    return makeDecision(config_.default_action(), "Default policy");
  }

  return makeDecision(pqc::envoy::config::v1::ACTION_UNSPECIFIED,
                      "No explicit policy matched");
}

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
