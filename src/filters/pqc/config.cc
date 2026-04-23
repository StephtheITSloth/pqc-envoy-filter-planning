// PQC Filter Factory and Registration
//
// This file registers the PQC filter with Envoy's filter factory system.
// It handles:
// 1. Proto configuration parsing
// 2. Filter instance creation
// 3. Registration with the HTTP filter registry
//
// Filter name: envoy.filters.http.pqc
//
// TODO: Add configuration validation in later step
// TODO: Add server factory context usage for stats/tracing

#include "src/filters/pqc/pqc_filter.h"

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"

#include "source/extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

/**
 * Config registration for the PQC filter.
 * This class is responsible for creating filter instances from proto configuration.
 */
class PqcFilterFactory
    : public Common::FactoryBase<pqc::envoy::config::v1::PqcFilter> {
public:
  PqcFilterFactory() : FactoryBase("envoy.filters.http.pqc") {}

private:
  // Create a filter instance from the proto configuration
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const pqc::envoy::config::v1::PqcFilter& proto_config,
      const std::string& stats_prefix,
      Server::Configuration::FactoryContext& context) override {

    // Create shared config object
    auto config = std::make_shared<PqcFilterConfig>(proto_config);
    auto* cluster_manager = &context.clusterManager();

    // TODO: Use context for stats scope in observability phase
    // TODO: Use context for tracing in observability phase
    // TODO: Validate configuration

    // Return factory callback that creates filter instances
    return [config, cluster_manager](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      // Create a new filter instance for each request
      callbacks.addStreamDecoderFilter(
          std::make_shared<PqcFilter>(config, cluster_manager));
    };
  }
};

/**
 * Static registration of the PQC filter factory.
 * This makes the filter available to Envoy's configuration system.
 */
REGISTER_FACTORY(PqcFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
