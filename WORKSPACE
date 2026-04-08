# Bazel workspace for PQC Envoy Filter
workspace(name = "pqc_envoy_filter")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Envoy dependency
# Run the following to get the proper hash tag for the envoy:  Invoke-WebRequest -Uri "https://github.com/envoyproxy/envoy/archive/refs/tags/v1.28.0.tar.gz" -OutFile "envoy-v1.28.0.tar.gz"
# Run this next: Get-FileHash .\envoy-v1.28.0.tar.gz -Algorithm SHA256
http_archive(
    name = "envoy",
    sha256 = "C5628B609EF9E5FAFE872B8828089A189BFBFFB6E261B8C4D34EFF4C65229A3F",
    strip_prefix = "envoy-1.28.0",
    urls = ["https://github.com/envoyproxy/envoy/archive/refs/tags/v1.28.0.tar.gz"],
)

# Load Envoy dependencies
load("@envoy//bazel:api_binding.bzl", "envoy_api_binding")
envoy_api_binding()

load("@envoy//bazel:api_repositories.bzl", "envoy_api_dependencies")
envoy_api_dependencies()

load("@envoy//bazel:repositories.bzl", "envoy_dependencies")
envoy_dependencies()

load("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra")
envoy_dependencies_extra()

load("@envoy//bazel:python_dependencies.bzl", "envoy_python_dependencies")
envoy_python_dependencies()

load("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports")
envoy_dependency_imports()

# BoringSSL with PQC support
# Note: Using a hypothetical PQC-enabled fork
# In production, replace with actual BoringSSL PQC fork URL
http_archive(
    name = "boringssl_pqc",
    sha256 = "1e759891e168c5957f2f4d519929e2b4cef9303b7cf2049601081f4fca95bf21",
    strip_prefix = "boringssl-88d7a40bd06a34da6ee0d985545755199d047258",
    urls = ["https://github.com/google/boringssl/archive/88d7a40bd06a34da6ee0d985545755199d047258.tar.gz"],
)

# Google Test framework (already included via Envoy, but explicit for clarity)
http_archive(
    name = "com_google_googletest",
    sha256 = "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7",
    strip_prefix = "googletest-1.14.0",
    urls = ["https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"],
)

# Protocol Buffers
http_archive(
    name = "com_google_protobuf",
    sha256 = "616bb3536ac1fff3fb1a141450fa28b875e985712170ea7f1bfe5e5fc41e2cd8",
    strip_prefix = "protobuf-24.4",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/refs/tags/v24.4.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()

# Abseil (C++ common libraries)
http_archive(
    name = "com_google_absl",
    sha256 = "338420448b140f0dfd1a1ea3c3ce71b3bc172071f24f4d9a57d59b45037da440",
    strip_prefix = "abseil-cpp-20240116.0",
    urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.0.tar.gz"],
)

# Rules for C++
http_archive(
    name = "rules_cc",
    sha256 = "2037875b9a4456dce4a79d112a8ae885bbc4aad968e6587dca6e64f3a0900cdf",
    strip_prefix = "rules_cc-0.0.9",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.9/rules_cc-0.0.9.tar.gz"],
)

# Rules for Python
http_archive(
    name = "rules_python",
    sha256 = "9d04041ac92a0985e344235f5d946f71ac543f1b1565f2cdbc9a2aaee8adf55b",
    strip_prefix = "rules_python-0.26.0",
    urls = ["https://github.com/bazelbuild/rules_python/releases/download/0.26.0/rules_python-0.26.0.tar.gz"],
)

load("@rules_python//python:repositories.bzl", "py_repositories")
py_repositories()
