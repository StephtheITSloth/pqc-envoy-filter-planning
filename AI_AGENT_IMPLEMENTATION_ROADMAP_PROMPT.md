# AI Agent Implementation Roadmap Generator - Universal Prompt Template

**Version**: 1.0.0  
**Purpose**: Universal prompt template for AI agents to generate detailed implementation roadmaps  
**Applicability**: All project types (web apps, APIs, infrastructure, ML/AI, enterprise systems)  
**Last Updated**: 2026-04-06

---

## How to Use This Prompt

1. **Copy the prompt template** from the [Main Prompt](#main-prompt) section below
2. **Fill in your project details** in the placeholders marked with `[YOUR_...]`
3. **Paste into your AI agent** (Claude, GPT-4, etc.)
4. **Review and refine** the generated roadmap
5. **Adapt to your specific needs** using the domain-specific sections

---

## Main Prompt

```markdown
You are a technical lead creating a detailed implementation roadmap for a software project.

# Project Context

**Project Name**: [YOUR_PROJECT_NAME]
**Project Type**: [YOUR_PROJECT_TYPE - e.g., Web App, API, Infrastructure, ML/AI, Enterprise System]
**Tech Stack**: [YOUR_TECH_STACK - e.g., Python/FastAPI, React/TypeScript, Kubernetes, etc.]
**Team Size**: [YOUR_TEAM_SIZE - e.g., 1-2 developers, 5-10 person team]
**Timeline**: [YOUR_TIMELINE - e.g., 8 weeks, 3 months, 6 months]
**Deployment Target**: [YOUR_DEPLOYMENT - e.g., AWS, GCP, Azure, On-premise, Kubernetes]

# Project Description

[YOUR_PROJECT_DESCRIPTION - Provide 2-3 paragraphs describing:
- What problem the project solves
- Who the users are
- Key features and functionality
- Any existing systems or constraints]

# Requirements Summary

## Functional Requirements
[YOUR_FUNCTIONAL_REQUIREMENTS - List 5-10 key functional requirements]

## Non-Functional Requirements
[YOUR_NON_FUNCTIONAL_REQUIREMENTS - List performance, security, scalability, etc.]

# Constraints and Dependencies

**Technical Constraints**: [YOUR_TECHNICAL_CONSTRAINTS]
**Business Constraints**: [YOUR_BUSINESS_CONSTRAINTS]
**External Dependencies**: [YOUR_DEPENDENCIES]

---

# Task: Generate Implementation Roadmap

Create a detailed, enterprise-grade implementation roadmap that includes:

## 1. Architecture Overview

Provide:
- **System Architecture Diagram** (in Mermaid or text format)
- **Component Breakdown** with responsibilities
- **Data Flow** between components
- **Integration Points** with external systems
- **Technology Stack Justification**

## 2. Implementation Phases

Break down the implementation into 6-8 phases:
- **Phase Name and Duration**
- **Lead Role** (who owns this phase)
- **Key Deliverables**
- **Dependencies** on other phases

## 3. Detailed Step-by-Step Plan

For each phase, provide:
- **Numbered Steps** (small, verifiable increments)
- **Goal** for each step
- **Tasks** to complete
- **Artifacts** to produce (files, PRs, documents)
- **Acceptance Criteria** (specific, measurable)
- **Verification Commands** (how to test/validate)

## 4. Code Work Breakdown

Detail:
- **Repository Structure** (directory layout)
- **New Modules/Files** to create
- **Key Classes/Functions** to implement
- **Configuration Files** needed
- **Build System Setup** (if applicable)

## 5. Development Workflow

Specify:
- **Local Development Setup** (Docker, virtual env, etc.)
- **Build Commands** (how to compile/build)
- **Test Commands** (how to run tests)
- **Development Tools** (linters, formatters, etc.)
- **Git Workflow** (branching strategy, PR process)

## 6. Testing Strategy

Define:
- **Test Pyramid** (unit, integration, E2E percentages)
- **Test Types** needed (unit, integration, load, security, etc.)
- **Coverage Targets** (e.g., 80% unit test coverage)
- **Test Data Requirements**
- **Performance Benchmarks**

## 7. Deployment and Operations

Include:
- **Deployment Strategy** (blue-green, canary, rolling)
- **Infrastructure as Code** (Terraform, CloudFormation, etc.)
- **CI/CD Pipeline** (GitHub Actions, Jenkins, etc.)
- **Monitoring and Observability** (metrics, logs, traces)
- **Rollback Procedures**

## 8. Rollout Plan

Provide:
- **Environment Progression** (dev → stage → prod)
- **Traffic Ramping Strategy** (percentages and timelines)
- **Feature Flags** configuration
- **Success Metrics** for each stage
- **Rollback Triggers** (automated thresholds)
- **Soak Periods** between stages

## 9. Documentation Requirements

List:
- **User Documentation** (getting started, tutorials)
- **API Documentation** (if applicable)
- **Developer Documentation** (architecture, contributing)
- **Operational Runbooks** (deployment, troubleshooting)

## 10. Success Criteria

Define success criteria for:
- **Each Phase** (what makes a phase complete)
- **Overall Project** (what makes the project successful)
- **Performance Targets** (latency, throughput, etc.)
- **Quality Targets** (test coverage, bug rates, etc.)

---

# Output Format Requirements

1. **Use Markdown** with clear headings and structure
2. **Include Mermaid Diagrams** for architecture and flows
3. **Provide Code Examples** where helpful
4. **Use Tables** for phase timelines and metrics
5. **Add Checklists** for acceptance criteria
6. **Include Commands** for verification steps
7. **Be Specific** - avoid vague statements like "implement feature X"
8. **Make Steps Small** - each step should be completable in 1-3 days
9. **Add Context** - explain WHY decisions are made, not just WHAT to do
10. **Consider Scale** - address how the solution scales with growth

---

# Additional Guidance

- **Assume TDD** (Test-Driven Development) where appropriate
- **Prioritize Observability** - metrics, logs, and traces from day one
- **Plan for Failure** - include error handling, retries, circuit breakers
- **Security First** - address security at each phase, not as an afterthought
- **Document Decisions** - use Architecture Decision Records (ADRs)
- **Think Enterprise** - consider multi-environment, multi-team scenarios
- **Be Pragmatic** - balance perfection with practical constraints

---

# Example Output Structure

Your output should follow this structure:

```markdown
# [Project Name] - Implementation Roadmap

## Executive Summary
[Brief overview of the implementation approach]

## Architecture Overview
[System architecture with diagrams]

## Implementation Phases
[8-phase breakdown with timeline]

## Detailed Implementation Steps

### PHASE 1: [Phase Name] (Week X)

#### Step 1.1: [Step Name]
**Goal**: [What this step achieves]

**Tasks**:
1. [Specific task]
2. [Specific task]

**Artifacts**:
- `path/to/file.ext` - [Description]
- `path/to/other.ext` - [Description]

**Acceptance Criteria**:
- [ ] [Specific, measurable criterion]
- [ ] [Specific, measurable criterion]

**Verification**:
```bash
# Commands to verify this step
command --to-verify
```

[Repeat for all steps in all phases]

## Testing Strategy
[Detailed testing approach]

## Deployment and Operations
[Deployment strategy and operational procedures]

## Rollout Plan
[Progressive rollout with metrics and triggers]

## Success Criteria
[How to measure success]
```

---

# Domain-Specific Considerations

Based on your project type, emphasize:

**Web Applications**:
- Frontend framework setup
- State management
- Responsive design
- Browser compatibility
- SEO considerations

**APIs and Backend Services**:
- API design (REST, GraphQL, gRPC)
- Authentication/authorization
- Rate limiting
- API versioning
- Documentation (OpenAPI/Swagger)

**Infrastructure Projects**:
- Infrastructure as Code
- Configuration management
- Secrets management
- Disaster recovery
- Cost optimization

**Machine Learning/AI**:
- Data pipeline setup
- Model training infrastructure
- Feature engineering
- Model versioning
- A/B testing framework

**Enterprise Systems**:
- Legacy system integration
- Compliance requirements
- Change management
- Governance and approvals
- Long-term support planning

---

Generate the implementation roadmap now.
```

---

## Quick Start Examples

### Example 1: Web Application

```markdown
**Project Name**: E-commerce Platform
**Project Type**: Web Application
**Tech Stack**: React, Node.js, PostgreSQL, Redis, AWS
**Team Size**: 5 developers
**Timeline**: 12 weeks
**Deployment Target**: AWS (ECS, RDS, ElastiCache)

**Project Description**:
Building a modern e-commerce platform with product catalog, shopping cart, 
checkout, and order management. Must support 10K concurrent users with 
<200ms page load times. Integration with Stripe for payments and 
SendGrid for emails.

**Functional Requirements**:
- User authentication and profiles
- Product catalog with search and filters
- Shopping cart and wishlist
- Checkout with multiple payment methods
- Order tracking and history
- Admin dashboard for inventory management

**Non-Functional Requirements**:
- 99.9% uptime
- <200ms page load time
- Support 10K concurrent users
- PCI DSS compliance for payments
- GDPR compliance for user data
```

### Example 2: API Service

```markdown
**Project Name**: Payment Processing API
**Project Type**: RESTful API
**Tech Stack**: Python/FastAPI, PostgreSQL, Redis, Kubernetes
**Team Size**: 3 developers
**Timeline**: 8 weeks
**Deployment Target**: Kubernetes on GCP

**Project Description**:
Building a payment processing API that integrates with multiple payment 
providers (Stripe, PayPal, Square). Must handle 1000 transactions/second 
with <100ms latency. Includes webhook handling, retry logic, and 
comprehensive audit logging.

**Functional Requirements**:
- Process payments via multiple providers
- Handle webhooks from payment providers
- Retry failed transactions
- Audit logging for all transactions
- Idempotency for duplicate requests

**Non-Functional Requirements**:
- 99.99% uptime
- <100ms P99 latency
- 1000 transactions/second
- PCI DSS Level 1 compliance
- SOC 2 Type II compliance
```

### Example 3: Infrastructure Project

```markdown
**Project Name**: Multi-Region Kubernetes Platform
**Project Type**: Infrastructure
**Tech Stack**: Kubernetes, Terraform, Helm, Prometheus, Grafana
**Team Size**: 2 SREs
**Timeline**: 10 weeks
**Deployment Target**: AWS (3 regions)

**Project Description**:
Building a multi-region Kubernetes platform with automated failover, 
centralized logging, and monitoring. Must support 50+ microservices 
with zero-downtime deployments and disaster recovery capabilities.

**Functional Requirements**:
- Multi-region Kubernetes clusters
- Automated failover between regions
- Centralized logging (ELK stack)
- Monitoring and alerting (Prometheus/Grafana)
- GitOps deployment (ArgoCD)

**Non-Functional Requirements**:
- 99.95% uptime
- <5 minute failover time
- Support 50+ microservices
- <1 hour disaster recovery time
- Cost optimization (<$10K/month)
```

---

## Tips for Best Results

### 1. Be Specific About Your Project
- Provide concrete examples of features
- Include actual technology names and versions
- Specify real performance targets
- Mention existing systems or constraints

### 2. Include Context
- Explain WHY you're building this
- Describe WHO will use it
- Clarify WHAT success looks like
- Mention any CONSTRAINTS or DEPENDENCIES

### 3. Adjust the Timeline
- Small projects: 4-8 weeks
- Medium projects: 8-16 weeks
- Large projects: 16-24 weeks
- Enterprise projects: 24+ weeks

### 4. Specify Your Team
- Solo developer: More automation, simpler architecture
- Small team (2-5): Moderate complexity, clear ownership
- Large team (5+): More coordination, parallel work streams

### 5. Clarify Deployment Target
- Cloud provider (AWS, GCP, Azure)
- On-premise infrastructure
- Hybrid cloud
- Edge computing
- Serverless

---

## Customization Options

### For Agile Teams
Add to prompt:
```
Use 2-week sprints. Break phases into sprint-sized increments. 
Include sprint planning, daily standup, and retrospective activities.
```

### For Waterfall Projects
Add to prompt:
```
Use traditional waterfall phases with clear gates between phases. 
Include detailed requirements sign-off and change control procedures.
```

### For Startups/MVPs
Add to prompt:
```
Focus on MVP (Minimum Viable Product) first. Prioritize speed over 
perfection. Include technical debt tracking and future enhancement plans.
```

### For Enterprise Projects
Add to prompt:
```
Include compliance requirements (SOC 2, HIPAA, PCI DSS, etc.). 
Add governance and approval workflows. Plan for long-term support 
and maintenance.
```

### For Open Source Projects
Add to prompt:
```
Include community contribution guidelines. Plan for documentation 
and examples. Add release management and versioning strategy.
```

---

## Common Pitfalls to Avoid

### ❌ Don't Do This:
- Vague steps like "Implement authentication"
- Missing acceptance criteria
- No verification commands
- Unrealistic timelines
- Ignoring non-functional requirements
- Skipping testing strategy
- No rollback plan

### ✅ Do This Instead:
- Specific steps like "Implement JWT-based authentication with refresh tokens"
- Clear, measurable acceptance criteria
- Concrete verification commands
- Realistic timelines with buffer
- Address performance, security, scalability upfront
- Comprehensive testing at each phase
- Detailed rollback procedures

---

## Version History

- **v1.0.0** (2026-04-06): Initial universal prompt template

---

## Related Resources

- [AI Agent Project Planning Roadmap](AI_AGENT_PROJECT_PLANNING_ROADMAP.md) - For planning phase
- [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) - Example project context
- [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) - Example implementation plan

---

**End of AI Agent Implementation Roadmap Generator Prompt Template**