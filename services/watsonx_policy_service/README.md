# Watsonx Policy Service

This service is the external policy-decision layer for the PQC Envoy MVP.

It is designed to run in two modes:

- `mock`: deterministic local policy logic for fast development and demos
- `live`: calls `watsonx.ai` Chat API and falls back to the mock policy when `POLICY_FAIL_OPEN=true`

## Endpoints

- `GET /healthz`
- `POST /v1/policy/evaluate`

Example request:

```json
{
  "service_name": "payments.internal",
  "request_method": "GET",
  "request_path": "/agent/decision",
  "mode": "hybrid",
  "pqc_capable": true,
  "authority": "payments.internal:8443",
  "client_ip": "10.0.0.3:50000",
  "sni": "api.quantum.internal",
  "tls_version": "TLSv1.3",
  "cipher_suite": "TLS_ML_KEM_768_SHA256"
}
```

Example response:

```json
{
  "action": "prefer_hybrid",
  "reason": "Hybrid mode prefers a PQC-capable downstream"
}
```

## Run locally

```powershell
$env:POLICY_DECISION_MODE="mock"
python -m app.server
```

From the service directory:

```powershell
cd services/watsonx_policy_service
python -m unittest discover tests
```

## Enable live watsonx

Set these environment variables:

- `POLICY_DECISION_MODE=live`
- `WATSONX_URL`
- `WATSONX_PROJECT_ID`
- `WATSONX_MODEL_ID`
- `WATSONX_API_VERSION`
- `IBM_CLOUD_APIKEY`

The service exchanges the IBM Cloud API key for an IAM bearer token, then calls:

- `POST /ml/v1/text/chat?version=...`

using a strict JSON-only system prompt so the returned content can be parsed into the existing policy contract.

## Notes

- This service intentionally uses the Python standard library only.
- `mock` mode is the default so local development works before secrets are available.
- `live` mode can fail open to the mock policy or fail closed, depending on `POLICY_FAIL_OPEN`.

