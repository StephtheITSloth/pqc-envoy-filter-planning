# Project Advanced Mode Rules (Non-Obvious Only)

## MCP Tool Integration

- Config generation via MCP requires [`config_template.yaml`](../../config/templates/config_template.yaml) - custom Jinja2 filters defined in [`src/tools/template_filters.py`](../../src/tools/template_filters.py)
- Feature flag service integration uses MCP `fetch` tool - must set `X-PQC-Auth` header from environment variable
- xDS config validation via MCP `validate` tool requires running Envoy in validation mode - use [`scripts/validate_xds.sh`](../../scripts/validate_xds.sh)

## Browser Tool Usage

- Grafana dashboard generation requires browser tool to scrape metric names from Prometheus `/metrics` endpoint
- Certificate generation wizard uses browser tool to fetch NIST-approved parameter sets from official website
- Load testing results visualization requires browser tool to render Locust HTML reports

## External Service Dependencies

- Feature flag service at `https://feature-flags.internal` requires VPN connection - MCP calls will timeout without it
- Prometheus instance must be accessible at `http://localhost:9090` for metric validation
- Envoy admin interface at `http://localhost:9901` required for runtime config updates

## Config Generation Patterns

- Template variables in [`config/templates/`](../../config/templates/) use `{{ pqc.* }}` namespace - other namespaces ignored
- Generated configs MUST be validated with [`bazel run //tools:validate_config`](../../tools/validate_config.cc) before deployment
- Environment-specific overrides in [`config/environments/`](../../config/environments/) are merged AFTER template rendering - order matters

## Deployment Automation

- Helm chart generation requires MCP tool to fetch current cluster state - uses `kubectl` context from `~/.kube/config`
- Canary rollout automation via MCP `deploy` tool requires `--dry-run` flag first - direct deployment not allowed
- Rollback automation uses MCP `rollback` tool which triggers Argo Rollouts - manual kubectl rollback breaks automation

## Observability Integration

- Dashboard provisioning via MCP requires Grafana API key in `GRAFANA_API_KEY` environment variable
- Alert rule generation uses MCP `alert` tool which validates against Prometheus rule syntax - invalid rules cause silent failures
- Log aggregation setup via MCP requires Elasticsearch cluster credentials in [`config/logging/es_config.yaml`](../../config/logging/es_config.yaml)

## Security Scanning

- Container image scanning via MCP `scan` tool uses Trivy - requires Docker daemon running
- Dependency vulnerability scanning uses MCP `audit` tool which checks against NIST NVD - requires API key
- Secret scanning via MCP `secrets` tool checks for hardcoded credentials - excludes [`test/`](../../test/) directory

## Performance Testing

- Load testing via MCP `load` tool uses Locust - requires [`test/load/locustfile.py`](../../test/load/locustfile.py)
- Benchmark results stored in [`benchmarks/results/`](../../benchmarks/results/) - MCP tool generates comparison reports
- Profiling via MCP `profile` tool requires Envoy built with `-c dbg` - release builds lack symbols

## CI/CD Integration

- GitHub Actions workflow generation via MCP requires [`github_token`](../../.github/workflows/ci.yml) secret
- Artifact publishing to container registry uses MCP `publish` tool - requires `REGISTRY_TOKEN` environment variable
- Release automation via MCP `release` tool creates Git tags and GitHub releases - requires maintainer permissions