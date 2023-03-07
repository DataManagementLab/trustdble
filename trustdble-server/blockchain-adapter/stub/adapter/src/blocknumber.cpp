/*! \addtogroup group22
 *  @{
 */
#include "blocknumber.h"

void BlockNumber::init(const std::string &path, int blocksize) {
  blockSize_ = blocksize;
  blockNumberFilename_ = path;
  blockNumberFile_.open(blockNumberFilename_);
  blockNumberFile_ << "1" << std::endl;
  blockNumberFile_.close();
};

void BlockNumber::incrementTransactionCount() {
  int transaction_count = getTransactionCount();
  blockNumberFile_.open(blockNumberFilename_);
  blockNumberFile_ << transaction_count + 1 << std::endl;
  blockNumberFile_.close();
}

auto BlockNumber::get() -> int {
  int transaction_count = getTransactionCount();
  return transaction_count / blockSize_;
}

auto BlockNumber::getTransactionCount() -> int {
  std::ifstream reader(blockNumberFilename_);

  if (reader.is_open()) {
    std::string transaction_count;
    getline(reader, transaction_count);
    blockNumberFile_.close();

    return std::stoi(transaction_count);
  }

  throw std::runtime_error("Couldn't read from file");
};
/** @} */