// Fallback Manager Unit Tests

#include "src/filters/pqc/fallback_manager.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace {

TEST(FallbackManagerTest, NoFallbackNeeded) {
  FallbackManager manager;
  PqcContext context;
  context.setPqcCapable(true);
  
  auto result = manager.triggerFallback(context);
  ASSERT_TRUE(result.ok());
  EXPECT_FALSE(result.value().should_fallback);
}

} // namespace
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
