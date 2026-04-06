# Project Documentation Rules (Non-Obvious Only)

## Documentation Structure

- Architecture docs in [`docs/architecture/`](../../docs/architecture/) use ADR (Architecture Decision Record) format - numbered sequentially
- API documentation generated from proto comments - use `///` not `//` for proto field docs
- Runbooks in [`docs/runbooks/`](../../docs/runbooks/) follow incident response template - missing sections cause review failures
- Code examples in docs MUST be tested via [`bazel test //docs:example_tests`](../../docs/example_tests.cc) - untested examples break CI

## Non-Obvious Code Organization

- [`src/filters/pqc/`](../../src/filters/pqc/) contains HTTP filter, [`src/filters/pqc_network/`](../../src/filters/pqc_network/) contains L4 filter - NOT in same directory
- Crypto implementations split between [`src/crypto/`](../../src/crypto/) (interface) and [`src/crypto/impl/`](../../src/crypto/impl/) (BoringSSL-specific) - abstraction layer required
- Config validation logic in [`src/config/validator.h`](../../src/config/validator.h), NOT in proto definitions - protos are just schema
- Test utilities in [`test/common/`](../../test/common/) are shared across unit and integration tests - duplication causes build errors

## Hidden Dependencies

- Filter depends on [`@envoy//source/extensions/transport_sockets/tls`](../../WORKSPACE) - not obvious from include paths
- Metrics depend on [`@com_github_jupp0r_prometheus_cpp`](../../WORKSPACE) - Envoy's built-in stats are insufficient for PQC
- Feature flags depend on external service - local development requires mock service in [`test/mocks/feature_flags.h`](../../test/mocks/feature_flags.h)

## Counterintuitive Patterns

- [`PqcFilter::onDestroy()`](../../src/filters/pqc/pqc_filter.h) is called BEFORE connection closes - use for cleanup, not final logging
- Config updates via xDS trigger filter recreation - state must be persisted in [`PqcContext`](../../src/filters/pqc/context.h)
- Metrics are per-worker-thread - aggregation happens in Prometheus, not in filter code
- Tracing spans created in filter are NOT automatically closed - must call [`span->finishSpan()`](../../src/filters/pqc/tracing.h)

## Testing Context

- Unit tests in [`test/filters/pqc/`](../../test/filters/pqc/) use mock Envoy APIs - real Envoy not started
- Integration tests in [`test/integration/`](../../test/integration/) start full Envoy instance - slower but more realistic
- Crypto tests in [`test/crypto/`](../../test/crypto/) use deterministic RNG - non-deterministic tests fail in CI
- Load tests in [`test/load/`](../../test/load/) require external Locust installation - not run in CI

## Configuration Gotchas

- Bootstrap config in [`config/bootstrap.yaml`](../../config/bootstrap.yaml) is for local dev - production uses Helm-generated config
- Feature flag config in [`config/feature_flags.yaml`](../../config/feature_flags.yaml) is environment-specific - dev/stage/prod have different files
- Cipher suite list in [`config/cipher_suites.yaml`](../../config/cipher_suites.yaml) must match BoringSSL supported suites - mismatches cause runtime errors
- Metrics config in [`config/metrics.yaml`](../../config/metrics.yaml) defines histogram buckets - changing requires Prometheus config update

## Observability Context

- Prometheus metrics scraped from `:9901/stats/prometheus` - NOT standard `:9090` port
- Grafana dashboards in [`dashboards/`](../../dashboards/) use Prometheus datasource named `pqc-prometheus` - different name breaks queries
- Structured logs output to stdout in JSON format - log aggregation expects specific schema in [`src/filters/pqc/logger.h`](../../src/filters/pqc/logger.h)
- Tracing uses Jaeger protocol - OpenTelemetry support planned but not implemented

## Security Context

- PQC keys stored in [`/etc/pqc/keys/`](../../config/keys/) in production - local dev uses [`test/crypto/test_keys/`](../../test/crypto/test_keys/)
- Certificate validation uses custom logic in [`src/crypto/cert_validator.h`](../../src/crypto/cert_validator.h) - standard X.509 validation insufficient for PQC
- Downgrade attack protection requires checking `pqc.negotiated` header - missing check allows classical fallback
- Audit logs written to separate file in [`/var/log/pqc/audit.log`](../../config/logging/) - NOT mixed with application logs

## Build Context

- Bazel WORKSPACE defines `@envoy` repository pointing to specific commit - NOT latest Envoy release
- BoringSSL PQC fork at `@boringssl_pqc` has different API than standard BoringSSL - wrapper layer in [`src/crypto/boringssl_wrapper.h`](../../src/crypto/boringssl_wrapper.h)
- Proto definitions in [`src/config/`](../../src/config/) generate C++ code in `bazel-bin/` - NOT in source tree
- Test data files in [`test/data/`](../../test/data/) are embedded in binary via Bazel `data` attribute - runtime file access fails without it