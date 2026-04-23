from __future__ import annotations

from .mock_policy import evaluate_mock_policy
from .models import PolicyRequest, PolicyResponse
from .settings import Settings
from .watsonx_client import WatsonxClientError, WatsonxPolicyClient


class PolicyDecisionService:
  def __init__(self, settings: Settings, watsonx_client: WatsonxPolicyClient | None = None):
    self._settings = settings
    self._watsonx_client = watsonx_client or WatsonxPolicyClient(settings)

  def evaluate(self, policy_request: PolicyRequest) -> PolicyResponse:
    if self._settings.decision_mode == "mock":
      return evaluate_mock_policy(policy_request, self._settings)

    try:
      return self._watsonx_client.evaluate(policy_request)
    except WatsonxClientError:
      if not self._settings.fail_open:
        raise

    fallback = evaluate_mock_policy(policy_request, self._settings)
    return PolicyResponse(
        action=fallback.action,
        reason=f"{fallback.reason} (watsonx unavailable; mock fallback applied)",
    )

