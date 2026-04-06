# Project Architecture Rules (Non-Obvious Only)

## Planning-Specific Constraints

- PQC filter MUST support both edge and sidecar deployment - architecture differs significantly
- Policy engine decisions are cached per-connection in [`PqcContext`](../../src/filters/pqc/context.h) - plan for state management
- Feature flags are evaluated ONCE per connection establishment - mid-connection changes ignored
- Hybrid mode requires TWO separate TLS handshakes internally - plan for 2x crypto overhead
- Fallback triggers are NOT reversible within same connection - design one-way state machine

## Multi-Agent Handoffs

- Plan → Code transition requires COMPLETE proto definitions - partial specs cause build failures
- SRE agent needs metric names BEFORE implementation - changing names breaks dashboards
- Security agent must approve cipher suite list BEFORE coding - post-implementation changes expensive
- Orchestrator needs rollout percentages defined upfront - dynamic adjustment not supported

## Architecture Decisions

- Filter chain position CRITICAL: Must be AFTER authentication, BEFORE routing
- Connection pooling disabled for PQC connections - each request gets new TLS handshake
- xDS config updates require Envoy restart in current implementation - plan for downtime
- Metrics aggregation happens at proxy level, not cluster - affects dashboard design
- Tracing context propagation requires custom header injection - standard propagators insufficient

## Non-Obvious Dependencies

- BoringSSL PQC fork has different API than standard BoringSSL - wrapper layer required
- Envoy's SSL context is immutable after creation - dynamic cipher changes need new context
- Feature flag service must be reachable during filter initialization - startup dependency
- Certificate validation happens in separate thread pool - callback timing unpredictable
- Memory allocator for PQC keys is NOT the standard allocator - custom arena required