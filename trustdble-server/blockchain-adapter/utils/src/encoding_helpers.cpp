#include "adapter_utils/encoding_helpers.h"

auto int_to_hex(size_t num, int size) -> std::string {
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(size) << std::hex << num;
  return stream.str();
}

auto hex_to_int(const std::string &hex) -> int {
  std::stringstream stream;
  stream << std::hex << hex;
  int i;
  stream >> i;
  return i;
}
static char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                          '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

auto byte_array_to_hex(const unsigned char *data, unsigned length)
    -> std::string {

  std::string s(length * 2, ' ');
  unsigned i = 0;
  for (; i < length; ++i) {
    s[2 * i]     = hexmap[(data[i] & 0xF0) >> 4];
    s[2 * i + 1] = hexmap[data[i] & 0x0F];
  }
  return s;
}

auto string_to_hex(const std::string &in) -> std::string {
  unsigned length = in.length();
  const unsigned char *buffer = (const unsigned char *)in.c_str();
  // buffer = &in[0];

  return byte_array_to_hex(buffer, length);
}

auto charArrayToHex(const unsigned char *data, unsigned length) -> std::string {
  return byte_array_to_hex(data, length);
}

auto charToInt( unsigned char input) -> int{
  if(input >= '0' && input <= '9') {
    return input - '0';
}
  if(input >= 'A' && input <= 'F') {
    return input - 'A' + 10;
}
  if(input >= 'a' && input <= 'f') {
    return input - 'a' + 10;
}
  throw std::invalid_argument("Invalid input string");
}

auto hexToCharArray(const std::string &in, unsigned char *data) -> int {
  if ((in.length() % 2) != 0) {
    std::cout << "String is not valid length ...";
    return 1;
  }
  try{
    const char* in_data = in.c_str();
    while((*in_data != 0) && (in_data[1] != 0))
    {
      *(data++) = charToInt(*in_data)*16 + charToInt(in_data[1]);
      in_data += 2;
    }
    return 0;
  }
  catch(std::invalid_argument const& exc){
    return 1;
  }

}

void hex_to_byte_array(const std::string &in, unsigned char *data) {
  int error = hexToCharArray(in, data);
  if (error == 1) {
    throw std::runtime_error("String is not valid length ...");
  }
}

auto hex_to_string(const std::string &str) -> std::string {
  std::string output;

  for (unsigned int i = 0; i < str.length(); i += 2) {
    std::string byte_string = str.substr(i, 2);
    char c = (char)strtol(byte_string.c_str(), nullptr, ENCODED_BYTE_SIZE);
    if (c != '\0') {
      output += c;
    }
  }
  return output;
}
