#include <memory>

#include "source/extensions/bootstrap/reverse_connection/reverse_connection_initiator.h"
#include "source/extensions/bootstrap/reverse_connection/reverse_connection_manager_impl.h"

#include "test/mocks/event/mocks.h"
#include "test/mocks/network/mocks.h"
#include "test/mocks/stats/mocks.h"
#include "test/test_common/test_runtime.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace Bootstrap {
namespace ReverseConnection {
namespace {

class ReverseConnectionManagerImplTest : public testing::Test {
public:
  ReverseConnectionManagerImplTest()
      : dispatcher_(std::make_unique<NiceMock<Event::MockDispatcher>>()),
        rc_manager_(std::make_shared<
                    Envoy::Extensions::Bootstrap::ReverseConnection::ReverseConnectionManagerImpl>(
            &dispatcher_)) {}

  std::unique_ptr<NiceMock<Event::MockDispatcher>> dispatcher_;
  std::shared_ptr<Envoy::Extensions::Bootstrap::ReverseConnection::ReverseConnectionManagerImpl>
      rc_manager_;
  NiceMock<Stats::MockScope> scope_;
  NiceMock<Network::MockListenerConfig> listener_config_;
  std::unique_ptr<ReverseConnectionInitiator> rc_initiator_;
};

TEST_F(ReverseConnectionManagerImplTest, InitializeStats) {
  EXPECT_CALL(scope_, createScope_("reverse_conn_manager."))
      .WillOnce(Return(std::make_shared<Stats::MockScope>()));
  rc_manager_->initializeStats(scope_);
}

// TEST_F(ReverseConnectionManagerImplTest, RegisterRCInitiators) {
//   auto reverse_conn_params =
//   std::make_unique<Network::ReverseConnectionListenerConfig::ReverseConnParams>();
//   reverse_conn_params->src_node_id_ = "node_id";
//   reverse_conn_params->src_cluster_id_ = "cluster_id";
//   reverse_conn_params->src_tenant_id_ = "tenant_id";
//   reverse_conn_params->remote_cluster_to_conn_count_map_ = {{"remote_cluster", 1}};

//   EXPECT_CALL(listener_config_, reverseConnectionListenerConfig())
//       .WillRepeatedly(Return(&reverse_connection_listener_config_));
//   EXPECT_CALL(reverse_connection_listener_config_, getReverseConnParams())
//       .WillRepeatedly(ReturnRef(reverse_conn_params));

//   // Expectations for the debug log statement
//   EXPECT_CALL(listener_config_, name()).WillRepeatedly(Return("test_listener"));
//   EXPECT_CALL(listener_config_, listenerTag()).WillRepeatedly(Return(123));
//   EXPECT_CALL(listener_config_, versionInfo()).WillRepeatedly(Return("test_version"));
//   EXPECT_CALL(*dispatcher_, name()).WillRepeatedly(Return("test_dispatcher"));

//   // Expectation for findOrCreateRCInitiator
//   EXPECT_CALL(rc_manager_, findOrCreateRCInitiator(_, _, _, _, _))
//       .WillOnce(Invoke([&](const Network::ListenerConfig& listener_ref, const std::string&
//       node_id, const std::string& cluster_id,
//                                  const std::string& tenant_id, const std::string& remote_cluster)
//                                  {
//         return std::make_unique<ReverseConnectionInitiator>(node_id, cluster_id, tenant_id,
//         remote_cluster);
//       }));

//   manager_.registerRCInitiators(listener_config_);
// }

TEST_F(ReverseConnectionManagerImplTest, RegisterRCInitiators) {
  // Setup mocks for ListenerConfig and ReverseConnectionListenerConfig
  NiceMock<Network::MockReverseConnectionListenerConfig> reverse_connection_listener_config;
  Network::ReverseConnectionListenerConfig::ReverseConnParams reverse_conn_params = {
      .src_node_id_ = "node-1",
      .src_cluster_id_ = "cluster-1",
      .src_tenant_id_ = "tenant-1",
      .remote_cluster_to_conn_count_map_ = {{"remote-cluster-1", 10}, {"remote-cluster-2", 5}}};

  // Setup ListenerConfig to return ReverseConnectionListenerConfig
  EXPECT_CALL(listener_config_, reverseConnectionListenerConfig())
      .WillRepeatedly(Return(&reverse_connection_listener_config));

  // Setup ReverseConnectionListenerConfig to return ReverseConnectionParams
  EXPECT_CALL(reverse_connection_listener_config, getReverseConnParams())
      .WillRepeatedly(Return(&reverse_conn_params));

  // Set up listener properties
  const std::string listener_name = "test-listener";
  const uint64_t listener_tag = 1234;
  const std::string version_info = "v1";

  EXPECT_CALL(listener_config_, name()).WillRepeatedly(ReturnRef(listener_name));
  EXPECT_CALL(listener_config_, listenerTag()).WillRepeatedly(Return(listener_tag));
  EXPECT_CALL(listener_config_, versionInfo()).WillRepeatedly(ReturnRef(version_info));

  // Mock Dispatcher to validate post actions
  EXPECT_CALL(*dispatcher_, post(_)).WillOnce(Invoke([](std::function<void()> cb) {
    cb();
  })); // Execute posted callback immediately

  // Expectations for creating a new ReverseConnectionInitiator
  EXPECT_CALL(*this, createRCInitiatorDone(_))
      .WillOnce(Invoke([](ReverseConnectionInitiator* initiator) {
        ASSERT_NE(initiator, nullptr);
        ENVOY_LOG(debug, "RC Initiator successfully created");
      }));

  // Call the method under test
  rc_manager_->registerRCInitiators(listener_config_);

  // Verify that the ReverseConnectionInitiator was created and added to the map
  auto it = rc_manager_->available_rc_initiators_.find(listener_tag);
  EXPECT_NE(it, rc_manager_->available_rc_initiators_.end());
  //   EXPECT_EQ(it->second->getOptions().src_node_id, src_node_id);
  //   EXPECT_EQ(it->second->getOptions().src_cluster_id, src_cluster_id);
  //   EXPECT_EQ(it->second->getOptions().src_tenant_id, src_tenant_id);
  //   EXPECT_EQ(it->second->getOptions().remote_cluster_to_conn_count,
  //   remote_cluster_to_conn_count_map);
}

} // namespace
} // namespace ReverseConnection
} // namespace Bootstrap
} // namespace Extensions
} // namespace Envoy
