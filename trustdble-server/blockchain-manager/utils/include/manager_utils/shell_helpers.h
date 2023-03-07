#ifndef SHELL_HELPERS_H
#define SHELL_HELPERS_H

#include <string>

// size for buffer to get return after executing node command
#define BUFFER_SIZE_EXEC 128

const int kRetryExecBackoff = 100;

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

/**
 * @brief Multiple attempts to execute a command via cmd if the execution no
 * success or not time out and capture the last output.
 *
 * @param cmd The command to be executed on cmd
 * @param exec_success True, if the command exits successful and false
 * otherweise
 * @param timeout Timeout
 * @return std::string The capture standard output of the executed command
 */
auto retry_exec(const char *cmd, bool *exec_success,
                       unsigned long timeout) -> std::string;

/**
 * @brief this method checks if a port is being currently used by the host
 * machine
 *
 * we use a docker command to list the ports being used by the docker containers
 * and check if the given port is included in the list.
 *
 * @param port this is the port that we will check if it is being used
 * @return bool true if the port is in use, otherwise false
 *
 */
auto port_in_use(unsigned short port) -> bool;

#endif // SHELL_HELPERS_H