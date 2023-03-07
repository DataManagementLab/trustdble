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

#include <gtest/gtest.h>

#include "rewrite_trustdble.h"

struct Query_pair {
  const std::string query;
  const std::string rewritten_query;
};

Query_pair createSharedDatabase{"create shared database my_database",
                                "create database my_database"};
Query_pair createSharedTable{"create shared table my_table(i int)",
                             "create table if not exists my_table(i int)"};
Query_pair loadSharedDatabase{"load shared database my_database",
                              "create database my_database"};
Query_pair loadSharedTable{"load shared table my_table", "my_table"};

Query_pair loadSharedTables{"load shared tables", ""};

// Single Test-Cases - TEST(test_suite_name, test_name)

TEST(RewriteTrustDBleTest, create_shared_database) {
  std::string rewrite_result = createSharedDatabase.query;
  rewrite_query(rewrite_result, CREATE_DATABASE);
  EXPECT_EQ(rewrite_result, createSharedDatabase.rewritten_query);
}

TEST(RewriteTrustDBleTest, create_shared_table) {
  std::string rewrite_result = createSharedTable.query;
  rewrite_query(rewrite_result, CREATE_TABLE);
  EXPECT_EQ(rewrite_result, createSharedTable.rewritten_query);
}

TEST(RewriteTrustDBleTest, load_shared_database) {
  std::string rewrite_result = loadSharedDatabase.query;
  rewrite_query(rewrite_result, LOAD_DATABASE);
  EXPECT_EQ(rewrite_result, loadSharedDatabase.rewritten_query);
}

TEST(RewriteTrustDBleTest, load_shared_table) {
  std::string rewrite_result = loadSharedTable.query;
  rewrite_query(rewrite_result, LOAD_TABLE);
  EXPECT_EQ(rewrite_result, loadSharedTable.rewritten_query);
}

TEST(RewriteTrustDBleTest, load_shared_tables) {
  std::string rewrite_result = loadSharedTables.query;
  rewrite_query(rewrite_result, LOAD_TABLES);
  EXPECT_EQ(rewrite_result, loadSharedTables.rewritten_query);
}
