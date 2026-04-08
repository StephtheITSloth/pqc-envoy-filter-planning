workspace(name = "pqc_envoy_filter")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "envoy",
        sha256 = "c5628b609ef9e5fafe872b8828089a189bfbffb6e261b8c4d34eff4c65229a3f",
    strip_prefix = "envoy-1.28.0",
    urls = ["https://github.com/envoyproxy/envoy/archive/refs/tags/v1.28.0.tar.gz"],
)

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
