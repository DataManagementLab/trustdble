# Rewrite plugin for TrustDBle
This plugin rewrites SQL staements that contain the key word *shared* into normal SQL staements. While processing shared statements addititonal tasks like setting up a blockchain and deploying a contract for a meta-data table are done.

## Shared SQL Statements
* Creating a new shared database with the blockchain type of the metachain.
```
create shared database demo_db type=ETHEREUM;
```

* Loading a shared database from invite file
```
load shared database /mypath_to_invite_dir/demo_db.json
```

* Creating a shared table
```
create shared table my_table (id INT, firstname VARCHAR(20), lastname VARCHAR(20));
```

* Loading a shared table by name of a shared database
(requires prior selection of the shared database with the use command)
```
load shared table my_table
```

* Loading all shared tables in a shared database
(requires prior selection of the shared database with the use command)
```
load shared tables
```

## Data Manipulation
After creating/loading a shared database and shared tables you can use standard SQL data manipulation statements to work with the table data. For example:

```
insert into mytable values (1, "Max", "Mueller");
select * from mytable;
update ...
...
```

