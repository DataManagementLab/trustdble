/** @defgroup group60 encoding_helpers
 *  @ingroup group6
 *  @{
 */
#ifndef ENCODING_HELPERS_H
#define ENCODING_HELPERS_H

#include <iomanip>
#include <iostream>

//! default encode byte size  
#define ENCODED_BYTE_SIZE 16

//! keys and values of smart contrat are 32 byte and represented as hex
//! strings with 64 chars. Each byte as 2 chars
#define VALUE_SIZE 64

/**
 * @brief Helper-Method to convert an integer into its hex-representation as
 * string.
 *
 * @param num The integer to be converted.
 *
 * @param size The total size of the string output. Default=VALUE_SIZE of
 * ethereum (64).
 *
 * @return Hex-encoded string of lenght size. It will be filled up with
 * leading zeros.
 */
auto int_to_hex(size_t num, int size = VALUE_SIZE) -> std::string;

/**
 * @brief Helper-Method to convert a hex-coded string into an integer.
 *
 * @param hex The hex-encoded string to be converted.
 *
 * @return Integer of the hex-coded string.
 */
auto hex_to_int(const std::string &hex) -> int;

/**
 * @brief Helper-Method to convert a hex-coded char into an integer.
 *
 * @param hex The hex-encoded char to be converted.
 *
 * @return Integer of the hex-coded char.
 */
auto charToInt( unsigned char input) -> int;

/**
 * @brief Helper-Method to convert an byte array into its hex-representation.
 *
 * @param data The byte array to be converted.
 *
 * @param length the length of the array that should be converted to hex
 * (Hexadecimal) string
 *
 * @return Hex-encoded string
 */
auto byte_array_to_hex(const unsigned char *data, unsigned length)
    -> std::string;

/**
 * @brief Helper-Method to convert an string into its hex-representation.
 *
 * @param data The string to be converted.
 *
 * @return Hex-encoded string
 */
auto string_to_hex(const std::string &in) -> std::string;

/**
 * @brief The method converts the string to hex (Hexadecimal) string
 *
 * @param[in] in the string that should be converted to hex (Hexadecimal) string
 * @param[in] length the length of string that should be converted to hex
 * (Hexadecimal) string
 * @return the converted hex (Hexadecimal) string
 */
auto charArrayToHex(const unsigned char *data, unsigned length) -> std::string;

/**
 * @brief The method converts hex (Hexadecimal) string to string
 *
 * @param[in] in the hex (Hexadecimal) string that should be converted to string
 * @param[in] data the converted hex to string
 * @return the converted string
 */
auto hexToCharArray(const std::string &in, unsigned char *data) -> int;

/**
 * @brief The method converts hex (Hexadecimal) string to byte array
 *
 * @param[in] in the hex (Hexadecimal) string that should be converted to byte array
 * @param[in] data the converted hex to byte array
 * @return the converted byte array
 */
void hex_to_byte_array(const std::string &in, unsigned char *data);

/**
 * @brief Helper-Method to convert an hex-coded string into readable string.
 *
 * @param str The hex-encoded string to be converted.
 *
 * @return Readable string
 */
auto hex_to_string(const std::string &str) -> std::string;

#endif  // ENCODING_HELPERS_H
/** @} */