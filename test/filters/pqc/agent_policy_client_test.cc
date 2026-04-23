// Agent Policy Client Unit Tests

#include "src/filters/pqc/agent_policy_client.h"

#include "gtest/gtest.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace {

TEST(LocalAgentPolicyClientTest, HybridModePrefersHybridForPqcCapableDownstream) {
  pqc::envoy::config::v1::PolicyConfig config;
  LocalAgentPolicyClient client(config);

  AgentPolicyRequest request;
  request.request_path = "/agent/decision";
  request.service_name = "payments.internal";
  request.pqc_capable = true;
  request.mode = pqc::envoy::config::v1::PqcFilter::HYBRID;

  auto result = client.evaluate(request);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(pqc::envoy::config::v1::PREFER_HYBRID, result->action());
}

TEST(LocalAgentPolicyClientTest, HybridModeFallsBackForClassicalDownstream) {
  pqc::envoy::config::v1::PolicyConfig config;
  LocalAgentPolicyClient client(config);

  AgentPolicyRequest request;
  request.request_path = "/agent/decision";
  request.service_name = "payments.internal";
  request.mode = pqc::envoy::config::v1::PqcFilter::HYBRID;

  auto result = client.evaluate(request);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(pqc::envoy::config::v1::FALLBACK, result->action());
}

TEST(LocalAgentPolicyClientTest, PqcOnlyModeDeniesClassicalDownstream) {
  pqc::envoy::config::v1::PolicyConfig config;
  LocalAgentPolicyClient client(config);

  AgentPolicyRequest request;
  request.request_path = "/secure";
  request.mode = pqc::envoy::config::v1::PqcFilter::PQC_ONLY;

  auto result = client.evaluate(request);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(pqc::envoy::config::v1::DENY, result->action());
}

TEST(LocalAgentPolicyClientTest, ServiceRuleRequiresPqcButAllowsFallback) {
  pqc::envoy::config::v1::PolicyConfig config;
  config.add_require_pqc_services("watsonx.ibm.com");
  config.add_allow_fallback_services("watsonx.ibm.com");

  LocalAgentPolicyClient client(config);

  AgentPolicyRequest request;
  request.request_path = "/mvp/check";
  request.service_name = "watsonx.ibm.com";
  request.mode = pqc::envoy::config::v1::PqcFilter::HYBRID;

  auto result = client.evaluate(request);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(pqc::envoy::config::v1::FALLBACK, result->action());
}

TEST(LocalAgentPolicyClientTest, RoutePolicyWinsByPriority) {
  pqc::envoy::config::v1::PolicyConfig config;
  auto* broad = config.add_route_policies();
  broad->set_route_pattern("/agent/*");
  broad->set_action(pqc::envoy::config::v1::ALLOW);
  broad->set_priority(1);

  auto* specific = config.add_route_policies();
  specific->set_route_pattern("/agent/secure/*");
  specific->mutable_require_pqc()->set_value(true);
  specific->set_priority(10);

  LocalAgentPolicyClient client(config);

  AgentPolicyRequest request;
  request.request_path = "/agent/secure/check";
  request.mode = pqc::envoy::config::v1::PqcFilter::HYBRID;

  auto result = client.evaluate(request);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(pqc::envoy::config::v1::DENY, result->action());
  EXPECT_EQ("/agent/secure/*", result->matched_route());
}

} // namespace
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
