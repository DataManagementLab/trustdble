/** @defgroup group12 adapter_ethereum_test
 *  @ingroup group1
 *  @{
 */

/**
 * @file
 * @brief This file contains simple test for ethereum adapter.
 *
 */

#include "adapter_ethereum/adapter_ethereum.h"
#include "adapter_interface_test.h"

//! Instantiate AdapterInterfaceTest suite.
//! NOLINTNEXTLINE(modernize-use-trailing-return-type)
INSTANTIATE_TEST_SUITE_P(
    EthereumAdapterTests, AdapterInterfaceTest,
    testing::Values(std::make_tuple(
        std::make_shared<EthereumAdapter>(),
        std::make_shared<EthereumAdapter>(), "./test-config.ini",
        "{     \"Network\": {         \"rpc-port\": \"8000\",         "
        "\"peer-port\": \"30303\",         \"join-ip\": \"172.17.0.1\",        "
        " \"enode\": "
        "\"enode:"
        "20e1163d1474178cb2a61b7daffa9c82b36fd62c8fbc88fa384bdcb49f8703ef23ffc8"
        "356109b98f6a3f3415018aa8113b58973547a0914228133282137a18cb@172.17.0.1:"
        "30303\"     } } ")));


//! Single Test-Cases - TEST(test_suite_name, test_name)
TEST(EthereumAdapterTests /*unused*/, singleethereumtest /*unused*/) {
  EXPECT_EQ(5, 5);
  EXPECT_TRUE(true);
}
/** @} */