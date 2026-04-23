from __future__ import annotations

from dataclasses import dataclass
import os
from typing import Iterable


def _split_csv(value: str) -> tuple[str, ...]:
  return tuple(item.strip().lower() for item in value.split(",") if item.strip())


def _env_bool(name: str, default: bool) -> bool:
  value = os.getenv(name)
  if value is None:
    return default
  return value.strip().lower() in {"1", "true", "yes", "on"}


def _env_int(name: str, default: int) -> int:
  value = os.getenv(name)
  if value is None:
    return default
  return int(value)


@dataclass(frozen=True)
class Settings:
  host: str
  port: int
  decision_mode: str
  fail_open: bool
  require_pqc_services: tuple[str, ...]
  allow_fallback_services: tuple[str, ...]
  require_pqc_path_prefixes: tuple[str, ...]
  watsonx_url: str
  watsonx_project_id: str
  watsonx_model_id: str
  watsonx_api_version: str
  ibm_cloud_apikey: str
  watsonx_timeout_seconds: int

  @property
  def watsonx_ready(self) -> bool:
    return bool(
        self.watsonx_url and
        self.watsonx_project_id and
        self.watsonx_model_id and
        self.ibm_cloud_apikey)


def load_settings() -> Settings:
  decision_mode = os.getenv("POLICY_DECISION_MODE", "mock").strip().lower()
  if decision_mode not in {"mock", "live"}:
    raise ValueError("POLICY_DECISION_MODE must be either 'mock' or 'live'")

  return Settings(
      host=os.getenv("POLICY_SERVICE_HOST", "0.0.0.0"),
      port=_env_int("POLICY_SERVICE_PORT", 8080),
      decision_mode=decision_mode,
      fail_open=_env_bool("POLICY_FAIL_OPEN", True),
      require_pqc_services=_split_csv(
          os.getenv("MOCK_REQUIRE_PQC_SERVICES", "payments.internal")),
      allow_fallback_services=_split_csv(
          os.getenv("MOCK_ALLOW_FALLBACK_SERVICES", "watsonx.ibm.com")),
      require_pqc_path_prefixes=_split_csv(
          os.getenv("MOCK_REQUIRE_PQC_PATH_PREFIXES", "/agent/secure,/secure")),
      watsonx_url=os.getenv("WATSONX_URL", "").strip().rstrip("/"),
      watsonx_project_id=os.getenv("WATSONX_PROJECT_ID", "").strip(),
      watsonx_model_id=os.getenv(
          "WATSONX_MODEL_ID", "meta-llama/llama-3-8b-instruct").strip(),
      watsonx_api_version=os.getenv("WATSONX_API_VERSION", "2024-10-08").strip(),
      ibm_cloud_apikey=os.getenv("IBM_CLOUD_APIKEY", "").strip(),
      watsonx_timeout_seconds=_env_int("WATSONX_TIMEOUT_SECONDS", 10),
  )

