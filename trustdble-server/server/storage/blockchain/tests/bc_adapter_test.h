/* Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* See http://code.google.com/p/googletest/wiki/Primer */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"
#include <map>

#include <gtest/gtest.h>

#include "adapter_factory/adapter_factory.h"

// Test-Fixture for adapter base tests. Test are implemented in bc_adapter_test.cc

class BC_Adapter_Test
    : public testing::TestWithParam<std::tuple<BC_TYPE, std::string>> {
 protected:
  void SetUp() override {
    // Called before each test
    type = std::get<0>(GetParam());
    config_path = std::get<1>(GetParam());

    adapter = AdapterFactory::create_adapter(type);

    adapter->init(config_path);
    std::string tableAddress;
    adapter->create_table(tablename, tableAddress);

    adapter->put(data_batch);
  }

  void TearDown() override {
    // Clean Up after each test
    adapter->drop_table();
    adapter.reset();
  }

  std::unique_ptr<BcAdapter> adapter;
  BC_TYPE type;
  std::string config_path;

  // TestData
  std::string tablename = "test-table";
  std::map<std::string, std::string> data_batch = {{"key1", "value1"}, {"key2", "value2"}, {"key3", "value3"}};
  std::string keys[4] = {"key1", "key2", "key3", "key4"};
  std::string values[4] = {"value1", "value2", "value3", "value4"};

  // Read buffers
  std::string result;
  std::vector<std::tuple<std::string, std::string>> result_vec;
};
