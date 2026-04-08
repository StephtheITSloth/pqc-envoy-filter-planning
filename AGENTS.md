# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Build & Test Commands

```bash
# Build entire project
bazel build //...

# Build specific target
bazel build //src/filters/pqc:pqc_filter

# Run all tests
bazel test //...

# Run specific test
bazel test //test/filters/pqc:pqc_filter_test

# Run single test case
bazel test //test/filters/pqc:pqc_filter_test --test_filter=PqcFilterTest.HandshakeNegotiation

# Build with debug symbols
bazel build -c dbg //src/filters/pqc:pqc_filter

# Run tests with verbose output
bazel test //... --test_output=all
```

## Docker Development

```bash
# Build development container
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev .

# Run development container
docker run -it -v $(pwd):/workspace pqc-envoy-dev bash

# Build production Envoy image
docker build -f docker/Dockerfile.envoy -t pqc-envoy:latest .

# Run with docker-compose
docker-compose -f docker/docker-compose.yml up
```

## Project-Specific Patterns

### Filter Implementation
- All PQC filters MUST inherit from [`Http::StreamDecoderFilter`](src/filters/pqc/pqc_filter.h) or [`Network::ReadFilter`](src/filters/pqc/pqc_network_filter.h)
- Use [`PqcContext`](src/filters/pqc/context.h) for connection state - do NOT store state in filter instance
- Cipher suite selection MUST go through [`PolicyEngine::evaluate()`](src/filters/pqc/policy_engine.h) - direct selection will bypass feature flags

### Configuration
- Proto definitions in [`src/config/`](src/config/) use custom validation - standard proto validation is insufficient
- Feature flags MUST be checked via [`FeatureFlagManager::isEnabled()`](src/config/feature_flags.h) - environment variables are ignored
- xDS config updates trigger [`onConfigUpdate()`](src/filters/pqc/pqc_filter.h) callback - implement for dynamic updates

### Crypto Integration
- PQC operations MUST use [`CryptoManager`](src/crypto/crypto_manager.h) wrapper - direct BoringSSL calls will fail in production
- Hybrid mode requires BOTH classical and PQC keys - missing either causes silent fallback
- Key rotation happens via [`KeyRotationCallback`](src/crypto/key_manager.h) - do NOT cache keys

### Testing
- Integration tests MUST run in [`test/integration/`](test/integration/) with full Envoy bootstrap
- Mock Envoy APIs available in [`test/mocks/`](test/mocks/) - use these instead of creating new mocks
- TDD workflow: Write test in [`test/filters/pqc/`](test/filters/pqc/) BEFORE implementation
- Crypto tests require [`test/crypto/test_keys/`](test/crypto/test_keys/) fixtures - do NOT generate keys in tests

### Observability
- Metrics MUST use [`Stats::Scope`](src/filters/pqc/metrics.h) from filter context - global stats are disabled
- Structured logs require [`ENVOY_LOG_TO_LOGGER`](src/filters/pqc/logger.h) macro with PQC-specific fields
- Tracing spans MUST include `pqc.cipher_suite` and `pqc.mode` tags for proper correlation

## Code Style

### C++ Conventions
- Use `absl::StatusOr<T>` for fallible operations - exceptions are disabled
- Prefer `absl::string_view` over `const std::string&` for read-only strings
- RAII wrappers in [`src/common/`](src/common/) for resource management - manual cleanup causes leaks
- Thread safety via `absl::Mutex` - Envoy's threading model requires this

### Naming
- Filter classes: `PqcHttpFilter`, `PqcNetworkFilter` (not `PQCFilter`)
- Config protos: `pqc.envoy.config.v1` namespace
- Metrics: `pqc.filter.handshake_duration` (not `pqc_handshake_duration`)
- Feature flags: `pqc.enabled.service_name` format

### Error Handling
- Network errors: Return `Network::FilterStatus::StopIteration` and log
- Config errors: Throw `EnvoyException` during initialization only
- Crypto errors: Trigger fallback via [`FallbackManager::triggerFallback()`](src/filters/pqc/fallback_manager.h)

## Multi-Agent Coordination

- **Plan Mode**: Can only edit `*.md` files - use for documentation and planning
- **Code Mode**: Full access to implementation - use for C++ and proto changes
- **Advanced Mode**: Includes MCP/Browser tools - use for config generation and external integrations

## Critical Gotchas

- Bazel WORKSPACE must include `@envoy` and `@boringssl_pqc` repositories - missing either breaks build
- Filter registration in [`src/filters/pqc/config.cc`](src/filters/pqc/config.cc) is REQUIRED - filter won't load without it
- Bootstrap config in [`test/integration/bootstrap.yaml`](test/integration/bootstrap.yaml) must match production - divergence causes test failures
- PQC handshake adds 2-5ms latency - tests with tight timeouts will fail