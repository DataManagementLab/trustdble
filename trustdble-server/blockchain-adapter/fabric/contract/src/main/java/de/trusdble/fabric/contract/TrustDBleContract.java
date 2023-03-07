/* Licensed under Apache-2.0 2020. */
package de.trusdble.fabric.contract;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.codec.DecoderException;
import com.google.gson.Gson;
import org.hyperledger.fabric.contract.Context;
import org.hyperledger.fabric.contract.ContractInterface;
import org.hyperledger.fabric.contract.annotation.*;
import org.hyperledger.fabric.shim.ChaincodeException;
import org.hyperledger.fabric.shim.ChaincodeStub;
import org.hyperledger.fabric.shim.ledger.KeyValue;
import org.hyperledger.fabric.shim.ledger.QueryResultsIterator;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.logging.Logger;

/** Define TrustDBle smart contract by extending Fabric Contract class */
@Contract(name = "de.trustdble.fabric.contract", info = @Info(title = "TrustDBle contract", description = "Contract for TrustDBle", version = "0.0.1", license = @License(name = "SPDX-License-Identifier: ", url = ""), contact = @Contact(email = "trustdble-contract@example.com", name = "Simon Karrer")))
@Default
public class TrustDBleContract implements ContractInterface {

	// use the classname for the logger, this way you can refactor
	private static final Logger LOG = Logger.getLogger(TrustDBleContract.class.getName());
	private static final String DELIMITER = "#";
	private static final String RANGE_END_DELIMITER = "$";

	/**
	 * Instantiate to perform any setup of the ledger that might be required.
	 *
	 * @param {Context} ctx the transaction context
	 */
	@Transaction
	public void instantiate(final Context ctx) {
		// No implementation required with this example
		// It could be where data migration is performed, if necessary
		LOG.info("No data migration to perform");
	}

	/**
	 * Put a batch of key-value pairs on the ledger.
	 *
	 * @param ctx the transaction context
	 * @param json_key_value_pairs json encoded array of key value pairs
	 * @return status message
	 */
	@Transaction(intent = Transaction.TYPE.SUBMIT)
	public void put(final Context ctx, final String table, final String json_key_value_pairs) {
		ChaincodeStub stub = ctx.getStub();

		Gson gson = new Gson();
		Map<String, String> key_value_pairs = gson.fromJson(json_key_value_pairs, Map.class);

		try {
			Hex.decodeHex(table);
		} catch (DecoderException e) {
			throw new ChaincodeException("Ilegal table name: table name must be hex encoded");
		}

		try {
			for (Map.Entry<String, String> pair : key_value_pairs.entrySet()) {
				// key, value and table name are hex encoded
				String compositeKey = table + DELIMITER + pair.getKey();
				stub.putState(compositeKey, Hex.decodeHex(pair.getValue()));
			}
		} catch (DecoderException e) {
			throw new ChaincodeException("Error decoding hex encoded value: %s", e.getMessage());
		}

		return;
	}

	/**
	 * Retrieves value with the specified key from the ledger.
	 *
	 * @param ctx   the transaction context
	 * @param rowID key of the pair
	 * @return the value found on the ledger if there was one
	 */
	@Transaction(intent = Transaction.TYPE.EVALUATE)
	public String get(final Context ctx, final String table, final String key) {
		ChaincodeStub stub = ctx.getStub();

		try {
			Hex.decodeHex(table);
		} catch (DecoderException e) {
			throw new ChaincodeException("Ilegal table name: table name must be hex encoded");
		}

		String compositeKey = table + DELIMITER + key;
		byte[] value = stub.getState(compositeKey);

		if (value == null || value.length == 0) {
			String errorMessage = String.format(" Key %s does not exists", key);
			LOG.warning(errorMessage);
			throw new ChaincodeException(errorMessage, "Key not found");
		}

		return Hex.encodeHexString(value);
	}

	/**
	 * Retrieves all key-value pairs from the ledger.
	 *
	 * @param ctx the transaction context
	 * @return Pairs found on the ledger as separated string 
	 */
	@Transaction(intent = Transaction.TYPE.EVALUATE)
	public String getAll(final Context ctx, final String table) {
		ChaincodeStub stub = ctx.getStub();

		try {
			Hex.decodeHex(table);
		} catch (DecoderException e) {
			throw new ChaincodeException("Ilegal table name: table name must be hex encoded");
		}

		Map<String, String> results = new HashMap<String, String>();

		String startKey = table + DELIMITER;
		String endKey = table +  RANGE_END_DELIMITER;
		QueryResultsIterator<KeyValue> resultsIterator = stub.getStateByRange(startKey, endKey);

		for (KeyValue result : resultsIterator) {
			// encode value as hex, because byte arrays are also hex encoded for put
			// key is already hex encoded
			int delimiterIndex = result.getKey().indexOf(DELIMITER);
			String key = result.getKey().substring(delimiterIndex + 1);
			results.put(key, Hex.encodeHexString(result.getValue()));
		}

		Gson gson = new Gson();
		return gson.toJson(results);
	}

	/**
	 * Deletes a key-value pair on the ledger.
	 *
	 * @param ctx   the transaction context
	 * @param key key of the pair being deleted
	 * @return status message
	 */
	@Transaction(intent = Transaction.TYPE.SUBMIT)
	public void delete(final Context ctx, final String table, final String json_keys) {
		ChaincodeStub stub = ctx.getStub();

		Gson gson = new Gson();
		List<String> keys = gson.fromJson(json_keys, List.class);

		try {
			Hex.decodeHex(table);
		} catch (DecoderException e) {
			throw new ChaincodeException("Ilegal table name: table name must be hex encoded");
		}
	
		for (String key : keys) {
			// key and table name are hex encoded
			String compositeKey = table + DELIMITER + key;

			if (!keyExists(ctx, compositeKey)) {
				String errorMessage = String.format("Key %s does not exist", key);
				LOG.warning(errorMessage);
				throw new ChaincodeException(errorMessage, "Key not found");
			}

			stub.delState(compositeKey);
		}

		return;
	}

	/**
	 * Checks the existence of the pair on the ledger
	 *
	 * @param ctx   the transaction context
	 * @param rowID key of the pair
	 * @return boolean indicating the existence of the pair
	 */
	@Transaction(intent = Transaction.TYPE.EVALUATE)
	public boolean keyExists(final Context ctx, final String key) {
		ChaincodeStub stub = ctx.getStub();
		String value = stub.getStringState(key);

		return (value != null && !value.isEmpty());
	}
}