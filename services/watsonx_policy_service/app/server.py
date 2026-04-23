from __future__ import annotations

import json
import logging
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

from .models import PolicyRequest, ValidationError
from .service import PolicyDecisionService
from .settings import Settings, load_settings
from .watsonx_client import WatsonxClientError


LOG = logging.getLogger("watsonx_policy_service")


class PolicyRequestHandler(BaseHTTPRequestHandler):
  service: PolicyDecisionService
  settings: Settings

  def do_GET(self) -> None:  # noqa: N802
    if self.path.rstrip("/") == "/healthz":
      self._send_json(HTTPStatus.OK, {
          "status": "ok",
          "decision_mode": self.settings.decision_mode,
          "watsonx_ready": self.settings.watsonx_ready,
      })
      return
    self._send_json(HTTPStatus.NOT_FOUND, {"error": "Not found"})

  def do_POST(self) -> None:  # noqa: N802
    if self.path.rstrip("/") != "/v1/policy/evaluate":
      self._send_json(HTTPStatus.NOT_FOUND, {"error": "Not found"})
      return

    try:
      content_length = int(self.headers.get("Content-Length", "0"))
      payload = self.rfile.read(content_length).decode("utf-8")
      request_body = json.loads(payload or "{}")
      policy_request = PolicyRequest.from_dict(request_body)
      decision = self.service.evaluate(policy_request)
      self._send_json(HTTPStatus.OK, decision.to_dict())
    except ValidationError as exc:
      self._send_json(HTTPStatus.BAD_REQUEST, {"error": str(exc)})
    except WatsonxClientError as exc:
      self._send_json(HTTPStatus.BAD_GATEWAY, {"error": str(exc)})
    except json.JSONDecodeError:
      self._send_json(HTTPStatus.BAD_REQUEST, {"error": "Request body was not valid JSON"})
    except Exception as exc:  # pragma: no cover - defensive catch for runtime errors
      LOG.exception("Unhandled policy service error")
      self._send_json(HTTPStatus.INTERNAL_SERVER_ERROR, {"error": str(exc)})

  def log_message(self, format: str, *args) -> None:
    LOG.info("%s - %s", self.address_string(), format % args)

  def _send_json(self, status: HTTPStatus, payload: dict[str, object]) -> None:
    body = json.dumps(payload).encode("utf-8")
    self.send_response(status.value)
    self.send_header("Content-Type", "application/json")
    self.send_header("Content-Length", str(len(body)))
    self.end_headers()
    self.wfile.write(body)


def create_server(settings: Settings | None = None) -> ThreadingHTTPServer:
  settings = settings or load_settings()
  handler = type("ConfiguredPolicyRequestHandler", (PolicyRequestHandler,), {})
  handler.settings = settings
  handler.service = PolicyDecisionService(settings)
  return ThreadingHTTPServer((settings.host, settings.port), handler)


def main() -> None:
  logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
  settings = load_settings()
  server = create_server(settings)
  LOG.info(
      "Starting watsonx policy service on %s:%s in %s mode",
      settings.host,
      settings.port,
      settings.decision_mode,
  )
  server.serve_forever()


if __name__ == "__main__":
  main()

