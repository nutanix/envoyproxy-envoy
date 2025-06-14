#include "contrib/reverse_connection/bootstrap/source/reverse_connection_address.h"

#include "source/common/common/fmt.h"

namespace Envoy {
namespace Extensions {
namespace Bootstrap {
namespace ReverseConnection {

ReverseConnectionAddress::ReverseConnectionAddress(const ReverseConnectionConfig& config)
    : config_(config) {
  
  // Create address string from reverse connection config
  address_string_ = fmt::format("rc://{}:{}:{}@{}:{}", 
                               config.src_node_id, config.src_cluster_id, config.src_tenant_id,
                               config.remote_cluster, config.connection_count);
  
  // Set logical name to be the same as address string
  logical_name_ = address_string_;

  ENVOY_LOG_MISC(info, "Reverse connection address: {}", address_string_);
}

bool ReverseConnectionAddress::operator==(const Instance& rhs) const {
  const auto* reverse_conn_addr = dynamic_cast<const ReverseConnectionAddress*>(&rhs);
  if (reverse_conn_addr == nullptr) {
    return false;
  }
  return config_.src_node_id == reverse_conn_addr->config_.src_node_id &&
         config_.src_cluster_id == reverse_conn_addr->config_.src_cluster_id &&
         config_.src_tenant_id == reverse_conn_addr->config_.src_tenant_id &&
         config_.remote_cluster == reverse_conn_addr->config_.remote_cluster &&
         config_.connection_count == reverse_conn_addr->config_.connection_count;
}

const std::string& ReverseConnectionAddress::asString() const {
  return address_string_;
}

absl::string_view ReverseConnectionAddress::asStringView() const {
  return address_string_;
}

const std::string& ReverseConnectionAddress::logicalName() const {
  return logical_name_;
}

const Network::Address::Ip* ReverseConnectionAddress::ip() const {
  // Reverse connection addresses don't have real IP
  return nullptr;
}

const sockaddr* ReverseConnectionAddress::sockAddr() const {
  // Reverse connection addresses don't have real socket address
  return nullptr;
}

socklen_t ReverseConnectionAddress::sockAddrLen() const {
  return 0;
}

} // namespace ReverseConnection
} // namespace Bootstrap
} // namespace Extensions
} // namespace Envoy