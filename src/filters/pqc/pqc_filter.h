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

#include <memory>
#include <string>

#include "envoy/http/filter.h"
#include "envoy/server/filter_config.h"

#include "src/config/pqc_filter.pb.h"

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
class PqcFilter : public Http::StreamDecoderFilter {
public:
  explicit PqcFilter(PqcFilterConfigSharedPtr config);
  
  // Http::StreamDecoderFilter interface
  Http::FilterHeadersStatus decodeHeaders(Http::RequestHeaderMap& headers, 
                                          bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance& data, bool end_stream) override;
  Http::FilterTrailersStatus decodeTrailers(Http::RequestTrailerMap& trailers) override;
  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;
  
  // Filter lifecycle
  void onDestroy() override;

private:
  // Extract PQC context from the downstream connection
  // TODO: Implement in later step
  void extractPqcContext();
  
  // Annotate request headers with PQC metadata
  // TODO: Implement in later step
  void annotatePqcHeaders(Http::RequestHeaderMap& headers);
  
  // Record metrics for this request
  // TODO: Implement in observability phase
  void recordMetrics();
  
  PqcFilterConfigSharedPtr config_;
  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{nullptr};
  
  // TODO: Add PqcContext member in later step
  // TODO: Add PolicyEngine member in later step
};

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
