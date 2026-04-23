# Watsonx Policy Service

This document describes the external policy service that sits between the Envoy PQC filter and `watsonx.ai`.

## MVP role

The current MVP flow is:

1. Envoy PQC filter gathers a small request context.
2. Envoy sends that context to the policy service.
3. The policy service returns one decision:
   - `allow`
   - `prefer_hybrid`
   - `fallback`
   - `deny`
4. Envoy enforces the returned action.

## Why this shape

We are deliberately keeping the large-model call out of the hot Envoy data path.

That gives us:

- deterministic local development with `mock` mode
- a clean seam for future Watsonx-backed decisions
- simpler secret handling
- a safer fallback path when IBM credentials are not ready yet

## Service location

Use these files in the repo:

- `services/watsonx_policy_service/app/server.py`
- `services/watsonx_policy_service/app/service.py`
- `services/watsonx_policy_service/app/watsonx_client.py`
- `services/watsonx_policy_service/app/mock_policy.py`

## Modes

### Mock mode

`POLICY_DECISION_MODE=mock`

The service uses deterministic local logic based on:

- filter mode: `classical`, `hybrid`, `pqc_only`
- `pqc_capable`
- service name
- request path

### Live mode

`POLICY_DECISION_MODE=live`

The service:

1. exchanges `IBM_CLOUD_APIKEY` for an IAM bearer token
2. calls the `watsonx.ai` Chat API
3. parses a strict JSON decision response
4. optionally falls back to mock logic when `POLICY_FAIL_OPEN=true`

## Required environment for live mode

- `WATSONX_URL`
- `WATSONX_PROJECT_ID`
- `WATSONX_MODEL_ID`
- `WATSONX_API_VERSION`
- `IBM_CLOUD_APIKEY`

## Local commands

Run unit tests:

```powershell
cd services/watsonx_policy_service
python -m unittest discover tests
```

Run the service:

```powershell
cd services/watsonx_policy_service
python -m app.server
```

Run with Docker Compose:

```powershell
docker compose -f docker/docker-compose.yml up watsonx-policy-service
```

## Current integration status

The Envoy C++ PQC filter now calls `POST /v1/policy/evaluate` asynchronously when `policy_service` is configured on the filter.

That flow currently supports:

- async request buffering while the policy decision is in flight
- structured JSON decisions from the external policy service
- fail-open fallback to the local policy evaluator
- fail-closed local replies when `policy_service.fail_open=false`

## Next integration step

The next engineering step after this integration is environment wiring:

1. add the Envoy cluster/bootstrap config that points to the policy service
2. run the full flow in Docker or CI
3. switch the policy service from `mock` to live `watsonx.ai` credentials
