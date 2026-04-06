
# PQC Envoy Filter – Multi-Agent Architecture

**Project**: Post-Quantum Cryptography (PQC) Envoy Filter  
**Author**: Stephane Karim, IBM SkillsBuild Fellowship Week 5, March 2026  
**Status**: Planning Phase  
**Last Updated**: 2026-04-06

---

## Executive Summary

This project implements a PQC-enabled Envoy filter with surrounding agentic tooling to help migrate services to quantum-safe cryptography while preserving observability, safety, and phased rollout capabilities. The solution integrates with IBM watsonx-based multi-agent workflows and follows industry best practices for progressive deployment.

---

## High-Level Goal

Design and implement a PQC-enabled Envoy filter and surrounding agentic tooling that helps migrate services to quantum-safe cryptography while preserving observability, safety, and phased rollout.

---

## Project Context

### Envoy Deployment Model
- Envoy is used as an edge/sidecar proxy for TLS connection termination and origination
- Support for PQC and hybrid (classical + PQC) cipher suites
- Integration with existing service mesh infrastructure

### Multi-Agent Architecture
- Week 5 Visual Kit defines multi-agent system mapped to IBM watsonx stack
- Explicit roles, orchestration logs, and observability expectations
- Multiple services and environments (dev/stage/prod)
- Progressive rollout with guardrails

---

## Functional Requirements

### FR1: PQC Cipher Suite Support
- **NIST-approved algorithms**: ML-KEM, ML-DSA, SLH-DSA
- **Hybrid mode**: Classical + PQC for backward compatibility
- **Pure PQC mode**: Quantum-safe-only environments
- **Dynamic negotiation**: Based on client capabilities

### FR2: Policy-Based Routing
- Route traffic based on PQC capability detection
- Apply different policies per service/route/environment
- Support feature flags for gradual enablement
- Environment-specific configurations (dev/stage/prod)

### FR3: Connection Context Inspection
- Inspect TLS handshake for PQC support
- Extract and annotate connection metadata
- Propagate PQC status through request headers
- Support both edge and sidecar deployment modes

### FR4: Fallback Mechanisms
- Automatic fallback to classical crypto on PQC failure
- Configurable fallback policies per service
- Graceful degradation without connection drops
- Fallback event logging and alerting

### FR5: Configuration Management
- Proto-based configuration schema
- Dynamic configuration updates via xDS
- Validation of PQC parameters and policies
- Template-based config generation for app teams

---

## Non-Functional Requirements

### NFR1: Observability
**Metrics**: PQC vs classical session ratio, handshake success/failure rates, downgrade events, latency impact  
**Logging**: PQC negotiation details, fallback triggers, policy decisions  
**Tracing**: PQC-aware span annotations, distributed tracing integration  
**Dashboards**: Real-time PQC adoption monitoring, health and performance metrics

### NFR2: Reliability & Safety
- 99.9% uptime during PQC rollout
- Zero-downtime configuration updates
- Automatic rollback on error rate thresholds
- Circuit breaker for PQC failures
- Canary and blue/green deployment support

### NFR3: Performance
- <5ms latency overhead for PQC handshakes
- Minimal memory footprint increase (<10%)
- Support for high-throughput scenarios (10K+ req/s)
- Efficient key exchange and signature verification

### NFR4: Security
- Compliance with IBM and NIST PQC guidelines
- Secure key storage and rotation
- Protection against downgrade attacks
- Audit logging for security events

### NFR5: Developer Experience
- Simple configuration templates
- Clear documentation and examples
- Minimal Envoy expertise required
- Integration with existing CI/CD pipelines

---

## Multi-Agent Roles

### Architect / Planner Agent
**Responsibilities**: Derive requirements, design architecture, define rollout plan, coordinate agents  
**Deliverables**: Requirements docs, architecture diagrams, implementation plan, risk assessment

### Envoy Filter Engineer Agent
**Responsibilities**: Design/implement filters, create config protos, write tests  
**Deliverables**: C++ implementation, proto definitions, test suites, documentation

### SRE / Observability Agent
**Responsibilities**: Define metrics/logging/monitoring, create dashboards, implement observability  
**Deliverables**: Prometheus metrics, Grafana dashboards, alert rules, deployment scripts

### Security / Crypto Agent
**Responsibilities**: Validate algorithms, review implementations, ensure compliance  
**Deliverables**: Security validation, crypto reviews, compliance docs, threat models

### Orchestrator Agent
**Responsibilities**: Coordinate handoffs, maintain logs, track decisions, manage rollout  
**Deliverables**: Orchestration logs, decision records, progress tracking, incident coordination

---

## Technical Stack

**Core**: C++17, Bazel, Google Test, Docker, Kubernetes  
**Envoy**: v1.28+, HTTP/Network filter, Proto3 config  
**Observability**: Prometheus, Grafana, Structured JSON logs, Jaeger/OpenTelemetry  
**PQC**: ML-KEM, ML-DSA, SLH-DSA via BoringSSL or liboqs

---

## Implementation Phases

### Phase 1: Foundation & Planning (1-2 weeks) - Plan Mode
**Status**: In Progress  
**Lead**: Architect/Planner  
**Deliverables**: Requirements, architecture, project structure, test strategy, Docker design, CI/CD design

### Phase 2: Core Filter Implementation (3-4 weeks) - Code Mode
**Status**: Not Started  
**Lead**: Filter Engineer  
**Deliverables**: C++ filter, config protos, unit tests, mock integration, cipher negotiation

### Phase 3: Policy & Routing Logic (2-3 weeks) - Code/Advanced Mode
**Status**: Not Started  
**Lead**: Filter Engineer  
**Deliverables**: Policy engine, feature flags, route selection, validation, integration tests

### Phase 4: Observability & Metrics (2 weeks) - Code Mode
**Status**: Not Started  
**Lead**: SRE/Observability  
**Deliverables**: Metrics, logging, tracing, dashboards, alerts

### Phase 5: Fallback & Safety (2 weeks) - Code Mode
**Status**: Not Started  
**Lead**: Filter Engineer + SRE  
**Deliverables**: Fallback logic, circuit breaker, error handling, rollback, chaos tests

### Phase 6: Container & Deployment (1-2 weeks) - Advanced Mode
**Status**: Not Started  
**Lead**: SRE/Observability  
**Deliverables**: Docker images, Helm charts, CI/CD pipeline, runbooks, automation

### Phase 7: Documentation (1 week) - Ask/Plan Mode
**Status**: Not Started  
**Lead**: Architect/Planner  
**Deliverables**: User docs, templates, migration guides, runbooks, training

### Phase 8: Pilot & Rollout (4-6 weeks) - Iterative
**Status**: Not Started  
**Lead**: Orchestrator  
**Deliverables**: Dev deployment, canary rollout, stage validation, prod rollout, analysis

---

## Rollout Strategy

| Stage | Traffic % | Duration | Auto-Rollback | Manual Gates | Soak |
|-------|-----------|----------|---------------|--------------|------|
| Dev | 100% | 1 week | Error >1% | Security review | 24h |
| Stage Canary | 5% | 3 days | Error >0.5%, Latency +10% | SRE approval | 48h |
| Stage Full | 100% | 1 week | Error >0.5%, Latency +10% | Load test | 72h |
| Prod Canary | 10% | 1 week | Error >0.1%, Business -2% | VP approval | 96h |
| Prod Gradual | 50% | 1 week | Error >0.1%, Business -2% | Exec review | 96h |
| Prod Full | 100% | Ongoing | Error >0.05%, Business -1% | CEO approval | 1 week |

### Safety Guardrails
**Critical (Immediate Rollback)**: Error rate 2x threshold, P99 latency +50%, connection failures >5%, security incidents, cert validation >1%  
**Warning (Alert)**: Error rate 1.5x threshold, P99 latency +20-50%, PQC adoption -20%, fallback >10%, memory +15%  
**Monitoring**: All metrics within bounds, continue rollout

---

## Key Metrics

**Connection**: pqc_connections_total, pqc_connections_active, classical_connections_total  
**Handshake**: pqc_handshake_duration_seconds, pqc_handshake_failures_total, pqc_fallback_events_total  
**Policy**: pqc_policy_decisions_total, pqc_feature_flag_checks_total, pqc_route_policy_hits_total  
**Security**: pqc_downgrade_attempts_total, pqc_cipher_suite_usage, pqc_certificate_errors_total

---

## Next Steps

1. Complete project structure setup
2. Create AGENTS.md files for all modes
3. Set up Docker development environment
4. Initialize Bazel workspace with test framework
5. Begin Phase 2: Core Filter Implementation

---

## References

- IBM SkillsBuild Fellowship Week 5 Visual Kit
- NIST Post-Quantum Cryptography Standards
- Envoy Proxy Documentation
- IBM watsonx Multi-Agent Architecture
- Previous PQC Envoy Filter: https://github.com/StephtheITSloth/pqc-envoy-filter