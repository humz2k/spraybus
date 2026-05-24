#include <spraybus/server/logging.hpp>

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/FileSink.h>

#include <optional>
#include <string>

namespace spraybus::server {

void init_logging() { quill::Backend::start(); }

static std::optional<std::string> logfile;

void set_logfile(const std::string& filename) { logfile = filename; }

Logger create_logger(const std::string& name) {
    if (logfile) {
        return quill::Frontend::create_or_get_logger(
            name,
            quill::Frontend::create_or_get_sink<quill::FileSink>(*logfile));
    }

    return quill::Frontend::create_or_get_logger(
        name,
        quill::Frontend::create_or_get_sink<quill::ConsoleSink>("default"));
}

} // namespace spraybus::server
