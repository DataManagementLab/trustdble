// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.6.8 <0.9.0;
pragma experimental ABIEncoderV2;

contract SimpleStorage {

      // tightly packed struct
    struct Value
    {
        uint blocknumber; // indicates when value was written
        string value;
    }

    mapping(bytes32 => Value) private data;        // data store
    bytes32[] internal keyList;                    // list of keys

    function put(bytes32 key, string memory value) public {

        Value memory v = Value(block.number,value);

        if(data[key].blocknumber == 0) {
            keyList.push(key);
        }

        // persist data in blockchain
        data[key] = v;
    }

    function get(bytes32 key) public view returns (string memory value) {

        Value memory v = data[key];
        value = v.value;

        // check if KV exists
        require(v.blocknumber > 0);

        return (value);
    }

    function tableScan() public view returns (bytes32[] memory keys, string memory values)
    {
        string memory tmp;
        uint size = keyList.length;
        keys = new bytes32[](size);

        for(uint i=0; i<size; i++) {
            keys[i] = keyList[i];
            // Concat all strings
            tmp = string(abi.encodePacked(tmp, data[keyList[i]].value, "####"));
        }

        return (keys, tmp);
    }

    function getSize() public view returns (uint size)
    {
        return (keyList.length);
    }

    function getBatch(uint key_id, uint batch_size) public view returns (bytes32[] memory keys, string memory values)
    {
        // check if KV exists
        require(key_id < keyList.length);

        string memory tmp;
        uint size = keyList.length-key_id;
        if(batch_size < size) {
            size=batch_size;
        }
        keys = new bytes32[](size);

        for(uint i=key_id; i<size+key_id; i++) {
            keys[i-key_id] = keyList[i];
            // Concat all strings
            tmp = string(abi.encodePacked(tmp, data[keyList[i]].value, "####"));
        }

        return (keys, tmp);
    }

    function remove(bytes32 key) public {

        Value memory v = data[key];

        // check if key exists
        require(v.blocknumber > 0);

        // remove from keyList: find index, then swap with last element, then call pop()
        uint size = keyList.length;
        uint index = type(uint256).max;

        for(uint i=0; i<size; i++) {
            if(keyList[i] == key) {
                index = i;
                break;
            }
        }

        keyList[index] = keyList[size - 1]; // move last element to position of key to delete
        keyList.pop();

        // delete from data
        delete data[key];
    }

    function putBatch(bytes32[] memory keys, string[] memory values) public {
        for (uint i = 0; i < keys.length; i++) {
            Value memory v = Value(block.number,values[i]);

            if(data[keys[i]].blocknumber == 0) {
                keyList.push(keys[i]);
            }

            data[keys[i]] = v;
        }
    }
}