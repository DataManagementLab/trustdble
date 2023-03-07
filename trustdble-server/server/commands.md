# TRUSTDBLE SQL Commands
Here is a list of all additional SQL commands
### CREATE SHARED DATABASE
```
    Name: 'CREATE SHARED DATABASE'
    Description:
    Syntax:

        CREATE SHARED DATABASE <database_name> TYPE=<database_type> SHARDS=<number_shards>  encrypt=<encryption_status>

        <database_name> = name of database
        <database_type> = STUB|ETHEREUM|FABRIC
        <number_shards> = number of shards
        <encryption_status> = true|false

        CREATE SHARED DATABASE is used to create a sharable database with the help of blockchain technology. All interactions with tables in a shared database are recorded on the choosen blockchain. On creation of a new SHARED DATABASE, meta data about the database is stored in the table trustdble.shared_databases. Creating a new shared database requires to deploy a new blockchain network that is used to record meta data of the shared databases in a tamper-proof way. All parties with which the created database is shared will participate in this so called "meta-chain" blockchain network.

    Examples:
        CREATE SHARED DATABASE my_db type=ETHEREUM shards=2 encrypt=true
```

### CREATE SHARED TABLE
```
    Name: 'CREATE SHARED TABLE'
    Description:
    Syntax:

        CREATE SHARED TABLE table_name (create_definition, ...)

        The create_definition is the same as in the CREATE TABLE command.

        CREATE SHARED TABLE is used to create a sharable table which stores data in a blockchain. The used blockchain is determined by the SHARED DATABASE in which the table was created. A sharable table enables other TRUSTDBLE users to load it and interact with it by reading and writing data. When a SHARED TABLE is created it also adds information on how it can be accessed in the meta_table of the same database. The shared table must be created in a shared database.

    Examples:
    CREATE SHARED TABLE my_table (id INT)
```

### DROP SHARED DATABASE
```
    Name: 'DROP SHARED DATABASE'
    Description:
    Syntax:

        DROP SHARED DATABASE database_name

        This commands removes the specified shared database and its corresponding tables from the local TRUSTDBLE instance. It does not delete the database and its content for other users. I.e., other users / TRUSTDBLE instances will might still have access to the database. The command will remove the corresponding entries from the trustdble.shared_databases meta-data table and stop as well as remove all blockchain nodes that are associated with the database. The SHARED database can be rejoined again by executing the LOAD SHARED DATABASE command with the corresponding invite-file.

    Examples:
    DROP SHARED DATABASE my_db
```

### DROP SHARED TABLE
```
    Name: 'DROP SHARED TABLE'
    Description:
    Syntax:

        DROP TABLE table_name

        This command removes the table table_name from the local TRUSTDBLE instance. The table is no longer available on the local server but is still in the shared database. This does not delete the contents of the shared table. I.e., other users of the table (TRUSTDBLE instances) will still have access to all the data. Hence, this operation is referred to as "local hide" of the table. The table can be reloaded by using the load shared table command.

    Examples:
    DROP TABLE mytable
```

### LOAD SHARED DATABASE
```
    Name: 'LOAD SHARED DATABASE'
    Description:
    Syntax:

        LOAD SHARED DATABASE  <database_name>
        This command joins a shared database based on the information provided by the trustdble.invites table.

    Examples:
    LOAD SHARED DATABASE my_db
```

### LOAD SHARED TABLE
```
    Name: 'LOAD SHARED TABLE'
    Description:
    Syntax:

        LOAD SHARED TABLE table_name

        This command is used to load a specific SHARED TABLE that is part of the active database. After loading the table the user can interact with the table by reading and writing data. Thereby, all modification of the table will be recorded in the underlying blockchain of the database.

    Examples:
    LOAD SHARED TABLE my_table
```

### LOAD SHARED TABLES
```
    Name: 'LOAD SHARED TABLES'
    Description:
    Syntax:

        LOAD SHARED TABLES

        This command retrieves all tables of the active SHARED DATABASE from its meta-chain and creates them on the local TRUSTDBLE instance. After executing this command the user will have access to all SHARED TABLES that were created in the database.

    Examples:
    LOAD SHARED TABLES
```
### CREATE DATABASE INVITE
```
    Name: 'CREATE DATABASE INVITE'
    Description:
    Syntax:

        CREATE DATABASE INVITE <database_name> public_key=<public key>

        This command works as below:
        1. extracts data related to the <database_name> from trustdble.shared_databases.
        2. generates symmetric key/iv which is used to encrypt data.
        3. encrypts data using generated symmetric key/iv.
        4. encrypts symmetric key/iv using given <public key>.
        5. inserts encrypted symmetric key, encrypted symmetrci iv and encrypted data into trustdble.encrypted_invite table.

    Examples:
    CREATE DATABASE INVITE my_db1 public_key=MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2GwqD7fxputPTnJhcddpySuxnqWWMZC+kKs+E1MwvDLWIgXdTeL6qVZKWkOcngcwaVROU6FB2Zv6IdECqKI8RG6LIXEiGAiV42Vh6TaBjZsjAuJhuJYuvj5P1DV7jn0k1YYJa7befzA0e4FSvrDBXMSLTgbvIIrsLVJVtjJrI/lGYhA3yL2CYUx5/rn3BiuP9oMRvLtiUFITNuHRnjyE5aPyAFCQ/3qG+D9wbv6SEnS/wTvZ8zbkcRZgSMI0W+dsSb/NLU1j/iAze0AzVs/rUDvCLH1/rHgiMYT90oiXnYwyAiMD2PHG9DmQRuxE2lYMi96Avhq770oIULsv4AFbLnmETLVsYnkwFHp1PFKEnsuFE5/9Fld/9JuLIW+Dy89yW1zcn7sWIh3cCtukzrhGGLW6B3Wija9qnqRUs8z+tNGg/NM5VxvPJ8nN2F1YCbpydZ6waalyMqqUg9o1TbEy/aAcTs5LMTSmYret8IDLbZjwxfuHV3owAAB5Rc9uxLnR/yjXs4x22+MEKjOveZFGakF7tgOxcfWd13O0H5ZM0rtTy9V2Jh6o4SaBML6JK24i9sS/XePEiSWEe1PdSHVKfAfi1YJL1yWsDFyDw2Qq7RVQnZkOKvf9UiWIQl3VTTTGJ04UX3VKUZ8soTbJ07Af7mAQYiefNSmiAIXhkV1G5iMCAwEAAQ==;
```

### ADD DATABASE INVITE
```
    Name: 'ADD DATABASE INVITE'
    Description:
    Syntax:

        ADD DATABASE INVITE encrypted_invite=<encrypted invite>

        This command is used to decrypt the content of encrypted string using private key and works as below:
        1. decrypts symmetric key and iv using private key.
        2. decrypts invite using decrypted symmetric key/iv.
        3. inserts decrypted invite into trustdble.invites table.

    Examples:
    ADD DATABASE INVITE encrypted_invite={"key":"453585...","iv":"cb3b63...","invite":"9a1491..."};
```

### REPARTITION SHARED DATABASE
```
    Name: 'REPARTITION SHARED DATABASE'
    Description:
    Syntax:

        REPARTITION SHARED DATABASE <database_name> SHARDS=<number_shards>

        <database_name> = name of database
        <number_shards> = number of shards

        REPARTITION SHARED DATABASEE is used to repartition a sharable database to the different number of shards (data chains).
        This command works as below:
        1. copy all data tables data into local tables in trustdble database.
        2. delete all rows in data tables.
        3. add / remove data chains.
        4. update system tables.
        5. drop + load shared data tables to update table_addresses.
        6. insert data from local tables (with data copy) to data chains.
        7. delete local tables (with data copy).

    Examples:
        REPARTITION SHARED DATABASE my_db shards=5
```
### CREATE TABLE INVITE
```
    Name: 'CREATE TABLE INVITE'
    Description:
    Syntax:

        CREATE TABLE INVITE <table_name> public_key=<public key>

        This command works as below:
        1. extracts data related to the <table_name> from key_store.
        2. encrypts data using given <public key>.
        3. inserts encrypted invite into trustdble.encrypted_table_invite table.

    Examples:
    CREATE TABLE INVITE my_table public_key=MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA2GwqD7fxputPTnJhcddpySuxnqWWMZC+kKs+E1MwvDLWIgXdTeL6qVZKWkOcngcwaVROU6FB2Zv6IdECqKI8RG6LIXEiGAiV42Vh6TaBjZsjAuJhuJYuvj5P1DV7jn0k1YYJa7befzA0e4FSvrDBXMSLTgbvIIrsLVJVtjJrI/lGYhA3yL2CYUx5/rn3BiuP9oMRvLtiUFITNuHRnjyE5aPyAFCQ/3qG+D9wbv6SEnS/wTvZ8zbkcRZgSMI0W+dsSb/NLU1j/iAze0AzVs/rUDvCLH1/rHgiMYT90oiXnYwyAiMD2PHG9DmQRuxE2lYMi96Avhq770oIULsv4AFbLnmETLVsYnkwFHp1PFKEnsuFE5/9Fld/9JuLIW+Dy89yW1zcn7sWIh3cCtukzrhGGLW6B3Wija9qnqRUs8z+tNGg/NM5VxvPJ8nN2F1YCbpydZ6waalyMqqUg9o1TbEy/aAcTs5LMTSmYret8IDLbZjwxfuHV3owAAB5Rc9uxLnR/yjXs4x22+MEKjOveZFGakF7tgOxcfWd13O0H5ZM0rtTy9V2Jh6o4SaBML6JK24i9sS/XePEiSWEe1PdSHVKfAfi1YJL1yWsDFyDw2Qq7RVQnZkOKvf9UiWIQl3VTTTGJ04UX3VKUZ8soTbJ07Af7mAQYiefNSmiAIXhkV1G5iMCAwEAAQ==;
```
### ADD TABLE INVITE
```
    Name: 'ADD TABLE INVITE'
    Description:
    Syntax:

        ADD TABLE INVITE encrypted_invite=<encrypted invite>

        This command is used to decrypt the content of encrypted string using private key and works as below:
        1. decrypts encrypted invite string using private key.
        2. inserts/updates decrypted invite into key_store table.

    Examples:
    ADD TABLE INVITE encrypted_invite={"invite":"9a1491..."};
```
### CREATE DATA CONTRACT
```
    Name: 'CREATE DATA CONTRACT'
    Description:
    Syntax:

        CREATE DATA CONTRACT name=<name> data_owner=<data owner> data_user=<data user> logic_path=<path to logic> parameters=<paramteres>

        This command is used to create data contract AND inserts it into trustdble.data_contracts table.
    Examples:
    CREATE DATA CONTRACT name=DC1 data_owner=DO1 data_user=DU1 logic_path=../logic.txt parameters=a;
```
### ADD DATA CONTRACT
```
    Name: 'ADD DATA CONTRACT'
    Description:
    Syntax:

        ADD DATA CONTRACT encrypted_data_contract=<encrypted data contract>

        This command is used to decrypt the content of encrypted data contract using private key and works as below:
        1. decrypts symmetric key and iv using private key.
        2. decrypts data contract using decrypted symmetric key/iv.
        3. inserts decrypted data contract into trustdble.remote_data_contracts table.

    Examples:
    ADD DATA CONTRACT encrypted_data_contract={"key":"453585...","iv":"cb3b63...","data_contract":"9a1491..."};
```

### ENCRYPT DATA CONTRACT
```
    Name: 'ENCRYPT DATA CONTRACT'
    Description:
    Syntax:

        ENCRYPT DATA CONTRACT name=<data contract name> public_key=<public key>

        This command is used to fetch the specified data contract from table trustdble.data_contracts and encrypt data contract using data users public key and works as below:
        1. generates data contract in json format using the informaton that is provided in the command
        2. generates symmetric key/iv which is used to encrypt data.
        3. encrypts data using generated symmetric key/iv.
        4. encrypts symmetric key/iv using given <public key>.
        5. inserts encrypted symmetric key, encrypted symmetrci iv and encrypted data into trustdble.encrypted_data_contracts table.

    Examples:
    ENCRYPT DATA CONTRACT name=DC1 public_key=MIICIjANBgkqhkiG9w0BA....;
```