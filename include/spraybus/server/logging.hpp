#pragma once

/**
 * @file logging.hpp
 * @brief Quill logging setup and per-class logger helpers for server code.
 */

#include <quill/LogMacros.h>
#include <quill/Logger.h>

#include <string>

namespace spraybus::server {

/**
 * @brief Pointer type used by server components for Quill loggers.
 */
using Logger = quill::Logger*;

/**
 * @brief Start the Quill backend logging thread.
 *
 * Call this once near process startup before expecting asynchronous log output.
 */
void init_logging();

/**
 * @brief Route subsequently created server loggers to a file sink.
 *
 * @param filename Path to the log file used for loggers created after this
 * call.
 */
void set_logfile(const std::string& filename);

/**
 * @brief Create or retrieve a named server logger.
 *
 * @param name Logger name.
 * @return A Quill logger pointer backed by the current configured sink.
 */
Logger create_logger(const std::string& name);

/**
 * @brief Mixin that gives a server class a named Quill logger.
 */
class ClassLogger {
  private:
    Logger m_logger;

  public:
    /**
     * @brief Construct a logger mixin with the given logger name.
     *
     * @param name Logger name.
     */
    ClassLogger(const std::string& name) : m_logger(create_logger(name)) {}

    /**
     * @brief Return the class logger.
     */
    Logger logger() { return m_logger; }

    /**
     * @brief Return the class logger from a const object.
     */
    const Logger logger() const { return m_logger; }
};

} // namespace spraybus::server
