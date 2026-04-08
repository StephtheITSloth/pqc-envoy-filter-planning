// Configuration Validator for PQC Filter
//
// Validates proto configurations for correctness and consistency.

#pragma once

#include "src/config/pqc_filter.pb.h"

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace Config {

/**
 * Validates PQC filter configuration.
 * 
 * @param config The proto configuration to validate
 * @return Status indicating validation success or failure with details
 */
absl::Status validateConfig(const pqc::envoy::config::v1::PqcFilter& config);

/**
 * Validates that the mode is set correctly.
 * 
 * @param mode The mode to validate
 * @return Status indicating validation success or failure
 */
absl::Status validateMode(pqc::envoy::config::v1::PqcFilter::Mode mode);

} // namespace Config
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
