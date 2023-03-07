/** @defgroup group34 adapter_fabric_test
 *  @ingroup group3
 *  @{
 */

 /**
 * @file
 * @brief This file contains simple test for adapter fabric.
 *
 */

//#include "adapter_interface_test.h"

#include "adapter_fabric/adapter_fabric.h"
#include "adapter_interface_test.h"

//! Instantiate AdapterInterfaceTest suite.
//! NOLINTNEXTLINE(modernize-use-trailing-return-type)
INSTANTIATE_TEST_SUITE_P(
    FabricAdapterTests, AdapterInterfaceTest,
    testing::Values(std::make_tuple(
        std::make_shared<FabricAdapter>(), std::make_shared<FabricAdapter>(),
        "./test-config.ini",
        "{     \"Network\": {         \"channel_name\": "
        "\"test-5548275e-2228-4676-8d9c-c24011cee155\",         \"msp_id\": "
        "\"Org1MSP\",         \"cert_path\": "
        "\"~/TrustDBle/fabric-samples/test-network/organizations/"
        "peerOrganizations/org1.example.com/users/User1@org1.example.com/msp/"
        "signcerts/cert.pem\",         \"key_path\": "
        "\"~/TrustDBle/fabric-samples/test-network/organizations/"
        "peerOrganizations/org1.example.com/users/User1@org1.example.com/msp/"
        "keystore/\",         \"tls_cert_path\": "
        "\"~/TrustDBle/fabric-samples/test-network/organizations/"
        "peerOrganizations/org1.example.com/peers/test/tls/"
        "ca.crt\",         \"gateway_peer\": \"test\",    "
        "\"test_network_path\": "
        "\"~/TrustDBle/fabric-samples/test-network\",     "
        "\"peer_port\": \"7056\",         \"peer_operations_port\": \"9446\",  "
        "       \"peer_endpoint\": \"localhost:7056\"     } }")));


//! Single Test-Cases - TEST(test_suite_name, test_name)
TEST(FabricAdapterTests, singlefabrictest) {
  EXPECT_EQ(5, 5);
  EXPECT_TRUE(true);
}

/**********************************************
 *  Tests for interactions between two adapters
 * running at the same time
 ***********************************************/

/**
 * @brief Test that operations using one adapter still work after closing
 * another
 *
 */
//! NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, TableScanAfterClose /*unused*/) {
  EXPECT_TRUE(adapter1_->shutdown());

  EXPECT_EQ(adapter0_->get_all(result_map_), 0);
  ASSERT_EQ(result_map_.size(), 3);
  std::map<const BYTES, BYTES>::iterator it;
  int i = 0;
  for (it = result_map_.begin(); it != result_map_.end(); it++) {
    EXPECT_EQ(it->first, keys_[i]);
    EXPECT_EQ(it->second, values_[i++]);
  }
}
/** @} */
