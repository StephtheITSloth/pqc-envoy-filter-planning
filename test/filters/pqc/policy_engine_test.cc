// Policy Engine Unit Tests

#include "src/filters/pqc/policy_engine.h"
#include "src/filters/pqc/context.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace {

TEST(PolicyEngineTest, DefaultAction) {
  pqc::envoy::config::v1::PolicyConfig config;
  config.set_default_action(pqc::envoy::config::v1::ALLOW);
  
  PolicyEngine engine(config);
  PqcContext context;
  
  auto result = engine.evaluate(context, "/test");
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().action(), pqc::envoy::config::v1::ALLOW);
}

// TODO: Add more comprehensive tests

} // namespace
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
