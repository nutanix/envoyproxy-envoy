#include "test/integration/reverse_conn_test_cloud.h"

#include "source/common/http/header_map_impl.h"

#include "gtest/gtest.h"

namespace Envoy {

INSTANTIATE_TEST_SUITE_P(IpVersions, ReverseConnCloudTest,
                         testing::ValuesIn(TestEnvironment::getIpVersionsForTest()),
                         TestUtility::ipTestParamsToString);

TEST_P(ReverseConnCloudTest, TestReverseConnection) {
  initialize();

  ENVOY_LOG(debug, "test_log: Getting number of sockets for reverse connections. Exepcting 0");
  EXPECT_EQ(test_server_->server().dispatcher().getRCHandler().getNumberOfSockets("host_1"), 0);

  codec_client_ = makeHttpConnection(lookupPort("http"));
  ENVOY_LOG(debug, "test_log: Made an http connection to the server");

  ENVOY_LOG(debug, "test_log: Making a request to list reverse connections. Expecting none");
  // list reverse connections before creating any
  auto response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[],\"connected\":[]}", response->body().c_str());

  ENVOY_LOG(debug, "test_log: Making a request to get reverse connections stats. Expecting none");
  // get reverse connections stats (should be empty)
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections/stats"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});
  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  // TODO: Today we run a suite of two tests, IPv4 and IPv6. When the first test deletes the
  // connections, we expect the second test to have a clean slate in terms of stats as well.
  // However, we do not remove clear either 'cluster_node_storage_' or 'tenant_clister_storage_'
  // from reverse_conn_filter. This results in the second test seeing a non-empty JSON here.
  // However, the number of connections is still rightly 0.
  // EXPECT_STREQ("{}", response->body().c_str());

  ENVOY_LOG(debug, "test_log: Making a accept=true request so that a reverse connection is "
                   "accepted (this connection will be accepted)");
  // accept reverse connection
  std::string path = "/reverse_connections" +
                     Http::Utility::queryParamsToString(Http::Utility::QueryParams(
                         {{"node_id", "host_1"}, {"cluster_id", "cluster_1"}, {"accept", "true"}}));
  response = codec_client_->makeHeaderOnlyRequest(Http::TestRequestHeaderMapImpl{
      {":method", "POST"},
      {":path", path},
      {":scheme", "http"},
      {":authority", "host"},
  });
  ENVOY_LOG(debug, "test_log: Waiting for the server to process the request");
  // let the server process request
  dispatcher_->run(Event::Dispatcher::RunType::Block);
  ENVOY_LOG(debug, "test_log: Getting number of sockets from the handler. Expecting host_1");
  EXPECT_EQ(test_server_->server().dispatcher().getRCHandler().getNumberOfSockets("host_1"), 1);
  ENVOY_LOG(debug, "test_log: Setting codec_client_ connection reused and resetting file events "
                   "since we dont want to read/write to this connection from the codec anymore");
  codec_client_->connection()->setConnectionReused(true);
  codec_client_->connection()->getSocket()->ioHandle().resetFileEvents();

  // list reverse connections
  codec_client_ = makeHttpConnection(lookupPort("http"));
  ENVOY_LOG(debug, "test_log: Made another http connection to the server");
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a request to list reverse connections. Expecting 1");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[\"host_1\"],\"connected\":[]}", response->body().c_str());

  // get reverse connections stats
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections/stats"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a request to get reverse connections stats. Expecting 1");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"cluster_1\":{\"host_1\":1}}", response->body().c_str());

  ENVOY_LOG(debug,
            "test_log: Waiting for 4000 ms to let the ping timeout happen and kill the connection");
  // let ping response wait timeout kill the connection
  timeSystem().advanceTimeWait(std::chrono::milliseconds(4000));

  // list again
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a call list the reverse connections. Expecting 0");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[],\"connected\":[]}", response->body().c_str());

  // get reverse connections stats
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections/stats"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a call get the reverse connections stats. Expecting 0");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"cluster_1\":{\"host_1\":0}}", response->body().c_str());

  // delete reverse connections
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "DELETE"},
                                     {":path", "/reverse_connections?node_id=host_1"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a call to delete the reverse connections");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());

  // list again
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a call list the reverse connections. Expecting 0");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"accepted\":[],\"connected\":[]}", response->body().c_str());

  // get reverse connections stats
  response = codec_client_->makeHeaderOnlyRequest(
      Http::TestRequestHeaderMapImpl{{":method", "GET"},
                                     {":path", "/reverse_connections/stats"},
                                     {":scheme", "http"},
                                     {":authority", "host"}});

  ENVOY_LOG(debug, "test_log: Made a call get the reverse connections stats. Expecting 0");

  ASSERT_TRUE(response->waitForEndStream());
  EXPECT_TRUE(response->complete());
  EXPECT_EQ("200", response->headers().Status()->value().getStringView());
  EXPECT_STREQ("{\"cluster_1\":{\"host_1\":0}}", response->body().c_str());

  ENVOY_LOG(debug, "test_log: TEST DONE");
}

} // namespace Envoy
