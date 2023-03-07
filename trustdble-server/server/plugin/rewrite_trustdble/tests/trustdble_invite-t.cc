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

#include "trustdble_invite.h"

// Write and read invite
TEST(TrustDBleInviteTest, write_read) {

  // Values
  std::string name="mydb";
  std::string meta_address="trustdble-server/server/meta_data/metadata.txt";
  std::string meta_chain_config="meta";
  std::string bc_type="STUB";
  std::string encryption_key="1234567890";

  // Invite with values
  SHARED_DATABASE invite;
  invite.name=name;
  invite.meta_address=meta_address;
  invite.meta_chain_config=meta_chain_config;
  invite.bc_type=bc_type;

  // Invite to store result
  trustdble::SHARED_DATABASE result_invite;

  write_invite("", invite);

  read_invite_file("mydb.json", result_invite);
  EXPECT_EQ(result_invite.name, invite.name);
  EXPECT_EQ(result_invite.meta_address, invite.meta_address);
  EXPECT_EQ(result_invite.meta_chain_config, invite.meta_chain_config);
  EXPECT_EQ(result_invite.bc_type, invite.bc_type);
  EXPECT_EQ(result_invite.encryption_key, invite.encryption_key);

  remove( "mydb.json" );
}
