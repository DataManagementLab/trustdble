#include "bc_adapter_test.h"

// Test Get-Method
TEST_P(BC_Adapter_Test, GetEntry) {
  EXPECT_EQ(adapter->get(keys[0], result), 0);
  EXPECT_EQ(result, values[0]);
}

TEST_P(BC_Adapter_Test, GetAfterRemove) {
  EXPECT_EQ(adapter->remove(keys[0]), 0);
  EXPECT_EQ(adapter->get(keys[0], result), 1);
}

TEST_P(BC_Adapter_Test, GetMissingEntry) {
  EXPECT_EQ(adapter->get(keys[3], result), 1);
}

TEST_P(BC_Adapter_Test, GetAfterDrop) {
  EXPECT_EQ(adapter->drop_table(), 0);
  EXPECT_EQ(adapter->get(keys[0], result), 1);
}

//Test Put-Method
TEST_P(BC_Adapter_Test, PutGetEntry) {
  EXPECT_EQ(adapter->put(data_batch), 0);
  EXPECT_EQ(adapter->get(keys[3], result), 0);
  EXPECT_EQ(result, values[3]);
}

// Test Remove-Method
TEST_P(BC_Adapter_Test, RemoveMissingEntry) {
  EXPECT_EQ(adapter->remove(keys[3]), 137);
}

TEST_P(BC_Adapter_Test, RemoveAfterDrop) {
  EXPECT_EQ(adapter->drop_table(), 0);
  EXPECT_EQ(adapter->remove(keys[0]), 1);
}

// Test TableScan
TEST_P(BC_Adapter_Test, TableScan) {
  EXPECT_EQ(adapter->get_all(result_vec), 0);

  ASSERT_EQ(result_vec.size(), 3);

  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(std::get<0>(result_vec.at(i)), keys[i]);
    EXPECT_EQ(std::get<1>(result_vec.at(i)), values[i]);
  }
}

TEST_P(BC_Adapter_Test, TableScanAfterDrop) {
  ASSERT_EQ(adapter->drop_table(), 0);
  EXPECT_EQ(adapter->get_all(result_vec), 1);
}