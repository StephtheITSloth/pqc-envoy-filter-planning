// Configuration Validator Implementation

#include "src/config/validator.h"

#include "absl/strings/str_cat.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace Config {

absl::Status validateConfig(const pqc::envoy::config::v1::PqcFilter& config) {
  // Validate mode
  auto mode_status = validateMode(config.mode());
  if (!mode_status.ok()) {
    return mode_status;
  }
  
  // TODO: Add more validation rules in later steps
  // - Validate policy configuration
  // - Validate feature flags
  // - Validate fallback configuration
  
  return absl::OkStatus();
}

absl::Status validateMode(pqc::envoy::config::v1::PqcFilter::Mode mode) {
  switch (mode) {
    case pqc::envoy::config::v1::PqcFilter::MODE_UNSPECIFIED:
      return absl::InvalidArgumentError("Mode must be specified");
    case pqc::envoy::config::v1::PqcFilter::CLASSICAL:
    case pqc::envoy::config::v1::PqcFilter::HYBRID:
    case pqc::envoy::config::v1::PqcFilter::PQC_ONLY:
      return absl::OkStatus();
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid mode: ", static_cast<int>(mode)));
  }
}

} // namespace Config
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
