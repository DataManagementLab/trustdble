/** @defgroup group23 stub_test
 *  @ingroup group2
 *  @{
 */

 /**
 * @file
 * @brief This file contains simple test for stub adapter.
 *
 */
#include "adapter_interface_test.h"
#include "adapter_stub/adapter_stub.h"

//! Instantiate AdapterInterface_Test suite.
//! NOLINTNEXTLINE(modernize-use-trailing-return-type)
INSTANTIATE_TEST_SUITE_P(
    StubAdapterTests, AdapterInterfaceTest,
    testing::Values(std::make_tuple(
        std::make_shared<StubAdapter>(), std::make_shared<StubAdapter>(),
        "./test-config.ini", "{\"Network\":{\"network-name\":\"db\"}}")));

//! Single Test-Cases - TEST(test_suite_name, test_name)
TEST(StubAdapterTests, willsucceed) {
  EXPECT_EQ(5, 5);
  EXPECT_TRUE(true);
}

/*
TEST(StubAdapterTests, initTest) {
  StubAdapter stub = StubAdapter();
  const int blocksize = 10;
  std::string tableAddress = "";

  std::string test_folder = "./test_data";
  // Create folder "test_data" manually
  if (!std::filesystem::exists(test_folder)) {
    std::filesystem::create_directory(test_folder);
  }

  std::string db_folder = "./test_data/db";
  // Create folder "db" manually
  if (!std::filesystem::exists(db_folder)) {
    std::filesystem::create_directory(db_folder);
  }

  bool stub_init_succ =
      stub.init("./test-config.ini", "{\"Network\":{\"network-name\":\"db\"}}");
  EXPECT_EQ(stub_init_succ, true);
  stub.create_table("test_table", tableAddress);
  // EXPECT_EQ(stub.getCurrentBlockNumber(), 0);

  std::map<const BYTES, const BYTES> map;
  for (int i = 0; i < blocksize; i++) {
    map.emplace(BYTES("k"), BYTES(std::to_string(i)));
  }
  stub.put(map);
  // EXPECT_EQ(stub.getCurrentBlockNumber(), 0);

  stub.shutdown();

  // delete all the files and folder, which created from Test
  if (std::filesystem::exists(test_folder)) {
    std::filesystem::remove_all(test_folder);
  }
}
*/
/** @} */