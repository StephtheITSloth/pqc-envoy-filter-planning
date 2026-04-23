from __future__ import annotations

import json
import time
from typing import Any, Mapping
from urllib import parse, request

from .models import PolicyRequest, PolicyResponse, ValidationError
from .settings import Settings


IAM_TOKEN_URL = "https://iam.cloud.ibm.com/identity/token"


class WatsonxClientError(RuntimeError):
  """Raised when the watsonx client cannot complete an evaluation."""


def _strip_code_fence(content: str) -> str:
  stripped = content.strip()
  if stripped.startswith("```"):
    lines = stripped.splitlines()
    if lines and lines[0].startswith("```"):
      lines = lines[1:]
    if lines and lines[-1].strip() == "```":
      lines = lines[:-1]
    return "\n".join(lines).strip()
  return stripped


def parse_policy_response_content(content: str) -> PolicyResponse:
  try:
    payload = json.loads(_strip_code_fence(content))
  except json.JSONDecodeError as exc:
    raise ValidationError("watsonx response was not valid JSON") from exc
  return PolicyResponse.from_dict(payload)


class WatsonxPolicyClient:
  def __init__(self, settings: Settings, opener: request.OpenerDirector | None = None):
    self._settings = settings
    self._opener = opener or request.build_opener()
    self._cached_token = ""
    self._token_expiration_epoch = 0

  def evaluate(self, policy_request: PolicyRequest) -> PolicyResponse:
    if not self._settings.watsonx_ready:
      raise WatsonxClientError(
          "watsonx live mode is enabled, but the required credentials are incomplete")

    token = self._get_iam_token()
    chat_url = (
        f"{self._settings.watsonx_url}/ml/v1/text/chat"
        f"?version={parse.quote(self._settings.watsonx_api_version)}"
    )
    body = {
        "model_id": self._settings.watsonx_model_id,
        "project_id": self._settings.watsonx_project_id,
        "messages": self._build_messages(policy_request),
        "max_tokens": 140,
        "time_limit": 1000,
    }
    payload = json.dumps(body).encode("utf-8")
    req = request.Request(
        chat_url,
        data=payload,
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json",
            "Accept": "application/json",
        },
        method="POST",
    )

    try:
      with self._opener.open(req, timeout=self._settings.watsonx_timeout_seconds) as response:
        response_payload = json.loads(response.read().decode("utf-8"))
    except Exception as exc:  # pragma: no cover - network errors are environment-specific
      raise WatsonxClientError("watsonx chat request failed") from exc

    try:
      content = response_payload["choices"][0]["message"]["content"]
    except (KeyError, IndexError, TypeError) as exc:
      raise WatsonxClientError("watsonx response did not contain a chat message") from exc

    if not isinstance(content, str):
      raise WatsonxClientError("watsonx response content was not text")

    try:
      return parse_policy_response_content(content)
    except ValidationError as exc:
      raise WatsonxClientError(str(exc)) from exc

  def _get_iam_token(self) -> str:
    now = int(time.time())
    if self._cached_token and now < self._token_expiration_epoch - 60:
      return self._cached_token

    encoded_form = parse.urlencode({
        "grant_type": "urn:ibm:params:oauth:grant-type:apikey",
        "apikey": self._settings.ibm_cloud_apikey,
    }).encode("utf-8")
    req = request.Request(
        IAM_TOKEN_URL,
        data=encoded_form,
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        method="POST",
    )
    try:
      with self._opener.open(req, timeout=self._settings.watsonx_timeout_seconds) as response:
        payload = json.loads(response.read().decode("utf-8"))
    except Exception as exc:  # pragma: no cover - network errors are environment-specific
      raise WatsonxClientError("IBM Cloud IAM token request failed") from exc

    token = payload.get("access_token")
    expiration = payload.get("expiration")
    if not isinstance(token, str) or not token:
      raise WatsonxClientError("IBM Cloud IAM token response did not include access_token")
    if not isinstance(expiration, int):
      raise WatsonxClientError("IBM Cloud IAM token response did not include expiration")

    self._cached_token = token
    self._token_expiration_epoch = expiration
    return token

  def _build_messages(self, policy_request: PolicyRequest) -> list[Mapping[str, Any]]:
    system_prompt = (
        "You are a deterministic PQC routing policy engine. "
        "Respond with JSON only and no markdown. "
        "The JSON schema is {\"action\":\"allow|prefer_hybrid|fallback|deny\","
        "\"reason\":\"short explanation\"}. "
        "Honor these rules strictly: "
        "if mode is pqc_only and pqc_capable is false, action must be deny. "
        "if mode is hybrid and pqc_capable is true, prefer prefer_hybrid. "
        "if mode is hybrid and pqc_capable is false, prefer fallback unless the route or service "
        "clearly looks security-sensitive, in which case deny is allowed. "
        "if mode is classical, action should be allow."
    )
    return [
        {
            "role": "system",
            "content": system_prompt,
        },
        {
            "role": "user",
            "content": [
                {
                    "type": "text",
                    "text": json.dumps(policy_request.to_prompt_payload(), sort_keys=True),
                }
            ],
        },
    ]

