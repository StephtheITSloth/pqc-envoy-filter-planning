// PQC Context Unit Tests

#include "src/filters/pqc/context.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace {

TEST(PqcContextTest, DefaultConstruction) {
  PqcContext context;
  EXPECT_FALSE(context.isPqcCapable());
  EXPECT_FALSE(context.hasFallback());
  EXPECT_FALSE(context.negotiatedCipher().has_value());
}

TEST(PqcContextTest, SetPqcCapable) {
  PqcContext context;
  context.setPqcCapable(true);
  EXPECT_TRUE(context.isPqcCapable());
}

TEST(PqcContextTest, SetNegotiatedCipher) {
  PqcContext context;
  context.setNegotiatedCipher("TLS_ML_KEM_768_SHA256");
  ASSERT_TRUE(context.negotiatedCipher().has_value());
  EXPECT_EQ(context.negotiatedCipher().value(), "TLS_ML_KEM_768_SHA256");
}

// TODO: Add more comprehensive tests

} // namespace
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
