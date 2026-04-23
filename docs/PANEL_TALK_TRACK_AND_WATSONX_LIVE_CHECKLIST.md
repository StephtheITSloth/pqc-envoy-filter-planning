# Panel Talk Track And Watsonx Live Checklist

## 60-second talk track

Today we moved the MVP from architecture-only planning into a working decision loop.

We stabilized the Envoy and Bazel foundation enough to keep development moving, implemented the PQC filter so it gathers the right request and TLS context, and then connected that filter to an external policy service. That policy service now returns a structured decision such as `allow`, `prefer_hybrid`, `fallback`, or `deny`, and the filter can enforce that decision asynchronously.

The important point is that we separated enforcement from intelligence. Envoy remains the fast data-plane component, while the external policy service is the decision layer. Right now that service runs in deterministic mock mode for demo and development, and we have already structured it so the next step is swapping the mock decision path for a live IBM `watsonx.ai` call.

So the outcome from today is that the MVP backbone exists: context collection, policy request, policy decision, and enforcement are all defined and implemented.

## 30-second version

Today we completed the core MVP decision loop. The Envoy PQC filter now gathers request and TLS context, sends that context to an external policy service, and enforces the returned policy decision. The service works in mock mode now and is already shaped for live IBM `watsonx.ai` integration next.

## 3-slide summary

### Slide 1: What we built today

Title:
`We completed the MVP decision loop`

Talking points:

- Envoy PQC filter gathers request and TLS context
- filter sends context to an external policy service
- service returns `allow`, `prefer_hybrid`, `fallback`, or `deny`
- Envoy enforces the decision
- fail-open and fail-closed behaviors are supported

### Slide 2: Why it matters

Title:
`We separated fast enforcement from AI decisioning`

Talking points:

- Envoy stays deterministic and fast
- the policy service becomes the AI-ready decision layer
- mock mode lets us demo and test safely
- the contract is stable enough to plug in `watsonx.ai` next

### Slide 3: What comes next

Title:
`Next step: live watsonx.ai`

Talking points:

- provision IBM credentials and project access
- point the policy service at a live `watsonx.ai` model
- keep the same request/response contract
- validate the full flow in Docker or CI
- move from mock decisions to live model-backed policy decisions

## If they ask "is it done?"

Recommended answer:

`The MVP backbone is implemented, but the live IBM model call still needs environment wiring and end-to-end validation. So this is a working prototype milestone, not a finished production deployment.`

## If they ask "what exactly is live vs mock?"

Recommended answer:

`Mock mode uses deterministic local decision logic so we can demonstrate behavior reliably. Live mode keeps the same API contract but replaces the local decision engine with an IBM watsonx.ai chat call.`

## What you need to do to get to live watsonx.ai

### 1. Get the IBM credentials and access in place

You need:

- `IBM_CLOUD_APIKEY`
- `WATSONX_URL`
- `WATSONX_PROJECT_ID`
- `WATSONX_MODEL_ID`
- `WATSONX_API_VERSION`

These are already the variables your service expects in:

- [settings.py](C:/Users/steph/Desktop/services/watsonx_policy_service/app/settings.py)
- [.env.example](C:/Users/steph/Desktop/services/watsonx_policy_service/.env.example)

### 2. Verify project permissions

Your IBM identity needs permission to run prompts in the target watsonx project.

For the current chat API docs, IBM says you need an `Admin` or `Editor` role in the project.

### 3. Choose one supported chat model

Your current default is:

- `meta-llama/llama-3-8b-instruct`

That is already wired in:

- [settings.py](C:/Users/steph/Desktop/services/watsonx_policy_service/app/settings.py)

Before demoing live mode, confirm the chosen model supports the chat API in your watsonx environment.

### 4. Switch the service from mock to live

Set:

```env
POLICY_DECISION_MODE=live
POLICY_FAIL_OPEN=true
WATSONX_URL=<your watsonx base URL>
WATSONX_PROJECT_ID=<your project id>
WATSONX_MODEL_ID=<your model id>
WATSONX_API_VERSION=2024-10-08
WATSONX_TIMEOUT_SECONDS=10
IBM_CLOUD_APIKEY=<your api key>
```

### 5. Smoke-test the policy service by itself

Run:

```powershell
cd services/watsonx_policy_service
python -m app.server
```

Then send one request to confirm the service returns valid JSON:

```json
{
  "service_name": "payments.internal",
  "request_method": "GET",
  "request_path": "/agent/secure",
  "mode": "hybrid",
  "pqc_capable": true,
  "authority": "payments.internal",
  "client_ip": "10.0.0.3",
  "sni": "payments.internal",
  "tls_version": "TLSv1.3",
  "cipher_suite": "TLS_ML_KEM_768_SHA256"
}
```

Expected response shape:

```json
{
  "action": "prefer_hybrid",
  "reason": "..."
}
```

### 6. Keep fail-open on for the first live demo

For the first real `watsonx.ai` run, keep:

```env
POLICY_FAIL_OPEN=true
```

That way:

- if IBM auth fails
- if the model response is malformed
- if the request times out

the system falls back to local policy logic instead of hard-failing the request path.

### 7. Validate the model output format

Your current live client expects the model to return JSON text that parses into:

```json
{
  "action": "allow|prefer_hybrid|fallback|deny",
  "reason": "short explanation"
}
```

That parsing logic lives in:

- [watsonx_client.py](C:/Users/steph/Desktop/services/watsonx_policy_service/app/watsonx_client.py)
- [models.py](C:/Users/steph/Desktop/services/watsonx_policy_service/app/models.py)

If live responses drift from that contract, tighten the system prompt before changing the API shape.

### 8. Run the full integration path

After the service works in live mode:

- run the policy service
- run Envoy with the policy-service cluster configured
- send one PQC-capable request
- send one non-PQC request
- show that Envoy receives and enforces different decisions

That becomes your strongest end-to-end demo.

## Best order of execution

1. Get IBM credentials and project access.
2. Test the policy service in `live` mode without Envoy.
3. Keep `POLICY_FAIL_OPEN=true`.
4. Validate one real model response.
5. Run Envoy plus the policy service together.
6. Only after that, consider fail-closed behavior for stricter demos.

## Good panel phrasing for the next milestone

`The next milestone is replacing deterministic mock decisions with live IBM watsonx.ai decisions while preserving the same enforcement contract in Envoy.`

## IBM references

- IBM watsonx.ai product overview: https://www.ibm.com/products/watsonx-ai
- IBM watsonx.ai chat API docs: https://www.ibm.com/docs/en/watsonx/w-and-w/2.3.x?topic=code-chat
- IBM developer overview for chat: https://www.ibm.com/watsonx/developer/capabilities/chat/
- IBM Cloud IAM token from API key: https://cloud.ibm.com/docs/account?topic=account-iamtoken_from_apikey
