#include <spraybus/client/client.hpp>
#include <spraybus/common/env.hpp>
#include <spraybus/common/interrupts.hpp>
#include <spraybus/networking/networking.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

struct Options {
    std::string host = "localhost";
    uint16_t port = 6767;
    std::string command;
    std::vector<std::string> args;
};

void print_usage(std::ostream& out) {
    out << "Usage:\n"
        << "  cli [--host HOST] [--port PORT] pub <topic> <message>\n"
        << "  cli [--host HOST] [--port PORT] sub <topic>\n";
}

uint16_t parse_port(std::string_view value) {
    size_t parsed = 0;
    int port = std::stoi(std::string(value), &parsed);
    if (parsed != value.size() || port < 0 ||
        port > std::numeric_limits<uint16_t>::max()) {
        throw std::out_of_range("Port must be between 0 and 65535");
    }
    return static_cast<uint16_t>(port);
}

std::string join_message(const std::vector<std::string>& args,
                         size_t first_arg) {
    std::string message;
    for (size_t i = first_arg; i < args.size(); ++i) {
        if (!message.empty()) {
            message += ' ';
        }
        message += args[i];
    }
    return message;
}

Options parse_options(int argc, char** argv) {
    Options options;
    bool host_from_args = false;
    bool port_from_args = false;

    int i = 1;
    while (i < argc) {
        std::string_view arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            options.command = "help";
            return options;
        }

        if (arg == "--host" || arg == "-H") {
            if (++i >= argc) {
                throw std::invalid_argument("--host requires a value");
            }
            options.host = argv[i++];
            host_from_args = true;
            continue;
        }

        if (arg.starts_with("--host=")) {
            options.host = std::string(arg.substr(7));
            host_from_args = true;
            ++i;
            continue;
        }

        if (arg == "--port" || arg == "-p") {
            if (++i >= argc) {
                throw std::invalid_argument("--port requires a value");
            }
            options.port = parse_port(argv[i++]);
            port_from_args = true;
            continue;
        }

        if (arg.starts_with("--port=")) {
            options.port = parse_port(arg.substr(7));
            port_from_args = true;
            ++i;
            continue;
        }

        options.command = arg;
        ++i;
        break;
    }

    for (; i < argc; ++i) {
        options.args.emplace_back(argv[i]);
    }

    if (options.command.empty()) {
        throw std::invalid_argument("missing command");
    }

    if (!host_from_args) {
        options.host =
            spraybus::common::get_env_or("SPRAYBUS_HOST", options.host);
    }

    if (!port_from_args) {
        options.port =
            parse_port(spraybus::common::get_env_or("SPRAYBUS_PORT", "6767"));
    }

    return options;
}

int run(int argc, char** argv) {
    Options options = parse_options(argc, argv);

    if (options.command == "help") {
        print_usage(std::cout);
        return 0;
    }

    spraybus::networking::init();

    spraybus::client::Client client(options.host, options.port);

    if (options.command == "pub") {
        if (options.args.size() < 2) {
            throw std::invalid_argument("pub requires <topic> and <message>");
        }

        client.publish(options.args[0], join_message(options.args, 1));
        return 0;
    }

    if (options.command == "sub") {
        if (options.args.size() != 1) {
            throw std::invalid_argument("sub requires exactly one <topic>");
        }

        client.subscribe(options.args[0]);
        spraybus::common::run_forever([&]() {
            client.process(
                [&](const spraybus::networking::protocol::Message& msg) {
                    std::cout << msg.payload_as_string() << '\n';
                    std::cout.flush();
                });
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
        return 0;
    }

    throw std::invalid_argument("unknown command: " + options.command);
}

} // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        print_usage(std::cerr);
        return 1;
    }
}
