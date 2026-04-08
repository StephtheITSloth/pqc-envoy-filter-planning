// Fallback Manager - Handles PQC to Classical Fallback

#pragma once

#include "src/filters/pqc/context.h"

#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

struct FallbackResult {
  bool should_fallback;
  std::string reason;
};

class FallbackManager {
public:
  FallbackManager() = default;
  
  absl::StatusOr<FallbackResult> triggerFallback(const PqcContext& context) const;
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
