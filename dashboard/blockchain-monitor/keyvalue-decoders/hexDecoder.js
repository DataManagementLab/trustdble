const { utils } = require('web3');
const crypto = require('crypto');
const { KeyValueDecoder } = require('./keyValueDecoder');

const HEX_LENGTH = {
  TINYINT: 2,
  SMALLINT: 4,
  MEDIUMINT: 6,
  INT: 8,
  BIGINT: 16,
  // Most of the characters requires 1 byte storage but some special characters like; € take 3 bytes
  // MySQL by default encode a character in 4 bytes
  // means it use 8 character length of hex string for a character
  CHAR: 8,
  VARCHAR_HEAD: 2,
  DATE: 6,
};

// Radix can be between 2 to 32 but in our case we are decoding a HexString so it should be 16
// It will use in some functions like parseInt();
const RADIX = 16;

/**
 * HexDecoder that is used if the rows are encoded as bytes and follow the SQL encoding schema.
 *
 * Example:
 *
 * If your SQL table schema is e.g. (id INT, name VARCHAR(20), birth_date DATE)
 * then first column will be your key which is id in this example
 * and the rest of the row are values, your schema should look like this:
 *
 * tableSchema = ["int", "varchar(20)", "date"]
 *
 */
class HexDecoder extends KeyValueDecoder {
  /**
     * Decode input data from blockchain transactions.
     *
     * @param {*[object]} tuples List of tuples from blockchain adapter
     * @param {object} tableSchemasDict Dictionaries of table schemas where table addresses are keys
     * @returns {*[object]} List of decoded tuples
     */
  decode(tuples, tableNamesDict, keysAndSchemaDict) {
    return tuples.filter(
      (tuple) => Object.keys(tableNamesDict).includes(tuple.contractAddress),
    ).map((tuple) => {
      if (tuple.txType === 'REMOVE') {
        tuple.keyHash = tuple.data;
        tuple.value = null;
        delete tuple.data;
        return tuple;
      }
      const {
        tableSchema,
        encryptionKey,
        iv,
      } = keysAndSchemaDict[tableNamesDict[tuple.contractAddress]];

      const valueHexLength = this._getValueHexLength(tableSchema);
      let data = null;

      if (encryptionKey !== '') {
        const encryptedHexLength = this.constructor._encryptedHexLength(valueHexLength);
        data = this.constructor._decrypt(
          tuple.data.substring(tuple.data.length - encryptedHexLength),
          encryptionKey,
          iv,
        );
      } else {
        data = tuple.data.substring(tuple.data.length - valueHexLength);
      }

      tuple.keyHash = tuple.data.substring(0, 64);
      tuple.value = this._decodeValue(`0x${data}`, tableSchema);
      delete tuple.data;
      return tuple;
    });
  }

  /**
   * Decrypt encrypted data by TRUSTDBLE-server using encryption key
   *
   * References:
   * https://codeforgeek.com/encrypt-and-decrypt-data-in-node-js
   * https://attacomsian.com/blog/nodejs-encrypt-decrypt-data
   *
   * @param {string} encryptedData Encrypted data received from blockchain transaction
   * @param {string} encryptionKey Encryption key obtained from meta_table
   * @returns {string} Decrypted data
   * @private
   */
  static _decrypt(encryptedData, encryptionKey, iv) {
    const algorithm = 'aes-256-cbc';
    const decipher = crypto.createDecipheriv(algorithm, Buffer.from(encryptionKey, 'hex'), Buffer.from(iv, 'hex'));
    decipher.setAutoPadding(false);
    const decrypted = Buffer.concat([decipher.update(Buffer.from(encryptedData, 'hex')), decipher.final()]);
    return decrypted.toString('hex');
  }

  /**
   * Calculate encrypted key and value length
   *
   * @param {number} hexLength
   * @returns {number} Encrypted data length
   * @private
   */
  static _encryptedHexLength(hexLength) {
    return (Math.floor(hexLength / 32) + 1) * 32;
  }

  /**
     * Reverse the Endianness(oder of bytes) of the given bytestring
     * Bytestring needs to be of even length (i.e. 2 hex characters represent one byte)
     *
     * @param {string} bytestring Hex Bytestring to be reversed (i.e. "0x011101")
     * @returns {string} Hex Bytestring with reversed Endianness
     * @private
     */
  static _reverseEndianness(bytestring) {
    let reversed = '0x';
    // 1 byte is quale to 2 Hex string length.
    // For example we want to reverse the order of '0x01101100' -> 4 bytes
    // We 4 pairs -> 01 10 11 00 and reverse order will be 00 11 10 01 -> '0x00111001'
    for (let i = bytestring.length; i > 2; i -= 2) {
      reversed += bytestring.slice(i - 2, i);
    }
    return reversed;
  }

  /**
     * Decode a hex bytestring that contains an integer into the corresponding number.
     * Bytestring may be padded.
     *
     * @param {string} bytestring Little Endian Hex Bytestring representation of the integer
     * @returns {number} Decoded Integer as Number
     * @private
     */
  _decodeInt(bytestring) {
    return parseInt(this.constructor._reverseEndianness(bytestring), RADIX);
  }

  /**
     * Decode a hex bytestring that represents an Utf8-encoded CHAR into the corresponding string.
     *
     * @param {string} bytestring Hex bytestring representation of the CHAR
     * @returns {string} Decoded string as Utf8 String
     * @private
     */
  static _decodeChar(bytestring) {
    // hexToUtf8() decode correctly special characters
    // like '€'. hexToAscii() can not decode '0xe282ac' -> '€'
    return utils.hexToUtf8(bytestring);
  }

  /**
     * Decode a hex bytestring that represents an Utf8-encoded VARCHAR into the corresponding string
     * Leftmost byte is expected to be the length of the string. Rest is the actual string.
     * Bytestring may be padded.
     *
     * @param {string} bytestring Hex bytestring representation of the VARCHAR
     * @returns {string} Decoded string as Utf8 String
     * @private
     */
  _decodeVarchar(bytestring) {
    // First two characters of Hex string after '0x' represents the number of bytes
    // which are used for current input, For example '0x0345d67e000000000....'
    // So '03' is length and (length*2) will the length of hex string for data
    const length = parseInt(bytestring.slice(2, 4), RADIX);
    const data = `0x${bytestring.slice(4, 4 + length * 2)}`;
    return this.constructor._decodeChar(data);
  }

  /**
     * Decode a hex bytestring that represents a DATE
     * into the corresponding YYYY-MM-DD string.
     *
     * @param {string} bytestring Hex bytestring of the DATE
     * @returns {string} Decoded date as string.
     * @private
     */
  _decodeDate(bytestring) {
    // DATE: A three-byte integer packed as YYYY×16×32 + MM×32 + DD
    const dateInt = this._decodeInt(bytestring);
    // 32 is common in year and month while encode so with module of 32 we can get exact day
    const day = dateInt % 32;
    // after subtract day from total we need to divide by 32 then module of 16
    const month = ((dateInt - day) / 32) % 16;
    const year = (dateInt - day - month * 32) / (16 * 32);

    return `${year}-${month}-${day}`;
  }

  /**
     * Get the Hex Bytestring length of 'char' type.
     *
     * @param {int} length length of char
     * @returns {int} Hex Bytestring length.
     * @private
     */
  static _getCharLength(length) {
    return length * HEX_LENGTH.CHAR;
  }

  /**
     * Get the Hex Bytestring length of 'varchar' type.
     *
     * @param {int} length length of char
     * @returns {int} Hex Bytestring length.
     * @private
     */
  static _getVarcharLength(length) {
    return HEX_LENGTH.VARCHAR_HEAD + (length * HEX_LENGTH.CHAR);
  }

  /**
     * Check if given column type is char or varchar, if it will char or varchar
     * Then it will comes with length like: char(2) or varchar(20)
     * So we need to separate type and length
     *
     * @param {string} columnSchema Type of column, it can like "int" or varchar(20)
     * @returns {[string, int]} List of column type and length
     * @private
     */
  static _extractTypeAndParameters(columnSchema) {
    const typeAndLength = columnSchema.split(/[\s()]+/);
    if (typeAndLength.length > 1) {
      return { columnType: typeAndLength[0], parameters: typeAndLength[1] };
    }
    return { columnType: typeAndLength[0] };
  }

  /**
     * Get the Hex Bytestring length of a column against column data type.
     *
     * @param {string} columnSchema columnSchema with length or not like; "varchar(20)" OR "int"
     * @returns {int} Hex Bytestring length.
     * @private
     */
  _getColumnHexLength(columnSchema) {
    const { columnType, parameters } = this.constructor._extractTypeAndParameters(columnSchema);

    switch (columnType) {
      case 'tinyint':
        return HEX_LENGTH.TINYINT;
      case 'smallint':
        return HEX_LENGTH.SMALLINT;
      case 'mediumint':
        return HEX_LENGTH.MEDIUMINT;
      case 'int':
        return HEX_LENGTH.INT;
      case 'bigint':
        return HEX_LENGTH.BIGINT;
      case 'char':
        return this.constructor._getCharLength(parameters);
      case 'varchar':
        return this.constructor._getVarcharLength(parameters);
      case 'date':
        return HEX_LENGTH.DATE;
      default:
        return 0;
    }
  }

  /**
     * Get the decode Function of hexString against data type.
     *
     * @param {string} columnSchema columnSchema with length or not like; "varchar(20)" OR "int"
     * @returns {function} return decode function against data type of column.
     * @private
     */
  _getDecodeFunction(columnSchema) {
    const { columnType } = this.constructor._extractTypeAndParameters(columnSchema);

    switch (columnType) {
      case 'int':
      case 'tinyint':
      case 'smallint':
      case 'mediumint':
      case 'bigint':
        return this._decodeInt.bind(this);
      case 'varchar':
        return this._decodeVarchar.bind(this);
      case 'char':
        return this.constructor._decodeChar.bind(this);
      case 'date':
        return this._decodeDate.bind(this);
      default:
        return undefined;
    }
  }

  /**
     * Decodes a valueData provided as bytestring and
     * returns the decoded string as CSV by using valueSchema
     *
     * @param {string} bytestring Bytestring to be parsed
     * @param {*[string]} valueSchema List of strings like; ["int", varchar(20)", "date", "char(2)"]
     * @returns {string} Decoded value as csvString by using valueSchema.
     * @private
     */
  _decodeValue(bytestring, valueSchema) {
    let csvString = '';
    let remainingBytestring = bytestring;
    for (const columnSchema of valueSchema) {
      const columnHexLength = this._getColumnHexLength(columnSchema);
      const decodeFunction = this._getDecodeFunction(columnSchema);

      // If columnHexLength is 6 then the length of relevantData should be 2+6=8, 2 for '0x'
      const relevantData = remainingBytestring.slice(0, 2 + columnHexLength);
      remainingBytestring = `0x${remainingBytestring.slice(2 + columnHexLength)}`;
      csvString += `,${decodeFunction(relevantData)}`;
    }

    return csvString.substring(1);
  }

  /**
     * Get the Hex Bytestring length of valueData according to data types from valueSchema.
     *
     * @param {*[string]} valueSchema List of strings like; ["int", varchar(20)", "date", "char(2)"]
     * @returns {int} Value Hex Bytestring length.
     * @private
     */
  _getValueHexLength(valueSchema) {
    let valueHexLength = 0;
    for (const columnSchema of valueSchema) {
      valueHexLength += this._getColumnHexLength(columnSchema);
    }

    return valueHexLength;
  }
}

module.exports = { HexDecoder };
