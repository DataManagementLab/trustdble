/** @defgroup group61 shell_helpers
 *  @ingroup group6
 *  @{
 */
#ifndef SHELL_HELPERS_H
#define SHELL_HELPERS_H

#include <boost/log/trivial.hpp>

//! size for buffer to get return after executing node command
#define BUFFER_SIZE_EXEC 128

/**
 * @brief Execute a command via cmd and capture its output.
 *
 * @param cmd The command to be executed on cmd
 * @param exec_success  True, if the command exits successful and false
 * otherweise
 * @return std::string The capture standard output of the executed command
 */
auto exec(const char *cmd, bool *exec_success) -> std::string;

/**
 * @brief Helper-Method to execute a system command e.g. "docker..." and get
 * the std output as return value.
 *
 * @param cmd Command to be executed
 *
 * @return Std output of the command
 */
auto exec(const char *cmd) -> std::string;

#endif  // SHELL_HELPERS_H
/** @} */