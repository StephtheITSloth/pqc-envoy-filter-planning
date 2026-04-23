from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Mapping


ALLOWED_ACTIONS = {"allow", "prefer_hybrid", "fallback", "deny"}
ALLOWED_MODES = {"classical", "hybrid", "pqc_only"}


class ValidationError(ValueError):
  """Raised when a request or model response is malformed."""


def _coerce_bool(value: Any) -> bool:
  if isinstance(value, bool):
    return value
  if isinstance(value, str):
    normalized = value.strip().lower()
    if normalized in {"true", "1", "yes"}:
      return True
    if normalized in {"false", "0", "no"}:
      return False
  raise ValidationError("Field 'pqc_capable' must be a boolean")


def _coerce_text(payload: Mapping[str, Any], key: str, default: str = "") -> str:
  value = payload.get(key, default)
  if value is None:
    return default
  if not isinstance(value, str):
    raise ValidationError(f"Field '{key}' must be a string")
  return value


@dataclass(frozen=True)
class PolicyRequest:
  service_name: str
  request_method: str
  request_path: str
  mode: str
  pqc_capable: bool
  authority: str = ""
  client_ip: str = ""
  sni: str = ""
  tls_version: str = ""
  cipher_suite: str = ""

  @classmethod
  def from_dict(cls, payload: Mapping[str, Any]) -> "PolicyRequest":
    if not isinstance(payload, Mapping):
      raise ValidationError("Request body must be a JSON object")

    missing = [
        key for key in ("service_name", "request_method", "request_path", "mode", "pqc_capable")
        if key not in payload
    ]
    if missing:
      raise ValidationError(f"Missing required field(s): {', '.join(missing)}")

    mode = _coerce_text(payload, "mode").strip().lower()
    if mode not in ALLOWED_MODES:
      raise ValidationError(
          "Field 'mode' must be one of: classical, hybrid, pqc_only")

    service_name = _coerce_text(payload, "service_name").strip()
    request_method = _coerce_text(payload, "request_method").strip().upper()
    request_path = _coerce_text(payload, "request_path").strip()

    if not service_name:
      raise ValidationError("Field 'service_name' must not be empty")
    if not request_method:
      raise ValidationError("Field 'request_method' must not be empty")
    if not request_path:
      raise ValidationError("Field 'request_path' must not be empty")

    return cls(
        service_name=service_name,
        request_method=request_method,
        request_path=request_path,
        mode=mode,
        pqc_capable=_coerce_bool(payload.get("pqc_capable")),
        authority=_coerce_text(payload, "authority").strip(),
        client_ip=_coerce_text(payload, "client_ip").strip(),
        sni=_coerce_text(payload, "sni").strip(),
        tls_version=_coerce_text(payload, "tls_version").strip(),
        cipher_suite=_coerce_text(payload, "cipher_suite").strip(),
    )

  def to_prompt_payload(self) -> dict[str, Any]:
    return {
        "service_name": self.service_name,
        "request_method": self.request_method,
        "request_path": self.request_path,
        "mode": self.mode,
        "pqc_capable": self.pqc_capable,
        "authority": self.authority,
        "client_ip": self.client_ip,
        "sni": self.sni,
        "tls_version": self.tls_version,
        "cipher_suite": self.cipher_suite,
    }


@dataclass(frozen=True)
class PolicyResponse:
  action: str
  reason: str

  @classmethod
  def from_dict(cls, payload: Mapping[str, Any]) -> "PolicyResponse":
    if not isinstance(payload, Mapping):
      raise ValidationError("Model response must be a JSON object")

    action = payload.get("action")
    reason = payload.get("reason")
    if not isinstance(action, str) or action not in ALLOWED_ACTIONS:
      raise ValidationError(
          "Field 'action' must be one of: allow, prefer_hybrid, fallback, deny")
    if not isinstance(reason, str) or not reason.strip():
      raise ValidationError("Field 'reason' must be a non-empty string")
    return cls(action=action, reason=reason.strip())

  def to_dict(self) -> dict[str, str]:
    return {"action": self.action, "reason": self.reason}

