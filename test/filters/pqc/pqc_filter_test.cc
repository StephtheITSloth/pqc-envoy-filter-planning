// PQC Filter Unit Tests

#include "src/filters/pqc/pqc_filter.h"

#include "envoy/common/optref.h"

#include "source/common/http/message_impl.h"

#include "test/mocks/http/mocks.h"
#include "test/mocks/server/mocks.h"
#include "test/mocks/upstream/cluster_manager.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::HasSubstr;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Pqc {
namespace {

class PqcFilterTest : public testing::Test {
protected:
  void SetUp() override {
    const auto proto_config = makeBaseConfig();
    config_ = std::make_shared<PqcFilterConfig>(proto_config);
    filter_ = createFilter(proto_config);
  }

  pqc::envoy::config::v1::PqcFilter makeBaseConfig() const {
    pqc::envoy::config::v1::PqcFilter proto_config;
    proto_config.set_mode(pqc::envoy::config::v1::PqcFilter::HYBRID);
    proto_config.mutable_enabled()->set_value(true);
    return proto_config;
  }

  pqc::envoy::config::v1::PqcFilter
  makePolicyServiceConfig(bool fail_open = true, absl::string_view cluster = "watsonx-policy") const {
    auto proto_config = makeBaseConfig();
    auto* policy_service = proto_config.mutable_policy_service();
    policy_service->set_cluster(std::string(cluster));
    policy_service->set_path("/v1/policy/evaluate");
    policy_service->set_authority("watsonx-policy-service");
    policy_service->mutable_fail_open()->set_value(fail_open);
    policy_service->mutable_timeout()->set_seconds(1);
    return proto_config;
  }

  std::unique_ptr<PqcFilter>
  createFilter(const pqc::envoy::config::v1::PqcFilter& proto_config,
               Upstream::ClusterManager* cluster_manager = nullptr) {
    auto config = std::make_shared<PqcFilterConfig>(proto_config);
    auto filter = std::make_unique<PqcFilter>(config, cluster_manager);
    filter->setDecoderFilterCallbacks(decoder_callbacks_);
    return filter;
  }

  PqcFilterConfigSharedPtr config_;
  std::unique_ptr<PqcFilter> filter_;
  NiceMock<Http::MockStreamDecoderFilterCallbacks> decoder_callbacks_;
  NiceMock<Upstream::MockClusterManager> cluster_manager_;
};

TEST_F(PqcFilterTest, ConfigurationCreation) {
  EXPECT_TRUE(config_->isEnabled());
  EXPECT_EQ(config_->mode(), pqc::envoy::config::v1::PqcFilter::HYBRID);
}

TEST_F(PqcFilterTest, FilterDisabled) {
  pqc::envoy::config::v1::PqcFilter proto_config;
  proto_config.mutable_enabled()->set_value(false);

  auto disabled_filter = createFilter(proto_config);
  Http::TestRequestHeaderMapImpl headers{};

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, disabled_filter->decodeHeaders(headers, false));
}

TEST_F(PqcFilterTest, DecodeHeadersContinues) {
  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/test"},
      {":authority", "example.com"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(headers, false));
}

TEST_F(PqcFilterTest, DecodeDataContinues) {
  Buffer::OwnedImpl data("test data");

  EXPECT_EQ(Http::FilterDataStatus::Continue, filter_->decodeData(data, false));
}

TEST_F(PqcFilterTest, DecodeTrailersContinues) {
  Http::TestRequestTrailerMapImpl trailers{};

  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter_->decodeTrailers(trailers));
}

TEST_F(PqcFilterTest, OnDestroy) {
  filter_->onDestroy();
}

TEST_F(PqcFilterTest, PqcCapableConnection) {
  auto ssl_connection = std::make_shared<NiceMock<Ssl::MockConnectionInfo>>();
  const std::string tls_version = "TLSv1.3";
  const std::string sni = "api.quantum.internal";
  EXPECT_CALL(*ssl_connection, peerCertificatePresented()).WillRepeatedly(Return(true));
  EXPECT_CALL(*ssl_connection, ciphersuiteString())
      .WillRepeatedly(Return("TLS_ML_KEM_768_SHA256"));
  EXPECT_CALL(*ssl_connection, tlsVersion()).WillRepeatedly(ReturnRef(tls_version));
  EXPECT_CALL(*ssl_connection, sni()).WillRepeatedly(ReturnRef(sni));

  auto connection = std::make_shared<NiceMock<Network::MockConnection>>();
  EXPECT_CALL(*connection, ssl()).WillRepeatedly(Return(ssl_connection));
  EXPECT_CALL(decoder_callbacks_, connection())
      .WillRepeatedly(Return(makeOptRef<const Network::Connection>(*connection)));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/agent/decision"},
      {":authority", "payments.internal:8443"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(headers, false));

  const auto& context = filter_->currentContext();
  EXPECT_EQ("true", headers.get_("x-pqc-capable"));
  EXPECT_EQ("TLS_ML_KEM_768_SHA256", headers.get_("x-pqc-cipher"));
  EXPECT_EQ("TLSv1.3", headers.get_("x-pqc-tls-version"));
  EXPECT_EQ("payments.internal", headers.get_("x-pqc-service"));
  EXPECT_EQ("prefer_hybrid", headers.get_("x-pqc-decision"));

  ASSERT_TRUE(context.requestMethod().has_value());
  EXPECT_EQ("GET", context.requestMethod().value());
  ASSERT_TRUE(context.requestPath().has_value());
  EXPECT_EQ("/agent/decision", context.requestPath().value());
  ASSERT_TRUE(context.authority().has_value());
  EXPECT_EQ("payments.internal:8443", context.authority().value());
  ASSERT_TRUE(context.serviceName().has_value());
  EXPECT_EQ("payments.internal", context.serviceName().value());
  ASSERT_TRUE(context.sni().has_value());
  EXPECT_EQ("api.quantum.internal", context.sni().value());
  ASSERT_TRUE(context.clientIp().has_value());
  EXPECT_EQ("10.0.0.3:50000", context.clientIp().value());

  ASSERT_TRUE(filter_->currentDecision().has_value());
  EXPECT_EQ(pqc::envoy::config::v1::PREFER_HYBRID, filter_->currentDecision()->action());
}

TEST_F(PqcFilterTest, ClassicalOnlyConnection) {
  auto ssl_connection = std::make_shared<NiceMock<Ssl::MockConnectionInfo>>();
  const std::string tls_version = "TLSv1.2";
  const std::string sni = "fallback.internal";
  EXPECT_CALL(*ssl_connection, peerCertificatePresented()).WillRepeatedly(Return(true));
  EXPECT_CALL(*ssl_connection, ciphersuiteString())
      .WillRepeatedly(Return("TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"));
  EXPECT_CALL(*ssl_connection, tlsVersion()).WillRepeatedly(ReturnRef(tls_version));
  EXPECT_CALL(*ssl_connection, sni()).WillRepeatedly(ReturnRef(sni));

  auto connection = std::make_shared<NiceMock<Network::MockConnection>>();
  EXPECT_CALL(*connection, ssl()).WillRepeatedly(Return(ssl_connection));
  EXPECT_CALL(decoder_callbacks_, connection())
      .WillRepeatedly(Return(makeOptRef<const Network::Connection>(*connection)));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/test"},
      {":authority", "example.com"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(headers, false));

  EXPECT_EQ("false", headers.get_("x-pqc-capable"));
  EXPECT_EQ("TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256", headers.get_("x-pqc-cipher"));
  EXPECT_EQ("fallback", headers.get_("x-pqc-decision"));
  EXPECT_EQ("true", headers.get_("x-pqc-fallback"));

  ASSERT_TRUE(filter_->currentDecision().has_value());
  EXPECT_EQ(pqc::envoy::config::v1::FALLBACK, filter_->currentDecision()->action());
}

TEST_F(PqcFilterTest, ServiceNameFallsBackToSniWhenAuthorityMissing) {
  auto ssl_connection = std::make_shared<NiceMock<Ssl::MockConnectionInfo>>();
  const std::string tls_version = "TLSv1.3";
  const std::string sni = "agent-gateway.internal";
  EXPECT_CALL(*ssl_connection, ciphersuiteString())
      .WillRepeatedly(Return("TLS_AES_128_GCM_SHA256"));
  EXPECT_CALL(*ssl_connection, tlsVersion()).WillRepeatedly(ReturnRef(tls_version));
  EXPECT_CALL(*ssl_connection, sni()).WillRepeatedly(ReturnRef(sni));

  auto connection = std::make_shared<NiceMock<Network::MockConnection>>();
  EXPECT_CALL(*connection, ssl()).WillRepeatedly(Return(ssl_connection));
  EXPECT_CALL(decoder_callbacks_, connection())
      .WillRepeatedly(Return(makeOptRef<const Network::Connection>(*connection)));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "POST"},
      {":path", "/policy/evaluate"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(headers, false));

  const auto& context = filter_->currentContext();
  ASSERT_TRUE(context.serviceName().has_value());
  EXPECT_EQ("agent-gateway.internal", context.serviceName().value());
  EXPECT_EQ("agent-gateway.internal", headers.get_("x-pqc-service"));
  EXPECT_EQ("fallback", headers.get_("x-pqc-decision"));
}

TEST_F(PqcFilterTest, PlaintextRequestStillCapturesRequestMetadata) {
  auto connection = std::make_shared<NiceMock<Network::MockConnection>>();
  EXPECT_CALL(*connection, ssl()).WillRepeatedly(Return(nullptr));
  EXPECT_CALL(decoder_callbacks_, connection())
      .WillRepeatedly(Return(makeOptRef<const Network::Connection>(*connection)));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "POST"},
      {":path", "/mvp/check"},
      {":authority", "watsonx.ibm.com:443"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter_->decodeHeaders(headers, false));

  const auto& context = filter_->currentContext();
  EXPECT_FALSE(context.isPqcCapable());
  ASSERT_TRUE(context.requestMethod().has_value());
  EXPECT_EQ("POST", context.requestMethod().value());
  ASSERT_TRUE(context.requestPath().has_value());
  EXPECT_EQ("/mvp/check", context.requestPath().value());
  ASSERT_TRUE(context.serviceName().has_value());
  EXPECT_EQ("watsonx.ibm.com", context.serviceName().value());
  EXPECT_EQ("false", headers.get_("x-pqc-capable"));
  EXPECT_EQ("watsonx.ibm.com", headers.get_("x-pqc-service"));
  EXPECT_EQ("fallback", headers.get_("x-pqc-decision"));
  EXPECT_EQ("true", headers.get_("x-pqc-fallback"));
}

TEST_F(PqcFilterTest, PqcOnlyModeRejectsClassical) {
  pqc::envoy::config::v1::PqcFilter proto_config;
  proto_config.set_mode(pqc::envoy::config::v1::PqcFilter::PQC_ONLY);
  proto_config.mutable_enabled()->set_value(true);

  auto pqc_only_filter = createFilter(proto_config);

  auto ssl_connection = std::make_shared<NiceMock<Ssl::MockConnectionInfo>>();
  const std::string tls_version = "TLSv1.2";
  const std::string sni = "legacy.internal";
  EXPECT_CALL(*ssl_connection, ciphersuiteString())
      .WillRepeatedly(Return("TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256"));
  EXPECT_CALL(*ssl_connection, tlsVersion()).WillRepeatedly(ReturnRef(tls_version));
  EXPECT_CALL(*ssl_connection, sni()).WillRepeatedly(ReturnRef(sni));

  auto connection = std::make_shared<NiceMock<Network::MockConnection>>();
  EXPECT_CALL(*connection, ssl()).WillRepeatedly(Return(ssl_connection));
  EXPECT_CALL(decoder_callbacks_, connection())
      .WillRepeatedly(Return(makeOptRef<const Network::Connection>(*connection)));
  EXPECT_CALL(decoder_callbacks_, sendLocalReply(Http::Code::Forbidden, _, _, _, _));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/secure"},
      {":authority", "legacy.internal"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            pqc_only_filter->decodeHeaders(headers, false));
  ASSERT_TRUE(pqc_only_filter->currentDecision().has_value());
  EXPECT_EQ(pqc::envoy::config::v1::DENY, pqc_only_filter->currentDecision()->action());
}

TEST_F(PqcFilterTest, AsyncPolicyServiceSuccessResumesDecoding) {
  auto policy_filter = createFilter(makePolicyServiceConfig(), &cluster_manager_);
  auto async_request =
      std::make_unique<NiceMock<Http::MockAsyncClientRequest>>(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* captured_callbacks = nullptr;
  std::string request_body;

  EXPECT_CALL(cluster_manager_, getThreadLocalCluster("watsonx-policy"))
      .WillOnce(Return(&cluster_manager_.thread_local_cluster_));
  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(Invoke([&](Http::RequestMessagePtr& request, Http::AsyncClient::Callbacks& callbacks,
                           const Http::AsyncClient::RequestOptions&) {
        captured_callbacks = &callbacks;
        request_body = request->bodyAsString();
        EXPECT_EQ("POST", request->headers().getMethodValue());
        EXPECT_EQ("/v1/policy/evaluate", request->headers().getPathValue());
        EXPECT_EQ("watsonx-policy-service", request->headers().getHostValue());
        return async_request.get();
      }));
  EXPECT_CALL(decoder_callbacks_, continueDecoding());

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/agent/decision"},
      {":authority", "payments.internal"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            policy_filter->decodeHeaders(headers, false));
  Buffer::OwnedImpl buffered_body("body");
  EXPECT_EQ(Http::FilterDataStatus::StopIterationAndBuffer,
            policy_filter->decodeData(buffered_body, false));
  ASSERT_NE(captured_callbacks, nullptr);
  EXPECT_THAT(request_body, HasSubstr("\"service_name\":\"payments.internal\""));
  EXPECT_THAT(request_body, HasSubstr("\"request_path\":\"/agent/decision\""));
  EXPECT_THAT(request_body, HasSubstr("\"mode\":\"hybrid\""));

  auto response_headers = Http::createHeaderMap<Http::ResponseHeaderMapImpl>(
      {{Http::Headers::get().Status, "200"}});
  auto response = std::make_unique<Http::ResponseMessageImpl>(std::move(response_headers));
  response->body().add(
      R"({"action":"prefer_hybrid","reason":"Watsonx preferred hybrid routing"})");

  captured_callbacks->onSuccess(*async_request, std::move(response));

  EXPECT_EQ("prefer_hybrid", headers.get_("x-pqc-decision"));
  EXPECT_EQ("Watsonx preferred hybrid routing", headers.get_("x-pqc-decision-reason"));
  EXPECT_EQ("payments.internal", headers.get_("x-pqc-service"));
}

TEST_F(PqcFilterTest, AsyncPolicyServiceFailOpenFallsBackToLocalPolicy) {
  auto policy_filter = createFilter(makePolicyServiceConfig(true), &cluster_manager_);
  auto async_request =
      std::make_unique<NiceMock<Http::MockAsyncClientRequest>>(&cluster_manager_.thread_local_cluster_.async_client_);
  Http::AsyncClient::Callbacks* captured_callbacks = nullptr;

  EXPECT_CALL(cluster_manager_, getThreadLocalCluster("watsonx-policy"))
      .WillOnce(Return(&cluster_manager_.thread_local_cluster_));
  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(Invoke([&](Http::RequestMessagePtr&, Http::AsyncClient::Callbacks& callbacks,
                           const Http::AsyncClient::RequestOptions&) {
        captured_callbacks = &callbacks;
        return async_request.get();
      }));
  EXPECT_CALL(decoder_callbacks_, continueDecoding());

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/test"},
      {":authority", "legacy.internal"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            policy_filter->decodeHeaders(headers, false));
  ASSERT_NE(captured_callbacks, nullptr);

  captured_callbacks->onFailure(*async_request, Http::AsyncClient::FailureReason::Reset);

  EXPECT_EQ("fallback", headers.get_("x-pqc-decision"));
  EXPECT_EQ("true", headers.get_("x-pqc-fallback"));
  EXPECT_THAT(headers.get_("x-pqc-decision-reason"), HasSubstr("Policy service request reset"));
}

TEST_F(PqcFilterTest, MissingPolicyClusterFailsClosedWhenConfigured) {
  auto policy_filter = createFilter(makePolicyServiceConfig(false, "missing-policy-cluster"),
                                    &cluster_manager_);

  EXPECT_CALL(cluster_manager_, getThreadLocalCluster("missing-policy-cluster"))
      .WillOnce(Return(nullptr));
  EXPECT_CALL(decoder_callbacks_, sendLocalReply(Http::Code::ServiceUnavailable, _, _, _, _));

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/secure"},
      {":authority", "payments.internal"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            policy_filter->decodeHeaders(headers, false));
}

TEST_F(PqcFilterTest, OnDestroyCancelsActivePolicyRequest) {
  auto policy_filter = createFilter(makePolicyServiceConfig(), &cluster_manager_);
  auto async_request =
      std::make_unique<NiceMock<Http::MockAsyncClientRequest>>(&cluster_manager_.thread_local_cluster_.async_client_);

  EXPECT_CALL(cluster_manager_, getThreadLocalCluster("watsonx-policy"))
      .WillOnce(Return(&cluster_manager_.thread_local_cluster_));
  EXPECT_CALL(cluster_manager_.thread_local_cluster_.async_client_, send_(_, _, _))
      .WillOnce(Return(async_request.get()));
  EXPECT_CALL(*async_request, cancel());

  Http::TestRequestHeaderMapImpl headers{
      {":method", "GET"},
      {":path", "/agent/decision"},
      {":authority", "payments.internal"},
  };

  EXPECT_EQ(Http::FilterHeadersStatus::StopIteration,
            policy_filter->decodeHeaders(headers, false));

  policy_filter->onDestroy();
}

} // namespace
} // namespace Pqc
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

// Made with Bob
