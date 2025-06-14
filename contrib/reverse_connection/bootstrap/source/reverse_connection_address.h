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
  Network::Address::Type type() const override { return Network::Address::Type::Ip; } // Masquerade as IP to avoid breaking checks
  const std::string& asString() const override;
  absl::string_view asStringView() const override;
  const std::string& logicalName() const override;
  const Network::Address::Ip* ip() const override;
  const Network::Address::Pipe* pipe() const override { return nullptr; }
  const Network::Address::EnvoyInternalAddress* envoyInternalAddress() const override { return nullptr; }
  const sockaddr* sockAddr() const override;
  socklen_t sockAddrLen() const override;
  absl::string_view addressType() const override { return "reverse_connection"; }
  const Network::SocketInterface& socketInterface() const override { return Network::SocketInterfaceSingleton::get(); }

  // Accessor for reverse connection config
  const ReverseConnectionConfig& reverseConnectionConfig() const { return config_; }

private:
  ReverseConnectionConfig config_;
  std::string address_string_;
  std::string logical_name_;
};

} // namespace ReverseConnection
} // namespace Bootstrap
} // namespace Extensions
} // namespace Envoy 