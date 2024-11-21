#pragma once

#include "test/integration/http_integration.h"

#include "gtest/gtest.h"

namespace Envoy {

const std::string REVERSE_CONN_CLOUD_CONFIG = R"EOF(
admin:
  access_log_path: /dev/null
  address:
    socket_address:
      address: 127.0.0.1
      port_value: 0
layered_runtime:
  layers:
  - name: layer
    static_layer:
      envoy.reloadable_features.http_filter_avoid_reentrant_local_reply: false
static_resources:
  clusters:
    - name: dc_envoy
      load_assignment:
        cluster_name: dc_envoy
        endpoints:
        - lb_endpoints:
          - endpoint:
              address:
                socket_address:
                  address: 127.0.0.1
                  port_value: 0
  listeners:
    - name: listener_rc
      address:
        socket_address:
          address: 127.0.0.1
          port_value: 0
      filter_chains:
        filters:
          name: envoy.http_connection_manager
          typed_config:
            "@type": type.googleapis.com/envoy.extensions.filters.network.http_connection_manager.v3.HttpConnectionManager
            stat_prefix: config_test
            http_filters:
              - name: reverse_conn
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.reverse_conn.v3.ReverseConn
                  ping_interval: 2
              - name: envoy.filters.http.router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
            codec_type: HTTP1
            route_config:
              virtual_hosts:
                name: integration
                routes:
                  route:
                    cluster: dc_envoy
                  match:
                    prefix: "/listener/cloud_envoy"
                domains: "*"
              name: route_config_0
)EOF";

class ReverseConnCloudTest : public HttpIntegrationTest,
                             public testing::TestWithParam<Network::Address::IpVersion> {
public:
  ReverseConnCloudTest()
      : HttpIntegrationTest(Http::CodecClient::Type::HTTP1, GetParam(), REVERSE_CONN_CLOUD_CONFIG) {
  }
};
} // namespace Envoy
