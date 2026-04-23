from __future__ import annotations

import unittest

from app.mock_policy import evaluate_mock_policy
from app.models import PolicyRequest
from app.service import PolicyDecisionService
from app.settings import Settings
from app.watsonx_client import WatsonxClientError, parse_policy_response_content


class FakeWatsonxClient:
  def __init__(self, response=None, error: Exception | None = None):
    self._response = response
    self._error = error

  def evaluate(self, policy_request: PolicyRequest):
    if self._error is not None:
      raise self._error
    return self._response


class PolicyServiceTest(unittest.TestCase):
  def setUp(self) -> None:
    self.settings = Settings(
        host="127.0.0.1",
        port=8080,
        decision_mode="mock",
        fail_open=True,
        require_pqc_services=("payments.internal",),
        allow_fallback_services=("watsonx.ibm.com",),
        require_pqc_path_prefixes=("/agent/secure", "/secure"),
        watsonx_url="",
        watsonx_project_id="",
        watsonx_model_id="meta-llama/llama-3-8b-instruct",
        watsonx_api_version="2024-10-08",
        ibm_cloud_apikey="",
        watsonx_timeout_seconds=10,
    )

  def test_mock_policy_prefers_hybrid_when_pqc_capable(self) -> None:
    policy_request = PolicyRequest.from_dict({
        "service_name": "payments.internal",
        "request_method": "GET",
        "request_path": "/checkout",
        "mode": "hybrid",
        "pqc_capable": True,
    })
    decision = evaluate_mock_policy(policy_request, self.settings)
    self.assertEqual("prefer_hybrid", decision.action)

  def test_mock_policy_denies_pqc_only_classical_request(self) -> None:
    policy_request = PolicyRequest.from_dict({
        "service_name": "legacy.internal",
        "request_method": "GET",
        "request_path": "/secure",
        "mode": "pqc_only",
        "pqc_capable": False,
    })
    decision = evaluate_mock_policy(policy_request, self.settings)
    self.assertEqual("deny", decision.action)

  def test_live_mode_falls_back_when_watsonx_unavailable(self) -> None:
    policy_request = PolicyRequest.from_dict({
        "service_name": "watsonx.ibm.com",
        "request_method": "POST",
        "request_path": "/mvp/check",
        "mode": "hybrid",
        "pqc_capable": False,
    })
    service = PolicyDecisionService(
        Settings(**{**self.settings.__dict__, "decision_mode": "live"}),
        watsonx_client=FakeWatsonxClient(error=WatsonxClientError("upstream unavailable")),
    )
    decision = service.evaluate(policy_request)
    self.assertEqual("fallback", decision.action)
    self.assertIn("mock fallback applied", decision.reason)

  def test_parse_policy_response_content_strips_code_fence(self) -> None:
    decision = parse_policy_response_content(
        "```json\n{\"action\":\"allow\",\"reason\":\"safe to continue\"}\n```")
    self.assertEqual("allow", decision.action)
    self.assertEqual("safe to continue", decision.reason)


if __name__ == "__main__":
  unittest.main()

