/* ------------------------------------------------------------------------
 * Create the required DB object that are required by the plugin to function
 * ------------------------------------------------------------------------
 */
CREATE DATABASE IF NOT EXISTS trustdble;

CREATE TABLE IF NOT EXISTS trustdble.shared_databases (
  database_id INT NOT NULL AUTO_INCREMENT,
  database_name VARCHAR(100) NOT NULL,
  blockchain_type ENUM('STUB', 'ETHEREUM', 'FABRIC') NOT NULL DEFAULT 'STUB',
  meta_table_address VARCHAR(200) NOT NULL,
  meta_chain_config JSON NOT NULL,
  encryption_key VARCHAR(64),
  iv VARCHAR(32),
  database_state ENUM('PROCESSING', 'CREATED', 'FAILED') NOT NULL DEFAULT 'PROCESSING',
  PRIMARY KEY (database_id)
);

CREATE TABLE IF NOT EXISTS trustdble.docker_containers (
  container_id INT NOT NULL AUTO_INCREMENT,
  container_name VARCHAR(100) NOT NULL,
  database_name VARCHAR(100) NOT NULL,
  blockchain_type ENUM('STUB', 'ETHEREUM', 'FABRIC') NOT NULL DEFAULT 'STUB',
  rpc_port INT,
  peer_port INT,
  join_ip VARCHAR(60),
  PRIMARY KEY (container_id)
);

CREATE function call_data_contract RETURNS INT SONAME "UDF.so";
/* ------------------------------------------------------------------------
 * Add HELP documentation about the commands that are introduced by this plugin
 * ------------------------------------------------------------------------
 */
START TRANSACTION;

/* Determine the next id that we should use, this differs by all 3 tables */
set @count_help_category_id = (select count(help_category_id) from mysql.help_category);
set @count_help_keyword_id = (select count(help_keyword_id) from mysql.help_keyword);
set @count_help_topic_id = (select count(help_topic_id) from mysql.help_topic);


INSERT INTO mysql.help_category (help_category_id,name,parent_category_id,url) VALUES (@count_help_category_id, 'Trustdble',0,'');

INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES (@count_help_keyword_id, 'CREATE_SHARED_TABLE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 1), 'CREATE_SHARED_DATABASE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 2),'LOAD_SHARED_DATABASE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 3),'DROP_SHARED_TABLE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 4),'DROP_SHARED_DATABASE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 5),'LOAD_SHARED_TABLES');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 6),'LOAD_SHARED_TABLE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 7),'CREATE DATABASE INVITE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 8),'ADD DATABASE INVITE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 9),'REPARTITION SHARED DATABASE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 10),'CREATE TABLE INVITE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 11),'ADD TABLE INVITE');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 12),'CREATE DATA CONTRACT');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 13),'ADD DATA CONTRACT');
INSERT INTO mysql.help_keyword (help_keyword_id,name) VALUES ((@count_help_keyword_id + 14),'ENCRYPT DATA CONTRACT');


INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES (@count_help_topic_id, @count_help_keyword_id);
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 1), (@count_help_keyword_id + 1));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 2), (@count_help_keyword_id + 2));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 3), (@count_help_keyword_id + 3));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 4), (@count_help_keyword_id + 4));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 5), (@count_help_keyword_id + 5));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 6), (@count_help_keyword_id + 6));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 7), (@count_help_keyword_id + 7));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 8), (@count_help_keyword_id + 8));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 9), (@count_help_keyword_id + 9));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 10), (@count_help_keyword_id + 10));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 11), (@count_help_keyword_id + 11));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 12), (@count_help_keyword_id + 12));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 13), (@count_help_keyword_id + 13));
INSERT INTO mysql.help_relation (help_topic_id,help_keyword_id) VALUES ((@count_help_topic_id + 14), (@count_help_keyword_id + 14));

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  @count_help_topic_id,
  @count_help_category_id,
  'CREATE SHARED TABLE',
  'Syntax:\n
  CREATE SHARED TABLE table_name (create_definition, ...)\n\n
  The create_definition is the same as in the CREATE TABLE command\n
  CREATE SHARED TABLE is used to create a sharable table which stores data in a blockchain.
  The used blockchain is determined by the SHARED DATABASE in which the table was created.
  A sharable table enables other TRUSTDBLE users to load it and interact with it by reading and writing data.
  When a SHARED TABLE is created it also adds information on how it can be accessed in the meta_table of the same database.
  The shared table must created in a shared database.\n\n',
  'CREATE SHARED TABLE my_table (id INT)\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 1),
  @count_help_category_id,
  'CREATE SHARED DATABASE',
  'Syntax:\n
  CREATE SHARED DATABASE <database_name> TYPE=<database_type> SHARDS=<number_shards> ENCRYPT=<encryption_status>\n\n
  <database_name> = name of database\n
  <database_type> = STUB|ETHEREUM|FABRIC\n
  <number_shards> = number of shards\n
  <encryption_status> = true|false\n

  CREATE SHARED DATABASE is used to create a sharable database with the help of blockchain technology.
  All interactions with tables in a shared database are recorded on the chosen blockchain.
  On creation of a new SHARED DATABASE, meta data about the database is stored in the table trustdble.shared_databases.
  Creating a new shared database requires to deploy a new blockchain network that is used to record meta data of the shared databases in a tamper-proof way.
  All parties with which the created database is shared will participate in this so called "meta-chain" blockchain network.\n\n',
  'SHARDS and ENCRYPT parameters are optional, if they are not specified a default value is allocated to them'
  'CREATE SHARED DATABASE my_db type=ETHEREUM shards=2 encrypt=false\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 2),
  @count_help_category_id,
  'LOAD SHARED DATABASE',
  'Syntax:\n
  LOAD SHARED DATABASE database_name\n\n
  This command joins a shared database based on the information provided by the trustdble.invites table.\n\n',
  'LOAD SHARED DATABASE my_db\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 3),
  @count_help_category_id,
  'DROP SHARED TABLE',
  'Syntax:\n
  DROP SHARED TABLE table_name\n\n
  This command removes the table table_name from the local TRUSTDBLE instance.
  The table is no longer available on the local server but is still in the shared database.
  This does not delete the contents of the shared table.
  I.e., other users of the table (TRUSTDBLE instances) will still have access to all the data.
  Hence, this operation is referred to as "local hide" of the table.
  The table can be reloaded by using the load shared table command.\n\n',
  'DROP SHARED TABLE mytable\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 4),
  @count_help_category_id,
  'DROP SHARED DATABASE',
  'Syntax:\n
  DROP SHARED DATABASE database_name\n\n
  This commands removes the specified shared database and its corresponding tables from the local TRUSTDBLE instance.
  It does not delete the database and its content for other users. I.e., other users / TRUSTDBLE instances will might still have access to the database.
  The command will remove the corresponding entries from the trustdble.shared_databases meta-data table and stop as well as remove all blockchain nodes that are associated with the database.
  The SHARED database can be rejoined again by executing the LOAD SHARED DATABASE command with the corresponding invite-file.\n\n',
  'DROP SHARED DATABASE my_db\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 5),
  @count_help_category_id,
  'LOAD SHARED TABLES',
  'Syntax:\n
  LOAD SHARED TABLES\n\n
  This command retrieves all tables of the active SHARED DATABASE from its meta-chain and creates them on the local TRUSTDBLE instance.
  After executing this command the user will have access to all SHARED TABLES that were created in the database.\n\n',
  'LOAD SHARED TABLES\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 6),
  @count_help_category_id,
  'LOAD SHARED TABLE',
  'Syntax:\n
  LOAD SHARED TABLE table_name\n\n
  This command is used to load a specific SHARED TABLE that is part of the active database.
  After loading the table the user can interact with the table by reading and writing data.
  Thereby, all modification of the table will be recorded in the underlying blockchain of the database.\n\n',
  'LOAD SHARED TABLE my_table\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 7),
  @count_help_category_id,
  'CREATE DATABASE INVITE',
  'Syntax:\n
  CREATE DATABASE INVITE database_name public_key=public key\n\n
  This command is used to encrypt invite information using public key and works as below:
        1. extracts data related to the <database_name> from trustdble.shared_databases.
        2. generates symmetric key/iv which is used to encrypt data.
        3. encrypts data using generated symmetric key/iv.
        4. encrypts symmetric key/iv using given <public key>.
        5. inserts encrypted symmetric key, encrypted symmetrci iv and encrypted data into trustdble.encrypted_invite table.\n\n',
  'CREATE DATABASE INVITE my_db1 public_key=MIICIjANBgkqhkiG9w0BA....;\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 8),
  @count_help_category_id,
  'ADD DATABASE INVITE',
  'Syntax:\n
  ADD DATABASE INVITE encrypted_invite=encrypted invite\n\n
  This command is used to decrypt the content of encrypted string using private key and works as below:
        1. decrypts symmetric key and iv using private key.
        2. decrypts invite using decrypted symmetric key/iv.
        3. inserts decrypted invite into trustdble.invites table.\n\n',
  'ADD DATABASE INVITE encrypted_invite={"key":"453585...","iv":"cb3b63...","invite":"9a1491..."};\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 9),
  @count_help_category_id,
  'REPARTITION SHARED DATABASE',
  'Syntax:\n
  REPARTITION SHARED DATABASE <database_name> SHARDS=<number_shards>\n\n
  <database_name> = name of database\n
  <number_shards> = number of shards\n

  REPARTITION SHARED DATABASEE is used to repartition a sharable database to the different number of shards (data chains).\n\n
  This command works as below:
    1. copy all data tables data into local tables in trustdble database.
    2. delete all rows in data tables.
    3. add / remove data chains.
    4. update system tables.
    5. drop + load shared data tables to update table_addresses.
    6. insert data from local tables (with data copy) to data chains.
    7. delete local tables (with data copy).\n\n',
  'REPARTITION SHARED DATABASE my_db shards=2;\n\n',
  '');

INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 10),
  @count_help_category_id,
  'CREATE TABLE INVITE',
  'Syntax:\n
  CREATE TABLE INVITE table_name public_key=public key \n\n
  This command works as below:
        1. extracts data related to the <table_name> from key_store.
        2. encrypts extracted data using given <public key>.
        3. inserts encrypted invite into trustdble.encrypted_invite_table table.\n\n',
  'CREATE TABLE INVITE my_table public_key=MIICIjANBgkqhkiG9w0BA....;\n\n',
  '');

  INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 11),
  @count_help_category_id,
  'ADD TABLE INVITE',
  'Syntax:\n
  ADD TABLE INVITE encrypted_invite=encrypted invite \n\n
  This command is used to decrypt the content of encrypted string using private key and works as below:
        1. decrypts encrypted invite string using private key.
        2. inserts/updates decrypted invite into key_store table.\n\n',
  'ADD TABLE INVITE encrypted_invite={"invite":"9a1491..."};\n\n',
  '');

  INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 12),
  @count_help_category_id,
  'CREATE DATA CONTRACT',
  'Syntax:\n
  CREATE DATA CONTRACT name=<data contract name> owner=<owner name> user=<user name> logic_path=<path to logic> parameters=<list of paramteres> \n\n
  This command is used to create data contract AND inserts it into trustdble.data_contracts table.\n\n',
  'CREATE DATA CONTRACT name=DC1 owner=DO1 user=DU1 logic_path=../logic.txt parameters=a \n\n',
  '');

  INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 13),
  @count_help_category_id,
  'ADD DATA CONTRACT',
  'Syntax:\n
  ADD DATA CONTRACT encrypted_data_contract=<encrypted data contract>\n\n
  This command is used to decrypt the content of encrypted data contract using private key and works as below:
        1. decrypts symmetric key and iv using private key.
        2. decrypts data contract using decrypted symmetric key/iv.
        3. inserts decrypted data contract into trustdble.remote_data_contracts table.\n\n',
  'ADD DATA CONTRACT encrypted_data_contract={"key":"453585...","iv":"cb3b63...","data_contract":"9a1491..."};\n\n',
  '');

 INSERT INTO mysql.help_topic (help_topic_id,help_category_id,name,description,example,url) VALUES (
  (@count_help_topic_id + 14),
  @count_help_category_id,
  'ENCRYPT DATA CONTRACT',
  'Syntax:\n
  ENCRYPT DATA CONTRACT name=<data contract name> public_key=<public key> \n\n
  This command is used to fetch the specified data contract from table trustdble.data_contracts and encrypt data contract using data users public key and works as below:
        1. generates data contract in json format using the informaton that is provided in the command
        2. generates symmetric key/iv which is used to encrypt data.
        3. encrypts data using generated symmetric key/iv.
        4. encrypts symmetric key/iv using given <public key>.
        5. inserts encrypted symmetric key, encrypted symmetrci iv and encrypted data into trustdble.encrypted_data_contracts table.\n\n',
  'ENCRYPT DATA CONTRACT name=DC1 public_key=MIICIjANBgkqhkiG9w0BA....;\n\n',
  '');

COMMIT;
