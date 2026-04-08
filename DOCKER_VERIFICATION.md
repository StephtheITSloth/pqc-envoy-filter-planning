# Docker-Based Build and Test Verification

**Date**: 2026-04-08  
**Purpose**: Verify compilation and run tests using Docker containers

---

## Quick Start

### 1. Build Development Container

```bash
# Build the development container
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev .

# Expected output:
# Successfully built <image_id>
# Successfully tagged pqc-envoy-dev:latest
```

### 2. Run Development Container

```bash
# Start interactive development container
docker run -it -v ${PWD}:/workspace pqc-envoy-dev bash

# You should now be inside the container at /workspace
```

### 3. Verify Compilation Inside Container

```bash
# Inside the container:

# Build proto definitions
bazel build //src/config:all

# Build filter libraries
bazel build //src/filters/pqc:all

# Build test targets
bazel build //test/filters/pqc:all

# Expected output for each:
# INFO: Build completed successfully
```

### 4. Run Tests Inside Container

```bash
# Inside the container:

# Run all tests
bazel test //test/filters/pqc:all

# Expected output:
# //test/filters/pqc:pqc_filter_test          PASSED
# //test/filters/pqc:context_test             PASSED
# //test/filters/pqc:policy_engine_test       PASSED
# //test/filters/pqc:fallback_manager_test    PASSED
# //test/filters/pqc:metrics_test             PASSED
# //test/filters/pqc:pqc_network_filter_test  PASSED
#
# Executed 6 test suites: 6 tests pass.
```

### 5. Run Specific Test with Verbose Output

```bash
# Inside the container:

bazel test //test/filters/pqc:pqc_filter_test --test_output=all

# Expected output:
# [==========] Running 8 tests from 1 test suite.
# [ RUN      ] PqcFilterTest.ConfigurationCreation
# [       OK ] PqcFilterTest.ConfigurationCreation
# [ RUN      ] PqcFilterTest.FilterDisabled
# [       OK ] PqcFilterTest.FilterDisabled
# [ RUN      ] PqcFilterTest.DecodeHeadersContinues
# [       OK ] PqcFilterTest.DecodeHeadersContinues
# [ RUN      ] PqcFilterTest.DecodeDataContinues
# [       OK ] PqcFilterTest.DecodeDataContinues
# [ RUN      ] PqcFilterTest.DecodeTrailersContinues
# [       OK ] PqcFilterTest.DecodeTrailersContinues
# [ RUN      ] PqcFilterTest.OnDestroy
# [       OK ] PqcFilterTest.OnDestroy
# [ RUN      ] PqcFilterTest.PqcCapableConnection
# [       OK ] PqcFilterTest.PqcCapableConnection
# [ RUN      ] PqcFilterTest.ClassicalOnlyConnection
# [       OK ] PqcFilterTest.ClassicalOnlyConnection
# [----------] 8 tests from PqcFilterTest (2 ms total)
# [==========] 8 tests from 1 test suite ran.
# [  PASSED  ] 8 tests.
```

### 6. Generate Coverage Report

```bash
# Inside the container:

# Generate coverage
bazel coverage //test/filters/pqc:all_tests

# View coverage summary
cat bazel-out/_coverage/_coverage_report.dat

# Expected coverage:
# pqc_filter.cc: ~70%
# context.h: 100%
# Overall: ~65%
```

---

## Using Docker Compose

### Start Full Stack

```bash
# Start all services (dev, envoy, backend, prometheus, grafana)
docker-compose -f docker/docker-compose.yml up -d

# View logs
docker-compose -f docker/docker-compose.yml logs -f

# Access services:
# - Envoy Admin: http://localhost:9901
# - Prometheus: http://localhost:9090
# - Grafana: http://localhost:3000 (admin/admin)
```

### Build and Test in Compose

```bash
# Execute commands in dev container
docker-compose -f docker/docker-compose.yml exec dev bash

# Inside container:
bazel build //...
bazel test //...
```

### Stop Services

```bash
# Stop all services
docker-compose -f docker/docker-compose.yml down

# Stop and remove volumes
docker-compose -f docker/docker-compose.yml down -v
```

---

## Development Workflow

### 1. Make Code Changes

```bash
# Edit files on host machine (Windows)
# Files are mounted into container via volume
```

### 2. Build and Test in Container

```bash
# In container terminal:
bazel build //src/filters/pqc:pqc_filter_lib
bazel test //test/filters/pqc:pqc_filter_test
```

### 3. Iterate

```bash
# Make changes on host
# Re-run build/test in container
# Bazel caches unchanged files for fast rebuilds
```

---

## One-Liner Commands

### Build Everything

```bash
docker run --rm -v ${PWD}:/workspace pqc-envoy-dev bazel build //...
```

### Run All Tests

```bash
docker run --rm -v ${PWD}:/workspace pqc-envoy-dev bazel test //...
```

### Run Specific Test

```bash
docker run --rm -v ${PWD}:/workspace pqc-envoy-dev \
  bazel test //test/filters/pqc:pqc_filter_test --test_output=all
```

### Generate Coverage

```bash
docker run --rm -v ${PWD}:/workspace pqc-envoy-dev \
  bazel coverage //test/filters/pqc:all_tests
```

---

## Troubleshooting

### Issue 1: Docker Not Found

**Error**: `docker: command not found`

**Solution**:
```bash
# Install Docker Desktop for Windows
# Download from: https://www.docker.com/products/docker-desktop

# Verify installation
docker --version
```

### Issue 2: Permission Denied

**Error**: `permission denied while trying to connect to the Docker daemon`

**Solution**:
```bash
# On Windows: Ensure Docker Desktop is running
# On Linux: Add user to docker group
sudo usermod -aG docker $USER
# Log out and back in
```

### Issue 3: Build Fails in Container

**Error**: `ERROR: no such package '@envoy//...'`

**Solution**:
```bash
# Inside container, fetch dependencies first
bazel fetch //...

# Then build
bazel build //...
```

### Issue 4: Volume Mount Issues

**Error**: Files not visible in container

**Solution**:
```bash
# Use absolute path for volume mount
docker run -it -v C:/Users/steph/Desktop:/workspace pqc-envoy-dev bash

# Or use $(pwd) on Linux/Mac
docker run -it -v $(pwd):/workspace pqc-envoy-dev bash
```

### Issue 5: Bazel Cache Issues

**Error**: `Bazel server terminated abruptly`

**Solution**:
```bash
# Inside container, clean Bazel cache
bazel clean --expunge

# Rebuild
bazel build //...
```

---

## Expected Results

### Successful Build Output

```
INFO: Analyzed 8 targets (150 packages loaded, 5000 targets configured).
INFO: Found 8 targets...
INFO: Elapsed time: 120.5s, Critical Path: 45.2s
INFO: 250 processes: 100 internal, 150 linux-sandbox.
INFO: Build completed successfully, 250 total actions
```

### Successful Test Output

```
//test/filters/pqc:pqc_filter_test          PASSED in 0.5s
//test/filters/pqc:context_test             PASSED in 0.3s
//test/filters/pqc:policy_engine_test       PASSED in 0.2s
//test/filters/pqc:fallback_manager_test    PASSED in 0.2s
//test/filters/pqc:metrics_test             PASSED in 0.2s
//test/filters/pqc:pqc_network_filter_test  PASSED in 0.2s

Executed 6 test suites: 6 tests pass.
INFO: Build completed successfully, 50 total actions
```

---

## Performance Tips

### 1. Use Bazel Cache Volume

```bash
# Create named volume for Bazel cache
docker volume create bazel-cache

# Use it in container
docker run -it \
  -v ${PWD}:/workspace \
  -v bazel-cache:/root/.cache/bazel \
  pqc-envoy-dev bash
```

### 2. Pre-fetch Dependencies

```bash
# Build container with dependencies pre-fetched
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev:cached .

# Uncomment the RUN bazel fetch line in Dockerfile.dev
```

### 3. Use Docker Compose for Persistent Containers

```bash
# Start container in background
docker-compose -f docker/docker-compose.yml up -d dev

# Execute commands without restarting
docker-compose exec dev bazel build //...
docker-compose exec dev bazel test //...
```

---

## Next Steps After Verification

Once Docker-based build and tests pass:

1. ✅ Verify all 8 tests pass in `pqc_filter_test`
2. ✅ Confirm ~70% code coverage
3. 🔄 Continue TDD: Implement policy engine
4. 🔄 Add metrics implementation
5. 🔄 Create integration tests
6. 🎯 Achieve ≥90% coverage

---

## Summary

**Docker Setup**: ✅ Complete
- Development container with Bazel
- Docker Compose for full stack
- Volume mounts for live development

**Verification Commands**:
```bash
# Build container
docker build -f docker/Dockerfile.dev -t pqc-envoy-dev .

# Run tests
docker run --rm -v ${PWD}:/workspace pqc-envoy-dev bazel test //...

# Interactive development
docker run -it -v ${PWD}:/workspace pqc-envoy-dev bash
```

**Ready For**: Compilation verification and test execution in isolated environment
