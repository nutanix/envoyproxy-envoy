#include "test/integration/reverse_conn_test.h"

#include <string>

#include "source/common/http/header_map_impl.h"

#include "gtest/gtest.h"

namespace Envoy {

INSTANTIATE_TEST_SUITE_P(IpVersions, ReverseConnTest,
                         testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                         TestUtility::ipTestParamsToString);

TEST_P(ReverseConnTest, TestReverseConn) {
  initialize();

  codec_client_ = makeHttpConnection(makeClientConnection(lookupPort("http")));

  // create reverse connection
  std::string path =
      "/reverse_connections" +
      Http::Utility::queryParamsToString(Http::Utility::QueryParams(
          {{"node_id", "host_1"}, {"remote_cluster", "cloud_envoy"}, {"cluster_id", "cluster_1"}}));

  auto response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "POST"},
      {":path", path},
      {":scheme", "http"},
      {":authority", "no-headers.com"},
  });

  waitForNextUpstreamRequest(0, TestUtility::DefaultTimeout);
  // Send response headers, and end_stream if there is no response body.
  upstream_request_->encodeHeaders(
      Http::TestResponseHeaderMapImpl{
          {":status", "200"},
          {"content-length", "27"},
          {"content-type", "text/plain"},
          {"server", "envoy"},
          {"custom-header-to-increase-response-size", "custom-header-value"},
      },
      false);
  // Send any response data, with end_stream true.
  Buffer::OwnedImpl response_data("reverse connection accepted");
  upstream_request_->encodeData(response_data, false);

  // Wait for the response to be read by the codec client.
  ASSERT_TRUE(response->waitForEndStream());

  EXPECT_TRUE(response->complete());
  EXPECT_STREQ("reversed", response->body().c_str());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());

  // get reverse connection info
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[],\"connected\":[\"cloud_envoy\"]}", response->body().c_str());

  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections?remote_cluster=cloud_envoy"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"available_connections\":1}", response->body().c_str());

  // delete reverse connections
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "DELETE"},
                                     {":path", "/reverse_connections?remote_cluster=cloud_envoy"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());

  // get again
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[],\"connected\":[]}", response->body().c_str());

  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections?remote_cluster=cloud_envoy"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"available_connections\":0}", response->body().c_str());
}

TEST_P(ReverseConnTest, TestReverseConnUnknownCluster) {
  initialize();

  codec_client_ = makeHttpConnection(makeClientConnection(lookupPort("http")));
  std::string path = "/reverse_connections" +
                     Http::Utility::queryParamsToString(
                         Http::Utility::QueryParams({{"node_id", "host_1"},
                                                     {"remote_cluster", "unknown_cluster"},
                                                     {"cluster_id", "cluster_1"}}));
  auto response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "POST"},
      {":path", path},
      {":scheme", "http"},
      {":authority", "no-headers.com"},
  });
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("500", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("cluster unknown_cluster not found", response->body().c_str());
}

TEST_P(ReverseConnTest, GetUnknownCluster) {
  initialize();

  codec_client_ = makeHttpConnection(makeClientConnection(lookupPort("http")));
  auto response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "GET"},
      {":path", "/reverse_connections?remote_cluster=unknown_cluster"},
      {":scheme", "http"},
      {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"available_connections\":0}", response->body().c_str());
}

TEST_P(ReverseConnTest, DeleteUnknownCluster) {
  initialize();

  codec_client_ = makeHttpConnection(makeClientConnection(lookupPort("http")));
  auto response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "DELETE"},
      {":path", "/reverse_connections?remote_cluster=unknown_cluster"},
      {":scheme", "http"},
      {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
}

} // namespace Envoy
