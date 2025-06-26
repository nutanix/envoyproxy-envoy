#pragma once

#include "envoy/network/address.h"
#include "source/common/network/socket_interface.h"

namespace Envoy {
namespace Extensions {
namespace Bootstrap {
namespace ReverseConnection {

/**
 * Custom address type that embeds reverse connection metadata.
 */
class ReverseConnectionAddress : public Network::Address::Instance {
public:
  // Struct to hold reverse connection configuration
  struct ReverseConnectionConfig {
    std::string src_node_id;
    std::string src_cluster_id;
    std::string src_tenant_id;
    std::string remote_cluster;
    uint32_t connection_count;
  };

  ReverseConnectionAddress(const ReverseConnectionConfig& config);

  // Network::Address::Instance
  bool operator==(const Instance& rhs) const override;
  Network::Address::Type type() const override { return Network::Address::Type::Ip; } // Use IP type with our custom IP implementation
  const std::string& asString() const override;
  absl::string_view asStringView() const override;
  const std::string& logicalName() const override;
  const Network::Address::Ip* ip() const override { return &ip_; }
  const Network::Address::Pipe* pipe() const override { return nullptr; }
  const Network::Address::EnvoyInternalAddress* envoyInternalAddress() const override { 
    return nullptr; 
  }
  const sockaddr* sockAddr() const override;
  socklen_t sockAddrLen() const override;
  absl::string_view addressType() const override { return "reverse_connection"; }
  const Network::SocketInterface& socketInterface() const override { return Network::SocketInterfaceSingleton::get(); }

  // Accessor for reverse connection config
  const ReverseConnectionConfig& reverseConnectionConfig() const { return config_; }

private:
  // Simple IPv4 implementation for reverse connection addresses
  struct ReverseConnectionIp : public Network::Address::Ip {
    const std::string& addressAsString() const override { return address_string_; }
    bool isAnyAddress() const override { return false; }
    bool isUnicastAddress() const override { return true; }
    const Network::Address::Ipv4* ipv4() const override { return nullptr; }
    const Network::Address::Ipv6* ipv6() const override { return nullptr; }
    uint32_t port() const override { return 0; }
    Network::Address::IpVersion version() const override { return Network::Address::IpVersion::v4; }
    
    std::string address_string_{"127.0.0.1"}; // Use localhost as default
  };

  ReverseConnectionConfig config_;
  std::string address_string_;
  std::string logical_name_;
  ReverseConnectionIp ip_;
};

} // namespace ReverseConnection
} // namespace Bootstrap
} // namespace Extensions
} // namespace Envoy 