/* Copyright (c) 2004, 2017, Oracle and/or its affiliates. All rights reserved.

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

/** @file ha_blockchain.h

    @brief
  The ha_blockchain engine is a stubbed storage engine for blockchain purposes
  only; it does nothing at this point. Its purpose is to provide a source code
  illustration of how to begin writing new storage engines; see also
  /storage/blockchain/ha_blockchain.cc.

    @note
  Please read ha_blockchain.cc before reading this file.
  Reminder: The blockchain storage engine implements all methods that are
  *required* to be implemented. For a full list of all methods that you can
  implement, see handler.h.BC_T

   @see
  /sql/handler.h and /storage/blockchain/ha_blockchain.cc
*/

#include <mutex>
#include <sys/types.h>

#include "my_base.h" /* ha_rows */
#include "my_compiler.h"
#include "my_inttypes.h"
#include "sql/handler.h" /* handler */
#include "thr_lock.h"    /* THR_LOCK, THR_LOCK_DATA */

#include "adapter_factory/adapter_factory.h"
#include "table_service.h"
#include "transaction.h"

using namespace trustdble;
static const size_t MAX_BC_KEY_SIZE = 32;

/**

 * @brief Struct that is representing a pair of encyption_key and iv.

 * It is used to create a map of table/database , pair of encryption_key and iv

 */
struct ENCRYPTION_CONFIG {
  unsigned char *encryption_key;
  unsigned char *encryption_iv;
};
/** @brief
  Class definition for the handler for blockchain storage engine
*/
class ha_blockchain : public handler {
  my_off_t current_position; // current position during table scan
  std::vector<std::tuple<BYTES, BYTES>>
      all_items; // buffer for table scan result

public:
  ha_blockchain(handlerton *hton, TABLE_SHARE *table_arg);
  ~ha_blockchain() override;

  /** @brief
    The name that will be used for display purposes.
   */
  const char *table_type() const override { return "BLOCKCHAIN"; }

  /**
    Replace key algorithm with one supported by SE, return the default key
    algorithm for SE if explicit key algorithm was not provided.

    @sa handler::adjust_index_algorithm().
  */
  enum ha_key_alg get_default_index_algorithm() const override {
    return HA_KEY_ALG_HASH;
  }
  bool is_index_algorithm_supported(enum ha_key_alg key_alg) const override {
    return key_alg == HA_KEY_ALG_HASH;
  }

  /** @brief
    This is a list of flags that indicate what functionality the storage engine
    implements. The current table flags are documented in handler.h
  */
  ulonglong table_flags() const override {
    /*
      We are saying that this engine is just statement capable to have
      an engine that can only handle statement-based logging. This is
      used in testing.
    */
    return HA_BINLOG_STMT_CAPABLE;
  }

  /** @brief
    This is a bitmap of flags that indicates how the storage engine
    implements indexes. The current index flags are documented in
    handler.h. If you do not implement indexes, just return zero here.

      @details
    part is the key part to check. First key part is 0.
    If all_parts is set, MySQL wants to know the flags for the combined
    index, up to and including 'part'.
  */
  ulong index_flags(uint inx MY_ATTRIBUTE((unused)),
                    uint part MY_ATTRIBUTE((unused)),
                    bool all_parts MY_ATTRIBUTE((unused))) const override {
    return 0;
  }

  /** @brief
    unireg.cc will call max_supported_record_length(), max_supported_keys(),
    max_supported_key_parts(), uint max_supported_key_length()
    to make sure that the storage engine can handle the data it is about to
    send. Return *real* limits of your storage engine here; MySQL will do
    min(your_limits, MySQL_limits) automatically.
   */
  uint max_supported_record_length() const override { return 50000; }

  /** @brief
    unireg.cc will call this to make sure that the storage engine can handle
    the data it is about to send. Return *real* limits of your storage engine
    here; MySQL will do min(your_limits, MySQL_limits) automatically.

      @details
    There is no need to implement ..._key_... methods if your engine doesn't
    support indexes.
   */
  uint max_supported_keys() const override { return 64; }

  /** @brief
    unireg.cc will call this to make sure that the storage engine can handle
    the data it is about to send. Return *real* limits of your storage engine
    here; MySQL will do min(your_limits, MySQL_limits) automatically.

      @details
    There is no need to implement ..._key_... methods if your engine doesn't
    support indexes.
   */
  /// uint max_supported_key_parts() const override { return 1; }
  uint
  max_supported_key_part_length(HA_CREATE_INFO *create_info) const override {
    (void)create_info; // this variable is not used, since we set a fixed limit
                       // independent from the meta information about the table
    return 3072;       // same key size as in innodb
  }

  /** @brief
    unireg.cc will call this to make sure that the storage engine can handle
    the data it is about to send. Return *real* limits of your storage engine
    here; MySQL will do min(your_limits, MySQL_limits) automatically.

      @details
    There is no need to implement ..._key_... methods if your engine doesn't
    support indexes.
   */
  uint max_supported_key_length() const override { return 3500; }

  /** @brief
    Called in test_quick_select to determine if indexes should be used.
  */
  double scan_time() override {
    return (double)(stats.records + stats.deleted) / 20.0 + 10;
  }

  /** @brief
    This method will never be called if you do not implement indexes.
  */
  double read_time(uint, uint, ha_rows rows) override {
    return (double)rows / 20.0 + 1;
  }

  /**********************************************************************
    Everything below are methods that we implement in ha_blockchain.cc.

    Most of these methods are not obligatory, skip them and
    MySQL will treat them as not implemented
  **********************************************************************/

  /** @brief
    We implement this in ha_blockchain.cc; it's a required method.
  */
  int open(const char *name, int mode, uint test_if_locked,
           const dd::Table *table_def) override; // required

  /** @brief
    We implement this in ha_blockchain.cc; it's a required method.
  */
  int close(void) override; // required

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int write_row(uchar *buf) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int update_row(const uchar *old_data, uchar *new_data) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int delete_row(const uchar *buf) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int index_read_map(uchar *buf, const uchar *key, key_part_map keypart_map,
                     enum ha_rkey_function find_flag) override;

  int index_read(uchar *buf, const uchar *key, uint key_len,
                 enum ha_rkey_function find_flag) override;

  /**
   * @brief returns the hex encoded key of the table for a certain row
   *
   * @param[in] buf row of the table
   * @return 0 key
   */
  auto get_primary_key(const uchar *buf) -> BYTES;

public:
  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int index_next(uchar *buf) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int index_prev(uchar *buf) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int index_first(uchar *buf) override;

  /** @brief
    We implement this in ha_blockchain.cc. It's not an obligatory method;
    skip it and and MySQL will treat it as not implemented.
  */
  int index_last(uchar *buf) override;

  /** MySQL calls this function at the start of each SQL statement inside LOCK
  TABLES. Inside LOCK TABLES the "::external_lock" method does not work to mark
  SQL statement borders. Note also a special case: if a temporary table is
  created inside LOCK TABLES, MySQL has not called external_lock() at all on
  that table.
  MySQL-5.0 also calls this before each statement in an execution of a stored
  procedure. To make the execution more deterministic for binlogging, MySQL-5.0
  locks all tables involved in a stored procedure with full explicit table locks
  (thd_in_lock_tables(thd) holds in store_lock()) before executing the
  procedure.
  @param[in]	thd		handle to the user thread
  @param[in]	lock_type	lock type
  @return 0 or error code */
  int start_stmt(THD *thd, thr_lock_type lock_type) override;

  /** @brief
    Unlike index_init(), rnd_init() can be called two consecutive times
    without rnd_end() in between (it only makes sense if scan=1). In this
    case, the second call should prepare for the new table scan (e.g if
    rnd_init() allocates the cursor, the second call should position the
    cursor to the start of the table; no need to deallocate and allocate
    it again. This is a required method.
  */
  int rnd_init(bool scan) override; // required
  int rnd_end() override;
  int rnd_next(uchar *buf) override;            ///< required
  int rnd_pos(uchar *buf, uchar *pos) override; ///< required
  void position(const uchar *record) override;  ///< required

  int info(uint) override; ///< required
  int extra(enum ha_extra_function operation) override;
  int external_lock(THD *thd, int lock_type) override; ///< required
  int delete_all_rows(void) override;
  ha_rows records_in_range(uint inx, key_range *min_key,
                           key_range *max_key) override;
  int delete_table(const char *from, const dd::Table *table_def) override;
  int rename_table(const char *from, const char *to,
                   const dd::Table *from_table_def,
                   dd::Table *to_table_def) override;
  int create(const char *name, TABLE *form, HA_CREATE_INFO *create_info,
             dd::Table *table_def) override; ///< required

  THR_LOCK_DATA **
  store_lock(THD *thd, THR_LOCK_DATA **to,
             enum thr_lock_type lock_type) override; ///< required

  /*******************
   * Helper Methods
   *******************/

  int find_current_row(uchar *buf);
  int find_row(my_off_t index, uchar *buf);

  // Storage engine methods
  static handler *bc_create_handler(handlerton *hton, TABLE_SHARE *table,
                                    bool partitioned, MEM_ROOT *mem_root);
  static int bc_close_connection(handlerton *hton, THD *thd);
  static int bc_commit(handlerton *hton, THD *thd, bool commit_trx);
  static int bc_rollback(handlerton *hton, THD *thd, bool all);
  /**
   * @brief Get data_chain_id for a specific row of the shared_tables
   *
   * @param[in] key object representing row of shared_tables
   * @param[in] num_shards number of shards
   * @return data chain id
  */
  static int get_data_chain_for_key(std::string key, const int num_shards);


  std::string dataconfig;
  std::string metaconfig;
  std::string meta_table_address;
  std::string bctype;
};
