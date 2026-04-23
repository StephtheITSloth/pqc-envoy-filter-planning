# Bazel + Envoy Debugging Checklist

This checklist is for debugging this repo when Bazel, Envoy dependencies, Docker, or CI start failing.

It is based on the actual failure patterns we hit while wiring up this MVP.

## 1. Identify What Kind of Failure You Have

Before changing code, sort the error into one of these buckets:

- `Environment failure`
  Examples:
  - Docker image will not pull
  - Bazel cannot find compiler tools
  - permission denied on cache/output directories
  - network fetch failures

- `Build system failure`
  Examples:
  - Bazel cannot resolve packages or labels
  - `WORKSPACE` and `MODULE.bazel` disagree
  - macro visibility or root package errors
  - toolchain or rule incompatibility

- `Compilation failure`
  Examples:
  - C++ symbols missing
  - wrong includes
  - API mismatch with Envoy

- `Test failure`
  Examples:
  - unit test assertion failures
  - integration bootstrap mismatch
  - timing-sensitive flakes

Rule of thumb:
If Bazel never reaches compiling your target, the problem is probably not your filter logic yet.

## 2. Read the First Real Error

Ignore long cascades at first.

Find the earliest error that mentions one of these:

- `fetching repository`
- `no such package`
- `Unable to load package`
- `toolchain`
- `permission denied`
- `cannot find gcc` / `msvc_not_found`
- `BUILD file not found`

That first error is usually the real blocker.

## 3. Check Version Alignment First

For Envoy-based repos, always compare:

- this repo's [.bazelversion](/c:/Users/steph/Desktop/.bazelversion)
- upstream Envoy's pinned Bazel version
- protobuf/rules versions pulled in by Envoy

What to do:

1. Read `.bazelversion`.
2. Read upstream Envoy's `.bazelversion` if you are pinning to a specific Envoy release.
3. If they differ, assume the mismatch matters until proven otherwise.

Failure pattern we saw:

- Repo used Bazel `7.4.0`
- Envoy `v1.28.0` expected Bazel `6.3.2`
- Result: protobuf analysis failures before our code compiled

## 4. Decide Whether `WORKSPACE` or `MODULE.bazel` Is Real

If a repo has both files, do not assume both are active.

Questions to ask:

- Are real dependencies declared in `WORKSPACE`?
- Is `MODULE.bazel` complete, or just a placeholder?
- Is Bazel defaulting into module behavior that the repo is not ready for?

What to inspect:

- [WORKSPACE](/c:/Users/steph/Desktop/WORKSPACE)
- [MODULE.bazel](/c:/Users/steph/Desktop/MODULE.bazel)
- [.bazelrc](/c:/Users/steph/Desktop/.bazelrc)

Failure pattern we saw:

- Repo had a placeholder `MODULE.bazel`
- Real dependencies lived in `WORKSPACE`
- Bazel 7 made this risky unless we forced the legacy path

## 5. Inspect the Macro, Not Just the BUILD File

If a BUILD rule uses Envoy macros like:

- `envoy_cc_library`
- `envoy_extension_package`
- `envoy_cc_test`

and Bazel errors look strange, read the macro definition upstream.

Why:

- Macros often assume root labels, package groups, visibility, or platform flags
- The error message may point at your target, but the real assumption lives inside the macro

Failure pattern we saw:

- `envoy_extension_package()` expected Envoy-style root visibility labels
- our repo had no root `BUILD.bazel`
- Bazel reported an empty-package style error instead of a friendly explanation

What to do:

1. Open the `.bzl` file for the macro.
2. Look for:
   - `package_group`
   - `select()`
   - `repository =`
   - root labels like `//:extension_library`
3. Check whether your repo provides the labels the macro expects.

## 6. Make the Workspace Root a Real Bazel Package

If dependencies or tools reference labels like:

- `//:WORKSPACE`
- `//:extension_library`
- `//:contrib_library`

then your repo usually needs a root [BUILD.bazel](/c:/Users/steph/Desktop/BUILD.bazel).

Useful root items:

- `exports_files(["WORKSPACE"])`
- `package_group(...)` labels expected by reused macros

Failure pattern we saw:

- Gazelle expected `//:WORKSPACE`
- Envoy macro visibility expected root package groups
- repo had no root `BUILD.bazel`

## 7. Prefer the Smallest Target That Reproduces the Failure

Do not start with `bazel build //...`.

Start narrow:

1. `//src/config:pqc_filter_cc_proto`
2. `//src/filters/pqc:context_lib`
3. `//src/filters/pqc:pqc_filter_lib`
4. `//test/filters/pqc:pqc_filter_test`

Why:

- proto target failures usually expose dependency/toolchain issues
- library target failures usually expose BUILD or compile errors
- test target failures often pull in more of Envoy and can hide the original problem

## 8. Separate Local Windows Problems From Linux CI Problems

This repo touches Envoy, so platform differences matter.

Typical Windows-only blockers:

- missing Visual C++ build tools
- path translation issues
- permissions on Bazel output/cache dirs
- Windows-specific Envoy targets analyzing differently than Linux

Typical Linux/CI blockers:

- missing `gcc`/`build-essential`
- dead Docker image tag
- network fetch failures
- root-owned workspace directories in containers

Use this mindset:

- local Windows helps catch repo wiring issues
- Linux container validation is closer to GitHub Actions truth

## 9. Keep Docker Honest

When Docker is involved, verify all of these:

- base image tag or digest exists
- image contains Bazel or Bazelisk
- image contains compiler toolchain
- workspace files copied into image match the real repo
- default user can write Bazel cache/output paths

What to inspect:

- [docker/Dockerfile.dev](/c:/Users/steph/Desktop/docker/Dockerfile.dev)
- [docker/docker-compose.yml](/c:/Users/steph/Desktop/docker/docker-compose.yml)
- [.github/workflows/dev-docker-build.yml](/c:/Users/steph/Desktop/.github/workflows/dev-docker-build.yml)

Failure patterns we saw:

- dead Envoy dev image reference
- missing compiler toolchain
- copied workspace missing new root files
- `/workspace` not writable for Bazel cache

## 10. Watch for Portability Smells in Config

These are easy to miss:

- `/tmp/...` paths
- `%workspace%`-style substitutions
- OS-specific shell assumptions
- hardcoded compiler expectations

Safer default:

- prefer repo-relative paths when possible
- keep CI and local cache paths aligned

Failure pattern we saw:

- disk cache config broke on Windows
- then broke inside Linux container for a different reason
- relative path was safer, but container ownership still mattered

## 11. Use These Questions When You Get Stuck

Ask yourself:

1. Did Bazel reach compilation, or did it fail earlier?
2. Is this a repo issue, an upstream dependency issue, or an environment issue?
3. Am I using the Bazel version upstream expects?
4. Does this macro assume root labels or package groups I have not defined?
5. Am I debugging on the same platform as CI?
6. Am I starting from the smallest failing target?

## 12. Suggested Debugging Workflow

Use this order:

1. Check `.bazelversion`, `WORKSPACE`, `MODULE.bazel`, `.bazelrc`
2. Run the smallest useful Bazel target
3. Read the first real error only
4. If the error comes from a macro, inspect the macro source
5. Fix environment/tooling issues before touching C++
6. Re-run the smallest target
7. Only expand to wider targets after analysis succeeds

## 13. Repo-Specific Lessons From This MVP

These were the big lessons from this repo:

- Envoy version pins are strong signals, not suggestions
- placeholder Bzlmod files are dangerous if not clearly disabled
- root Bazel package setup matters when borrowing upstream macros
- Docker image tags can fail before CI even starts
- local Windows and Linux CI can fail for different reasons

## 14. Fast Triage Commands

Use commands like these:

```powershell
Get-Content .bazelversion,WORKSPACE,MODULE.bazel,.bazelrc
```

```powershell
.\.tools\bazel.exe cquery //src/filters/pqc:context_lib
```

```powershell
.\.tools\bazel.exe cquery //src/filters/pqc:pqc_filter_lib
```

```powershell
.\.tools\bazel.exe build //src/config:pqc_filter_cc_proto --verbose_failures
```

```powershell
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev .
```

## 15. What to Practice Next

To get better at spotting these quickly:

- read upstream `.bzl` macro files when errors feel mysterious
- compare version pins before changing implementation code
- practice classifying failures into environment vs build-system vs compile vs test
- keep notes on recurring failure patterns

The goal is not memorizing every Bazel rule.
The goal is getting good at recognizing which layer is actually broken.
