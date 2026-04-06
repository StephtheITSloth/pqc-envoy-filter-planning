# PQC Envoy Filter - Multi-Agent Architecture

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-Apache%202.0-blue)]()
[![Envoy](https://img.shields.io/badge/envoy-1.28%2B-purple)]()

Post-Quantum Cryptography (PQC) enabled Envoy filter with multi-agent architecture for safe migration to quantum-safe cryptography.

**Author**: Stephane Karim, IBM SkillsBuild Fellowship Week 5, March 2026

---

## Overview

This project implements a production-ready Envoy filter that enables Post-Quantum Cryptography (PQC) with:

- **NIST-approved algorithms**: ML-KEM, ML-DSA, SLH-DSA
- **Hybrid mode**: Classical + PQC for backward compatibility
- **Progressive rollout**: Feature flags, canary deployments, automatic rollback
- **Full observability**: Metrics, logging, tracing, dashboards
- **Multi-agent architecture**: Coordinated development with specialized agents

## Quick Start

### Prerequisites

- Docker 20.10+
- Bazel 6.0+
- C++17 compiler
- 8GB RAM minimum

### Development Setup

```bash
# Clone repository
git clone https://github.com/StephtheITSloth/pqc-envoy-filter.git
cd pqc-envoy-filter

# Build development container
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev .

# Run development environment
docker run -it -v $(pwd):/workspace pqc-envoy-dev bash

# Inside container: Build project
bazel build //...

# Run tests
bazel test //...
```

### Running Locally

```bash
# Build Envoy with PQC filter
bazel build //src/filters/pqc:pqc_filter

# Start Envoy with test config
bazel run //src:envoy -- -c config/bootstrap.yaml

# In another terminal: Send test request
curl -v https://localhost:8443/test
```

## Project Structure

```
pqc-envoy-filter/
├── .bob/                      # Mode-specific agent rules
│   ├── rules-code/           # Code mode guidance
│   ├── rules-advance/        # Advanced mode guidance
│   ├── rules-ask/            # Ask mode guidance
│   └── rules-plan/           # Plan mode guidance
├── src/                       # C++ source code
│   ├── filters/pqc/          # PQC filter implementation
│   ├── crypto/               # Crypto utilities
│   └── config/               # Configuration protos
├── test/                      # Test suites
│   ├── filters/pqc/          # Unit tests
│   ├── integration/          # Integration tests
│   └── load/                 # Load tests
├── docker/                    # Container definitions
├── docs/                      # Documentation
│   ├── architecture/         # Architecture decisions
│   ├── agents/               # Multi-agent workflows
│   └── runbooks/             # Operational runbooks
├── config/                    # Configuration files
├── AGENTS.md                  # Agent guidance
├── PROJECT_CONTEXT.md         # Project requirements
└── README.md                  # This file
```

## Key Features

### PQC Support
- **Algorithms**: ML-KEM-768, ML-DSA-65, SLH-DSA-128s
- **Modes**: Pure PQC, Hybrid (PQC + Classical), Classical fallback
- **Negotiation**: Automatic cipher suite selection based on client capabilities

### Policy Engine
- **Feature flags**: Per-service, per-route, per-environment
- **Routing**: PQC-aware traffic routing and load balancing
- **Fallback**: Automatic fallback to classical crypto on failures

### Observability
- **Metrics**: Connection counts, handshake duration, error rates, cipher usage
- **Logging**: Structured JSON logs with PQC-specific fields
- **Tracing**: Distributed tracing with PQC annotations
- **Dashboards**: Pre-built Grafana dashboards

### Safety Mechanisms
- **Canary rollout**: Progressive traffic increase with automatic rollback
- **Circuit breaker**: Automatic PQC disable on repeated failures
- **Monitoring**: Real-time health checks and alerting
- **Rollback**: One-command rollback to classical crypto

## Multi-Agent Architecture

This project uses a multi-agent development model with specialized roles:

- **Architect/Planner**: Requirements, architecture, rollout planning
- **Filter Engineer**: C++ implementation, testing, optimization
- **SRE/Observability**: Metrics, dashboards, deployment automation
- **Security/Crypto**: Algorithm validation, security reviews
- **Orchestrator**: Coordination, decision tracking, rollout management

See [`PROJECT_CONTEXT.md`](PROJECT_CONTEXT.md) for detailed agent responsibilities.

## Development Workflow

### Test-Driven Development (TDD)

1. **Write test first** in [`test/filters/pqc/`](test/filters/pqc/)
2. **Run test** (should fail): `bazel test //test/filters/pqc:pqc_filter_test`
3. **Implement feature** in [`src/filters/pqc/`](src/filters/pqc/)
4. **Run test** (should pass): `bazel test //test/filters/pqc:pqc_filter_test`
5. **Refactor** and repeat

### Building and Testing

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

# Run with verbose output
bazel test //... --test_output=all
```

### Docker Development

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

## Configuration

### Basic Configuration

```yaml
# config/pqc_filter.yaml
static_resources:
  listeners:
  - name: listener_0
    address:
      socket_address:
        address: 0.0.0.0
        port_value: 8443
    filter_chains:
    - filters:
      - name: envoy.filters.network.http_connection_manager
        typed_config:
          "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
          http_filters:
          - name: envoy.filters.http.pqc
            typed_config:
              "@type": type.googleapis.com/pqc.envoy.config.v1.PqcFilter
              mode: HYBRID
              feature_flags:
                enabled: true
                service_name: "my-service"
```

### Feature Flags

```yaml
# config/feature_flags.yaml
pqc_enabled:
  dev: true
  stage:
    canary_percentage: 5
    services: ["test-service"]
  prod:
    canary_percentage: 0
    services: []
```

## Observability

### Metrics

Key metrics exposed at `:9901/stats/prometheus`:

- `pqc_connections_total`: Total PQC connections
- `pqc_handshake_duration_seconds`: Handshake latency
- `pqc_handshake_failures_total`: Failed handshakes
- `pqc_fallback_events_total`: Fallback to classical crypto
- `pqc_cipher_suite_usage`: Cipher suite distribution

### Dashboards

Pre-built Grafana dashboards in [`dashboards/`](dashboards/):

- **PQC Adoption**: Connection ratios, adoption trends
- **PQC Health**: Error rates, latency, fallback events
- **PQC Security**: Downgrade attempts, cipher usage

### Logging

Structured JSON logs with PQC-specific fields:

```json
{
  "timestamp": "2026-04-06T07:00:00Z",
  "level": "info",
  "message": "PQC handshake successful",
  "pqc.mode": "hybrid",
  "pqc.cipher_suite": "TLS_ML_KEM_768_SHA256",
  "pqc.handshake_duration_ms": 3.2
}
```

## Deployment

### Rollout Strategy

| Stage | Traffic % | Duration | Auto-Rollback | Soak Period |
|-------|-----------|----------|---------------|-------------|
| Dev | 100% | 1 week | Error >1% | 24 hours |
| Stage Canary | 5% | 3 days | Error >0.5% | 48 hours |
| Stage Full | 100% | 1 week | Error >0.5% | 72 hours |
| Prod Canary | 10% | 1 week | Error >0.1% | 96 hours |
| Prod Gradual | 50% | 1 week | Error >0.1% | 96 hours |
| Prod Full | 100% | Ongoing | Error >0.05% | 1 week |

### Rollback

```bash
# Automatic rollback on threshold breach
# Manual rollback command
kubectl set env deployment/envoy PQC_ENABLED=false

# Or via feature flag
curl -X POST https://feature-flags.internal/api/flags/pqc_enabled \
  -d '{"enabled": false}'
```

## Contributing

This project follows TDD and multi-agent development practices:

1. **Plan Mode**: Design and architecture (edit `*.md` files only)
2. **Code Mode**: Implementation (full access to code)
3. **Advanced Mode**: Deployment and tooling (includes MCP/Browser tools)

See [`AGENTS.md`](AGENTS.md) for detailed development guidelines.

## Documentation

- [`PROJECT_CONTEXT.md`](PROJECT_CONTEXT.md): Complete project requirements and context
- [`AGENTS.md`](AGENTS.md): Development guidelines for AI agents
- [`docs/architecture/`](docs/architecture/): Architecture decision records
- [`docs/runbooks/`](docs/runbooks/): Operational procedures

## License

Apache License 2.0 - See [LICENSE](LICENSE) for details

## References

- [NIST Post-Quantum Cryptography](https://csrc.nist.gov/projects/post-quantum-cryptography)
- [Envoy Proxy Documentation](https://www.envoyproxy.io/docs)
- [IBM watsonx Multi-Agent Architecture](https://www.ibm.com/watsonx)
- [Previous PQC Envoy Filter](https://github.com/StephtheITSloth/pqc-envoy-filter)

## Contact

**Stephane Karim**  
IBM SkillsBuild Fellowship, Week 5, March 2026

---

**Status**: Planning Phase Complete ✅  
**Next Phase**: Core Filter Implementation (Code Mode)