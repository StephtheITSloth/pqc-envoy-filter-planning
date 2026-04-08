// Logger - Structured Logging for PQC

#pragma once

#include "src/filters/pqc/context.h"

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

class Logger {
public:
  Logger() = default;
  
  // TODO: Implement structured logging
  void logPqcConnection(const PqcContext& context) const {}
  void logPolicyDecision(absl::string_view decision) const {}
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
