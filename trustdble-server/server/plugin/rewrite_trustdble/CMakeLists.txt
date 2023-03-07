# Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

add_subdirectory(../../../blockchain-manager ./manager)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(../../storage/blockchain/include)

set(HEADER_LIST
  "${CMAKE_CURRENT_SOURCE_DIR}/include/rewrite_trustdble.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/trustdble_invite.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/trustdble_command.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(rewriteTrustDBle src/rewrite_trustdble.cc)
add_library(trustDBleInvite src/trustdble_invite.cc)

target_link_libraries(rewriteTrustDBle PUBLIC TrustDBle::trustdbleUtils)

MYSQL_ADD_PLUGIN(rewrite_trustdble
  src/rewrite_trustdble_plugin.cc
  src/trustdble_command.cc
  ../../storage/blockchain/src/table_service.cc
  MODULE_ONLY
  MODULE_OUTPUT_NAME "rewrite_trustdble"
  LINK_LIBRARIES TrustDBle::adapterFactory TrustDBle::bcManagerFactory rewriteTrustDBle trustDBleInvite trustdbleCryptoService
  )

ADD_DEFINITIONS(-DMYSQL_SERVER)
ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="Rewrite_TrustDBle")

# Copy script to build folder
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/setup_trustdble.sql
  ${CMAKE_CURRENT_BINARY_DIR}/scripts/setup_trustdble.sql
  )

# Testing
OPTION(WITH_TRUSTDBLE_TESTS
  "Include the TRUSTDBLE test programs" OFF)
IF(WITH_TRUSTDBLE_TESTS)
  add_subdirectory(tests)
ELSE()
ENDIF()