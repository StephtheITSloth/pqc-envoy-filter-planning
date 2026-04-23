from __future__ import annotations

from .models import PolicyRequest, PolicyResponse
from .settings import Settings


def _requires_pqc(request: PolicyRequest, settings: Settings) -> bool:
  service_name = request.service_name.lower()
  if service_name in settings.require_pqc_services:
    return True
  return any(request.request_path.startswith(prefix) for prefix in settings.require_pqc_path_prefixes)


def _allows_fallback(request: PolicyRequest, settings: Settings) -> bool:
  if request.mode == "hybrid":
    return True
  return request.service_name.lower() in settings.allow_fallback_services


def evaluate_mock_policy(request: PolicyRequest, settings: Settings) -> PolicyResponse:
  if request.mode == "classical":
    return PolicyResponse(
        action="allow",
        reason="Classical mode allows the request without PQC enforcement",
    )

  if request.mode == "pqc_only":
    if request.pqc_capable:
      return PolicyResponse(
          action="allow",
          reason="PQC-only mode accepted a PQC-capable downstream",
      )
    return PolicyResponse(
        action="deny",
        reason="PQC-only mode requires a PQC-capable downstream",
    )

  if _requires_pqc(request, settings):
    if request.pqc_capable:
      return PolicyResponse(
          action="prefer_hybrid",
          reason="Request matched a PQC-sensitive service or route and the downstream is PQC-capable",
      )
    if _allows_fallback(request, settings):
      return PolicyResponse(
          action="fallback",
          reason="Request matched a PQC-sensitive service or route but fallback is allowed",
      )
    return PolicyResponse(
        action="deny",
        reason="Request matched a PQC-sensitive service or route and fallback is not allowed",
    )

  if request.mode == "hybrid":
    if request.pqc_capable:
      return PolicyResponse(
          action="prefer_hybrid",
          reason="Hybrid mode prefers a PQC-capable downstream",
      )
    return PolicyResponse(
        action="fallback",
        reason="Hybrid mode falls back for non-PQC downstreams",
    )

  return PolicyResponse(
      action="allow",
      reason="No specific policy matched the request",
  )

