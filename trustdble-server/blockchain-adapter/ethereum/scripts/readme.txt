0xA2885d3e0E2a93951531d0E91ECbC3D97234874E
0x95D56b3fc9C34c83a5d538a435EB9b6D422a7E86
    
Commands to run Blockchain storage engine:

	1.	start ehtereum node with bc controller
	
	2.	deploy contract 
		2.1	cd /home/simon/Project/dm/mysql-server/storage/blockchain/scripts
		2.2	node ./deploy_KV_contract.js
		
	3.	start mysql-server
		3.1	cd /home/simon/Project/dm/mysql-server
		3.2	build/bin/mysqld --binlog-format=STATEMENT --datadir=$(pwd)/test_data_dir --basedir=$(pwd)/build --plugin-load=ha_blockchain.so \
			    --blockchain-bc-type=0 \
			    --blockchain-bc-connection='http://localhost:8000' \
			    --blockchain-bc-eth-contracts=test:0xA2885d3e0E2a93951531d0E91ECbC3D97234874E \
			    --blockchain-bc-eth-from='0x542BB0f7035f4bf7dB677a54B73e5d0514B9bfBC'
			    
	4.	run mysql client
		4.1	cd /home/simon/Project/dm/mysql-server
		4.2	build/bin/mysql -u root -h localhost -P 3306
		
	5.	create table with bc storage engine
		5.1	CREATE TABLE foo (data VARCHAR(8)) ENGINE = BLOCKCHAIN;
		
	6.	run insert and select statements
