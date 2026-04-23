// PQC Context - Connection Metadata
//
// Stores PQC-related metadata for a connection.
// This context is extracted from the TLS connection and used for routing decisions.

#pragma once

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {

/**
 * PQC connection context.
 * Stores metadata about the PQC capability and negotiated parameters.
 */
class PqcContext {
public:
  PqcContext() = default;
  
  // Check if connection is PQC-capable
  bool isPqcCapable() const { return pqc_capable_; }
  void setPqcCapable(bool capable) { pqc_capable_ = capable; }
  
  // Get negotiated cipher suite
  absl::optional<std::string> negotiatedCipher() const { return cipher_suite_; }
  void setNegotiatedCipher(absl::string_view cipher) { 
    cipher_suite_ = std::string(cipher); 
  }
  
  // Get TLS version
  absl::optional<std::string> tlsVersion() const { return tls_version_; }
  void setTlsVersion(absl::string_view version) { 
    tls_version_ = std::string(version); 
  }
  
  // Check if fallback occurred
  bool hasFallback() const { return fallback_; }
  void setFallback(bool fallback) { fallback_ = fallback; }
  
  // Get handshake duration in milliseconds
  absl::optional<uint64_t> handshakeDuration() const { return handshake_duration_ms_; }
  void setHandshakeDuration(uint64_t duration_ms) { 
    handshake_duration_ms_ = duration_ms; 
  }
  
  // Get client IP address
  absl::optional<std::string> clientIp() const { return client_ip_; }
  void setClientIp(absl::string_view ip) {
    client_ip_ = std::string(ip);
  }

  // Get SNI (Server Name Indication)
  absl::optional<std::string> sni() const { return sni_; }
  void setSni(absl::string_view sni) {
    sni_ = std::string(sni);
  }

  // Get the HTTP request method
  absl::optional<std::string> requestMethod() const { return request_method_; }
  void setRequestMethod(absl::string_view method) {
    request_method_ = std::string(method);
  }

  // Get the HTTP request path
  absl::optional<std::string> requestPath() const { return request_path_; }
  void setRequestPath(absl::string_view path) {
    request_path_ = std::string(path);
  }

  // Get the request authority/host header value
  absl::optional<std::string> authority() const { return authority_; }
  void setAuthority(absl::string_view authority) {
    authority_ = std::string(authority);
  }

  // Get the service name resolved for downstream policy decisions.
  // This is derived from authority first, then SNI if authority is unavailable.
  absl::optional<std::string> serviceName() const { return service_name_; }
  void setServiceName(absl::string_view service_name) {
    service_name_ = std::string(service_name);
  }

private:
  bool pqc_capable_{false};
  bool fallback_{false};
  absl::optional<std::string> cipher_suite_;
  absl::optional<std::string> tls_version_;
  absl::optional<uint64_t> handshake_duration_ms_;
  absl::optional<std::string> client_ip_;
  absl::optional<std::string> sni_;
  absl::optional<std::string> request_method_;
  absl::optional<std::string> request_path_;
  absl::optional<std::string> authority_;
  absl::optional<std::string> service_name_;
};

using PqcContextSharedPtr = std::shared_ptr<PqcContext>;

} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
