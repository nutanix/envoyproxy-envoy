#pragma once

#include "envoy/extensions/filters/listener/proxy_protocol/v3/proxy_protocol.pb.h"

#include "test/integration/http_integration.h"

#include "gtest/gtest.h"

namespace Envoy {

const std::string REVERSE_CONN_DC_CONFIG = R"EOF(
admin:
  access_log_path: /dev/null
  address:
    socket_address:
      address: 127.0.0.1
      port_value: 0
static_resources:
  clusters:
    - name: cloud_envoy
      load_assignment:
        cluster_name: cloud_envoy
        endpoints:
        - lb_endpoints:
          - endpoint:
              address:
                socket_address:
                  address: 127.0.0.1
                  port_value: 0
  listeners:
    - name: http_listener
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
              - name: router
                typed_config:
                  "@type": type.googleapis.com/envoy.extensions.filters.http.router.v3.Router
            codec_type: HTTP1
            route_config:
              virtual_hosts:
                name: integration
                routes:
                  route:
                    cluster: cloud_envoy
                  match:
                    prefix: "/reverse_conn"
                domains: "*"
              name: route_config_0
)EOF";

class ReverseConnTest : public HttpIntegrationTest,
                        public testing::TestWithParam<Network::Address::IpVersion> {
public:
  ReverseConnTest()
      : HttpIntegrationTest(Http::CodecClient::Type::HTTP1, GetParam(), REVERSE_CONN_DC_CONFIG) {
    config_helper_.addConfigModifier(
        [&](envoy::config::bootstrap::v3::Bootstrap& bootstrap) -> void {
          auto* rc_listener = bootstrap.mutable_static_resources()->add_listeners();
          rc_listener->set_name("rc_listener");
          auto* socket_address = rc_listener->mutable_address()->mutable_socket_address();
          socket_address->set_address("127.0.0.1");
          socket_address->set_port_value(0);
          // Add an empty filter chain.
          rc_listener->add_filter_chains();
          auto* listener_filter = rc_listener->add_listener_filters();
          listener_filter->set_name("envoy.filters.listener.proxy_protocol");
          envoy::extensions::filters::listener::proxy_protocol::v3::ProxyProtocol config;
          listener_filter->mutable_typed_config()->PackFrom(config);
        });
  }
};
} // namespace Envoy
