// Metrics - PQC Observability Metrics

#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

class Metrics {
public:
  Metrics() = default;
  
  // TODO: Implement metrics collection
  void recordConnection(absl::string_view mode) const {}
  void recordHandshakeDuration(uint64_t duration_ms) const {}
  void recordPolicyDecision(absl::string_view action) const {}
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
