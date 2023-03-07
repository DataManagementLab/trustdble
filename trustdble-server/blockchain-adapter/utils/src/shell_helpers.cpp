#include "adapter_utils/shell_helpers.h"

auto exec(const char *cmd, bool *exec_success) -> std::string {
  auto *pipe = popen(cmd, "r");  // execute command and capture stdout
  // this only checks if capturing the stdoutput of the command was successful
  // it does not say anything about the success of the executed command (ie. its
  // return code)
  if (pipe == nullptr) {
    throw std::runtime_error("popen() failed!");
  }

  // read the captured output and store it in a string
  std::array<char, BUFFER_SIZE_EXEC> buffer;
  std::string result;
  while (feof(pipe) == 0) {
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }
  }

  int return_code = pclose(pipe);
  *exec_success = (return_code == EXIT_SUCCESS);

  // remove trailing new line(s)
  while (result.back() == '\n') {
    result.pop_back();
  }
  // command execution was successful return captured ouput
  return result;
}

auto exec(const char *cmd) -> std::string {
  bool exec_success;
  std::string result = exec(cmd, &exec_success);

  if (!exec_success) {
    // something went wrong during the execution of the command
    // since it was a system call and we cannot recover from it
    // we throw an runtime_error
    std::string error_msg =
        "Error during execution of the following command:\n" + std::string(cmd);
    // Note: When errors occur they are usually sent to stderr and not stdout
    // Hence, in the case of an error stdout is usually empty. Below, we include
    // the ouput only for completness
    BOOST_LOG_TRIVIAL(error)
        << error_msg << "\nThe command returned the following std output:\n"
        << result;
    throw std::runtime_error(error_msg);
  }
  // command execution was successful return captured ouput
  return result;
}