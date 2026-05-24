#pragma once

#include <quill/DeferredFormatCodec.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/Logger.h>
#include <quill/bundled/fmt/format.h>

#include <string>
#include <string_view>
#include <vector>

namespace spraybus::common {

using Logger = quill::Logger*;

void init_logging();

void set_logfile(const std::string& filename);

Logger create_logger(const std::string& name);

class ClassLogger {
  private:
    Logger m_logger;

  public:
    ClassLogger(const std::string& name) : m_logger(create_logger(name)) {}
    Logger logger() { return m_logger; }
    const Logger logger() const { return m_logger; }
};

} // namespace spraybus::common
