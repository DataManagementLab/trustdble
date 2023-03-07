/*! \addtogroup group52
 *  @{
 */
#include "adapter_interface_test.h"

#include <chrono>

#define TIME(fn)                                                           \
  ({                                                                       \
    std::chrono::high_resolution_clock::time_point start =                 \
        std::chrono::high_resolution_clock::now();                         \
    fn;                                                                    \
    std::chrono::high_resolution_clock::time_point end =                   \
        std::chrono::high_resolution_clock::now();                         \
    auto duration =                                                        \
        std::chrono::duration_cast<std::chrono::microseconds>(end - start) \
            .count();                                                      \
    printf("%16.d µs\n", duration);                                        \
  });

/**
 * @file
 * @brief This file contains generic test cases, that all adapter
 * implementations should pass.
 *
 */

/**********************************************
 *  Tests for the get(const BYTES &key, BYTES &result) method
 ***********************************************/

/**
 * @brief Test that reading an existing value works
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, GetEntry /*unused*/) {
  EXPECT_EQ(adapter0_->get(keys_[0], result_), 0);
  EXPECT_EQ(result_, values_[0]);
}

/**
 * @brief Test that removing a previously deleted key results in the expected
 * return code
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, GetAfterRemove /*unused*/) {
  EXPECT_EQ(adapter0_->remove(keys_[0]), 0);
  EXPECT_EQ(adapter0_->get(keys_[0], result_), 1);
  std::cout << "\nGetAfterRemove: \" GET, Failed to open File \" expect!! \n"
            << std::endl;
}

/**
 * @brief Test that reading a non-existing key results in the expected return
 * code
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, GetMissingEntry /*unused*/) {
  EXPECT_EQ(adapter0_->get(keys_[3], result_), 1);
  std::cout << "\nGetMissingEntry: \" GET, No value \" expect!! \n"
            << std::endl;
}

/**
 * @brief Test that reading values from a dropped table results in the
 expected
 * return code
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, GetAfterDrop /*unused*/) {
  EXPECT_EQ(adapter0_->drop_table(), 0);
  EXPECT_EQ(adapter0_->get(keys_[0], result_), 1);
  std::cout << "\nGetAfterDrop: \" GET, Failed to open File \" expect!! \n"
            << std::endl;
}

/**********************************************
 *  Tests for the put(std::map<const BYTES, const BYTES>) method
 ***********************************************/

/**
 * @brief Test that inserting a batch works
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, PutGetEntries /*unused*/) {
  EXPECT_EQ(adapter0_->put(batch_), 0);
  EXPECT_EQ(adapter0_->get(batch_keys_[0], result_), 0);
  EXPECT_EQ(result_, batch_values_[0]);
  EXPECT_EQ(adapter0_->get(batch_keys_[1], result_), 0);
  EXPECT_EQ(result_, batch_values_[1]);
  EXPECT_EQ(adapter0_->get(batch_keys_[2], result_), 0);
  EXPECT_EQ(result_, batch_values_[2]);
  EXPECT_EQ(adapter0_->get(batch_keys_[3], result_), 0);
  EXPECT_EQ(result_, batch_values_[3]);
  EXPECT_EQ(adapter0_->get(batch_keys_[4], result_), 0);
  EXPECT_EQ(result_, batch_values_[4]);
}

/**********************************************
 *  Tests for the remove(const BYTES &key) method
 ***********************************************/

/**
 * @brief Test that removing a missing entry gives the correct return code
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, RemoveMissingEntry /*unused*/) {
  EXPECT_EQ(adapter0_->remove(keys_[3]), 1);
  std::cout << "\nTableScanAfterDrop: \" REMOVE, failed due to key not found"
               "\" expect!! \n"
            << std::endl;
}

/**
 * @brief Test that removing from a non-existing (dropped) table results in
 the
 * expected return code
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, RemoveAfterDrop /*unused*/) {
  EXPECT_EQ(adapter0_->drop_table(), 0);
  EXPECT_EQ(adapter0_->remove(keys_[0]), 1);
  std::cout
      << "\nRemoveAfterDrop: \" REMOVE, Failed to open File \" expect!! \n"
      << std::endl;
}

/**********************************************
 *  Tests for the table scan
 * get_all(std::map<const BYTES, BYTES> &results) method
 ***********************************************/

/**
 * @brief Test that all entries are returned for a given table
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, TableScan /*unused*/) {
  EXPECT_EQ(adapter0_->get_all(result_map_), 0);
  ASSERT_EQ(result_map_.size(), 3);
  std::map<const BYTES, BYTES>::iterator it;
  int i = 0;
  for (it = result_map_.begin(); it != result_map_.end(); it++) {
    EXPECT_EQ(it->first, keys_[i]);
    EXPECT_EQ(it->second, values_[i++]);
  }
}

/**
 * @brief Test that no entries are returned if table has been dropped before
 *
 */
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
TEST_P(AdapterInterfaceTest /*unused*/, TableScanAfterDrop /*unused*/) {
  ASSERT_EQ(adapter0_->drop_table(), 0);
  EXPECT_EQ(adapter0_->get_all(result_map_), 1);
  std::cout
      << "\nTableScanAfterDrop: \" GET_ALL, Failed to open File \" expect!! \n"
      << std::endl;
}
/** @} */