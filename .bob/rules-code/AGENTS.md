# Project Coding Rules (Non-Obvious Only)

## Filter Implementation Gotchas

- Filter state MUST be stored in [`PqcContext`](../../src/filters/pqc/context.h), NOT in filter member variables - Envoy reuses filter instances
- [`decodeHeaders()`](../../src/filters/pqc/pqc_filter.h) is called BEFORE TLS handshake completes - use [`onNewConnection()`](../../src/filters/pqc/pqc_network_filter.h) for crypto decisions
- Returning `FilterStatus::StopIteration` does NOT pause the filter chain - use `FilterStatus::StopIterationAndBuffer` for async operations
- Filter destruction happens on worker thread, not main thread - avoid blocking operations in destructor

## Crypto Integration Patterns

- [`CryptoManager::selectCipherSuite()`](../../src/crypto/crypto_manager.h) returns `StatusOr<CipherSuite>` - MUST check status before using value
- PQC key generation is synchronous and blocks - call during initialization, NOT per-request
- Hybrid mode requires calling [`negotiateHybrid()`](../../src/crypto/hybrid_negotiator.h) which internally performs TWO handshakes - handle both failure modes
- Certificate chain validation uses custom validator in [`src/crypto/cert_validator.h`](../../src/crypto/cert_validator.h) - standard OpenSSL validation is bypassed

## Configuration Proto Patterns

- Proto field `oneof cipher_mode` requires explicit case checking - default case causes runtime error
- Repeated fields in [`pqc_config.proto`](../../src/config/pqc_config.proto) are NOT thread-safe - copy before iteration
- Proto validation happens in [`validateConfig()`](../../src/config/validator.h) which throws `EnvoyException` - catch during initialization only
- xDS config updates call [`onConfigUpdate()`](../../src/filters/pqc/pqc_filter.h) on worker thread - use `postToMainThread()` for state changes

## Testing Patterns

- Mock Envoy APIs in [`test/mocks/`](../../test/mocks/) use gmock `EXPECT_CALL` - order matters for sequential calls
- Integration tests require [`bootstrap.yaml`](../../test/integration/bootstrap.yaml) with specific listener config - missing `transport_socket` causes silent failures
- Crypto tests use pre-generated keys in [`test/crypto/test_keys/`](../../test/crypto/test_keys/) - regenerating keys breaks test determinism
- Bazel test targets with `size = "large"` get 15min timeout - PQC tests often need this

## Observability Instrumentation

- Metrics MUST use [`Stats::Scope::counterFromString()`](../../src/filters/pqc/metrics.h) - `Stats::Counter` constructor is private
- Histogram buckets for `pqc_handshake_duration` are hardcoded in [`metrics.cc`](../../src/filters/pqc/metrics.cc) - changing requires recompile
- Structured logs require `ENVOY_LOG_TO_LOGGER` macro with `spdlog::source_loc` - plain `ENVOY_LOG` loses file/line info
- Tracing spans created with [`Tracing::Span::setTag()`](../../src/filters/pqc/tracing.h) are NOT propagated to child spans automatically

## Memory Management

- PQC key buffers use custom allocator from [`src/common/memory.h`](../../src/common/memory.h) - standard `new`/`delete` causes memory corruption
- Connection context in [`PqcContext`](../../src/filters/pqc/context.h) is ref-counted - use `std::shared_ptr`, NOT raw pointers
- Envoy's `Buffer::Instance` is move-only - cannot copy, must use `move()` or `drain()`
- Thread-local storage via `ThreadLocal::Slot` requires explicit cleanup in destructor

## Build System Quirks

- Bazel `cc_library` with `alwayslink = 1` is required for filter registration - without it, filter won't load
- Proto dependencies must use `@envoy_api//` prefix, not relative paths - Bazel won't find them otherwise
- Test data files require `data = ["//test:test_data"]` in BUILD file - runtime file not found errors otherwise
- Debug builds (`-c dbg`) disable optimizations for PQC crypto - 10x slower than release builds

## Error Handling Patterns

- Network errors return `FilterStatus::StopIteration` and log - throwing exceptions crashes Envoy
- Config validation errors throw `EnvoyException` during initialization - caught by Envoy and logged
- Crypto errors trigger fallback via [`FallbackManager::triggerFallback()`](../../src/filters/pqc/fallback_manager.h) - returns `StatusOr<FallbackResult>`
- Async operations use `Event::Dispatcher::post()` for callbacks - direct calls cause race conditions