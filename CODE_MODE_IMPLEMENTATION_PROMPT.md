# Code Mode Implementation Prompt - Step 1 Filter Skeleton

**Version**: 1.0.0  
**Purpose**: Reusable prompt for generating initial filter skeleton code  
**Use Case**: Creating new Envoy HTTP filters or similar components  
**Last Updated**: 2026-04-06

---

## How to Use This Prompt

1. **Copy the prompt** from the [Main Prompt](#main-prompt) section
2. **Fill in your project details** in the placeholders
3. **Paste into your AI agent** in Code mode
4. **Review and adapt** the generated code
5. **Commit and test** the implementation

---

## Main Prompt

```markdown
You are implementing Step 1 of an Envoy filter implementation plan.

# Context

**Filter Purpose**: [YOUR_FILTER_PURPOSE - e.g., "Negotiate PQC-capable upstreams based on config"]
**Filter Name**: [YOUR_FILTER_NAME - e.g., "envoy.filters.http.pqc"]
**Repository**: [YOUR_REPO_PATH - e.g., "Envoy repo open at relevant directory"]
**Coding Style**: [YOUR_STYLE - e.g., "Match existing Envoy conventions"]

# Task

Create a minimal HTTP filter skeleton with:

1. **Filter Name and Config Proto**
   - Propose a descriptive filter name following Envoy conventions
   - Define configuration protocol buffer
   - Include basic configuration options
   - Add TODOs for future configuration

2. **C++ Filter Implementation**
   - Filter header file with interface
   - Filter implementation with stub methods
   - Config wrapper class
   - Proper Envoy namespace structure

3. **Factory Registration**
   - Filter factory class
   - Registration with Envoy's filter system
   - Configuration parsing

4. **Unit Test Scaffolding**
   - Test fixture setup
   - Basic test cases
   - Mock setup for Envoy interfaces
   - TODOs for comprehensive tests

# Constraints

- **Match existing coding style** in the repository
- **Follow Envoy conventions** for filters
- **Add TODOs** where specific logic will be implemented later
- **Include comments** explaining each file's purpose
- **Use proper namespacing** (Envoy::Extensions::HttpFilters::YourFilter)
- **Implement required interfaces** (Http::StreamDecoderFilter)
- **Return appropriate status codes** (Continue, StopIteration, etc.)

# Output Format

For each file, provide:

1. **File path** (e.g., `src/filters/your_filter/your_filter.h`)
2. **Complete file content** ready to paste
3. **Brief explanation** of what the file does
4. **Key TODOs** that need to be filled in later steps

# Required Files

## 1. Configuration Proto

**File**: `src/config/[filter_name].proto`

**Must Include**:
- Proto syntax declaration
- Package name
- Configuration message
- Enum for operating modes (if applicable)
- Enable/disable flag
- TODOs for future config options

**Example Structure**:
```protobuf
syntax = "proto3";
package your.package.v1;

message YourFilter {
  enum Mode {
    MODE_UNSPECIFIED = 0;
    MODE_A = 1;
    MODE_B = 2;
  }
  
  Mode mode = 1;
  google.protobuf.BoolValue enabled = 2;
  
  // TODO: Add additional config
}
```

## 2. Filter Header

**File**: `src/filters/[filter_name]/[filter_name].h`

**Must Include**:
- File-level comment explaining purpose
- Include guards
- Necessary Envoy includes
- Config wrapper class
- Filter class implementing Http::StreamDecoderFilter
- Private helper methods with TODOs
- Proper documentation

**Example Structure**:
```cpp
#pragma once

#include "envoy/http/filter.h"
#include "src/config/your_filter.pb.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace YourFilter {

class YourFilterConfig {
public:
  explicit YourFilterConfig(const proto::YourFilter& config);
  bool isEnabled() const;
private:
  proto::YourFilter config_;
};

class YourFilter : public Http::StreamDecoderFilter {
public:
  explicit YourFilter(std::shared_ptr<YourFilterConfig> config);
  
  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(...) override;
  Http::FilterDataStatus decodeData(...) override;
  Http::FilterTrailersStatus decodeTrailers(...) override;
  void setDecoderFilterCallbacks(...) override;
  void onDestroy() override;

private:
  // TODO: Add helper methods
  std::shared_ptr<YourFilterConfig> config_;
  Http::StreamDecoderFilterCallbacks* callbacks_{nullptr};
};

} // namespace YourFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
```

## 3. Filter Implementation

**File**: `src/filters/[filter_name]/[filter_name].cc`

**Must Include**:
- File-level comment
- Implementation of config class
- Implementation of filter class
- Stub methods with TODOs
- Proper error handling
- Return appropriate FilterStatus values

**Example Structure**:
```cpp
#include "src/filters/your_filter/your_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace YourFilter {

YourFilterConfig::YourFilterConfig(const proto::YourFilter& config)
    : config_(config) {
  // TODO: Validate config
}

bool YourFilterConfig::isEnabled() const {
  return !config_.has_enabled() || config_.enabled().value();
}

YourFilter::YourFilter(std::shared_ptr<YourFilterConfig> config)
    : config_(std::move(config)) {}

Http::FilterHeadersStatus YourFilter::decodeHeaders(
    Http::RequestHeaderMap& headers, bool end_stream) {
  
  if (!config_->isEnabled()) {
    return Http::FilterHeadersStatus::Continue;
  }
  
  // TODO: Implement logic
  
  return Http::FilterHeadersStatus::Continue;
}

// TODO: Implement other methods

} // namespace YourFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
```

## 4. Factory Registration

**File**: `src/filters/[filter_name]/config.cc`

**Must Include**:
- File-level comment
- Factory class extending FactoryBase
- Filter creation logic
- REGISTER_FACTORY macro
- TODOs for context usage

**Example Structure**:
```cpp
#include "src/filters/your_filter/your_filter.h"
#include "envoy/registry/registry.h"
#include "source/extensions/filters/http/common/factory_base.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace YourFilter {

class YourFilterFactory
    : public Common::FactoryBase<proto::YourFilter> {
public:
  YourFilterFactory() : FactoryBase("envoy.filters.http.your_filter") {}

private:
  Http::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const proto::YourFilter& proto_config,
      const std::string& stats_prefix,
      Server::Configuration::FactoryContext& context) override {
    
    auto config = std::make_shared<YourFilterConfig>(proto_config);
    
    // TODO: Use context for stats, tracing
    
    return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      callbacks.addStreamDecoderFilter(
          std::make_shared<YourFilter>(config));
    };
  }
};

REGISTER_FACTORY(YourFilterFactory, 
                 Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace YourFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
```

## 5. Unit Test Scaffolding

**File**: `test/filters/[filter_name]/[filter_name]_test.cc`

**Must Include**:
- File-level comment with TODOs
- Test fixture class
- Basic test cases (config, enabled/disabled, decode methods)
- Mock setup
- TODO test cases for future implementation

**Example Structure**:
```cpp
#include "src/filters/your_filter/your_filter.h"
#include "test/mocks/http/mocks.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::NiceMock;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace YourFilter {
namespace {

class YourFilterTest : public testing::Test {
protected:
  void SetUp() override {
    proto::YourFilter config;
    config.mutable_enabled()->set_value(true);
    
    config_ = std::make_shared<YourFilterConfig>(config);
    filter_ = std::make_unique<YourFilter>(config_);
    filter_->setDecoderFilterCallbacks(callbacks_);
  }
  
  std::shared_ptr<YourFilterConfig> config_;
  std::unique_ptr<YourFilter> filter_;
  NiceMock<Http::MockStreamDecoderFilterCallbacks> callbacks_;
};

TEST_F(YourFilterTest, ConfigCreation) {
  EXPECT_TRUE(config_->isEnabled());
}

TEST_F(YourFilterTest, FilterDisabled) {
  // TODO: Test disabled behavior
}

TEST_F(YourFilterTest, DecodeHeadersContinues) {
  Http::TestRequestHeaderMapImpl headers{};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue,
            filter_->decodeHeaders(headers, false));
}

// TODO: Add more tests

} // namespace
} // namespace YourFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
```

# Additional Requirements

## Documentation

Each file must include:
- **File-level comment** explaining purpose
- **TODOs** for future implementation
- **Inline comments** for complex logic
- **Function documentation** for public methods

## Coding Style

Follow these conventions:
- **Namespace**: `Envoy::Extensions::HttpFilters::YourFilter`
- **Class names**: PascalCase (e.g., `YourFilter`, `YourFilterConfig`)
- **Method names**: camelCase (e.g., `decodeHeaders`, `isEnabled`)
- **Member variables**: snake_case with trailing underscore (e.g., `config_`, `callbacks_`)
- **Constants**: UPPER_SNAKE_CASE
- **Indentation**: 2 spaces
- **Line length**: 100 characters max

## Error Handling

- Return appropriate `FilterStatus` values
- Don't throw exceptions in filter methods
- Check for null pointers before dereferencing
- Validate configuration in constructor

## Testing

- Use Google Test and Google Mock
- Create test fixture for common setup
- Test both enabled and disabled states
- Mock Envoy interfaces (callbacks, connections, etc.)
- Add TODO comments for future test cases

# Next Steps Section

After generating the code, provide a "Next Edits Bob Should Make" section with:

1. **BUILD.bazel files** to create
2. **WORKSPACE** configuration
3. **.bazelrc** settings
4. **Verification commands** to run
5. **Next implementation steps** (e.g., Step 2.1, 2.2, etc.)

# Example Output Format

```markdown
# Step 1.1: Filter Skeleton Implementation

## Files Created

### 1. Configuration Proto (`src/config/your_filter.proto`)

[Complete file content]

**Purpose**: Defines configuration schema for the filter
**Key Elements**: Mode enum, enable flag, TODOs for future config
**Next**: Add validation rules and additional options in Step 2

### 2. Filter Header (`src/filters/your_filter/your_filter.h`)

[Complete file content]

**Purpose**: Defines filter interface and config wrapper
**Key Classes**: YourFilterConfig, YourFilter
**TODOs**: Add context extraction, policy engine, metrics

[Continue for all files...]

## Next Edits Bob Should Make

1. **Create BUILD.bazel files**
   - src/config/BUILD.bazel
   - src/filters/your_filter/BUILD.bazel
   - test/filters/your_filter/BUILD.bazel

2. **Create WORKSPACE file**
   - Add Envoy dependency
   - Add proto dependencies
   - Add test dependencies

3. **Verify compilation**
   ```bash
   bazel build //src/filters/your_filter:all
   bazel test //test/filters/your_filter:all
   ```

4. **Next implementation steps**
   - Step 2.1: Implement context extraction
   - Step 2.2: Implement header annotation
   - Step 2.3: Implement policy engine
```

---

Generate the filter skeleton code now.
```

---

## Quick Start Example

### Example: PQC Filter

```markdown
**Filter Purpose**: Negotiate PQC-capable upstreams based on config
**Filter Name**: envoy.filters.http.pqc
**Repository**: Envoy repo at C:/Users/steph/Desktop
**Coding Style**: Match existing Envoy conventions

[Use the main prompt above with these values filled in]
```

### Example: Rate Limiting Filter

```markdown
**Filter Purpose**: Apply rate limiting based on request headers
**Filter Name**: envoy.filters.http.custom_ratelimit
**Repository**: /path/to/envoy
**Coding Style**: Envoy C++17 conventions

[Use the main prompt above with these values filled in]
```

### Example: Authentication Filter

```markdown
**Filter Purpose**: Validate JWT tokens and extract user claims
**Filter Name**: envoy.filters.http.jwt_auth
**Repository**: /workspace/envoy
**Coding Style**: Google C++ Style Guide + Envoy conventions

[Use the main prompt above with these values filled in]
```

---

## Tips for Best Results

### 1. Be Specific About Filter Purpose
- Clearly describe what the filter does
- Mention key features or capabilities
- Specify integration points (TLS, headers, routing, etc.)

### 2. Choose Appropriate Filter Name
- Follow Envoy naming: `envoy.filters.http.[name]`
- Use descriptive, lowercase names
- Avoid generic names like "custom" or "my_filter"

### 3. Specify Configuration Needs
- List required configuration options
- Mention optional settings
- Identify future configuration needs

### 4. Consider Filter Placement
- Decoder filter (request path)
- Encoder filter (response path)
- Both (bidirectional)

### 5. Plan for Testing
- Identify key test scenarios
- Consider edge cases
- Plan for integration tests

---

## Common Patterns

### Pattern 1: Inspection Filter
Inspects requests/responses without modification:
- Read headers/body
- Extract metadata
- Make routing decisions
- Record metrics

### Pattern 2: Transformation Filter
Modifies requests/responses:
- Add/remove headers
- Transform body content
- Rewrite URLs
- Compress/decompress

### Pattern 3: Authentication Filter
Validates credentials:
- Check tokens
- Verify signatures
- Extract claims
- Enforce policies

### Pattern 4: Rate Limiting Filter
Controls request rate:
- Count requests
- Apply limits
- Return 429 on exceed
- Support quotas

### Pattern 5: Caching Filter
Caches responses:
- Check cache
- Store responses
- Validate freshness
- Handle cache keys

---

## Troubleshooting

### Issue: Filter Not Registered
**Solution**: Ensure `alwayslink = 1` in BUILD.bazel for config.cc

### Issue: Proto Not Found
**Solution**: Check proto package name matches import path

### Issue: Compilation Errors
**Solution**: Verify all Envoy includes are correct

### Issue: Tests Fail
**Solution**: Check mock setup and expectations

### Issue: Filter Not Called
**Solution**: Verify filter is added to filter chain in config

---

## Version History

- **v1.0.0** (2026-04-06): Initial code mode prompt template

---

## Related Resources

- [AI Agent Implementation Roadmap Prompt](AI_AGENT_IMPLEMENTATION_ROADMAP_PROMPT.md)
- [Implementation Plan](IMPLEMENTATION_PLAN.md)
- [Step 1 Summary](STEP_1_SUMMARY.md)

---

**End of Code Mode Implementation Prompt Template**