#pragma once

/**
 * @file logging.hpp
 * @brief Quill logging setup and per-class logger helpers.
 */

#include <quill/DeferredFormatCodec.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/bundled/fmt/format.h>

#include <string>
#include <string_view>
#include <vector>

namespace spraybus::common {

/**
 * @brief Pointer type used by the project for Quill loggers.
 */
using Logger = quill::Logger*;

/**
 * @brief Start the Quill backend logging thread.
 *
 * Call this once near process startup before expecting asynchronous log output.
 */
void init_logging();

/**
 * @brief Route subsequently created loggers to a file sink.
 *
 * @param filename Path to the log file used for loggers created after this
 * call.
 */
void set_logfile(const std::string& filename);

/**
 * @brief Create or retrieve a named logger.
 *
 * @param name Logger name.
 * @return A Quill logger pointer backed by the current configured sink.
 */
Logger create_logger(const std::string& name);

/**
 * @brief Mixin that gives a class a named Quill logger.
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

} // namespace spraybus::common
