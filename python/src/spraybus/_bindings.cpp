#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>

#include <pybind11/pybind11.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace py = pybind11;

namespace {

void init_networking_once() {
    static std::once_flag once;
    std::call_once(once, []() { spraybus::networking::init(); });
}

py::bytes payload_bytes(const spraybus::client::FanoutMessage& message) {
    if (message.payload.empty()) {
        return py::bytes();
    }

    return py::bytes(reinterpret_cast<const char*>(message.payload.data()),
                     message.payload.size());
}

std::span<const std::byte> byte_span(const std::string& payload) {
    return std::as_bytes(std::span<const char>(payload.data(), payload.size()));
}

} // namespace

PYBIND11_MODULE(_native, module) {
    module.doc() = "Native spraybus client bindings";

    py::class_<spraybus::client::FanoutMessage>(module, "Message")
        .def_property_readonly(
            "topic_key",
            [](const spraybus::client::FanoutMessage& message) {
                return message.topic_key;
            })
        .def_property_readonly(
            "topic",
            [](const spraybus::client::FanoutMessage& message) {
                return message.topic;
            })
        .def_property_readonly("payload", &payload_bytes)
        .def(
            "payload_text",
            [](const spraybus::client::FanoutMessage& message,
               const std::string& encoding, const std::string& errors) {
                py::object bytes = payload_bytes(message);
                return bytes.attr("decode")(encoding, errors);
            },
            py::arg("encoding") = "utf-8", py::arg("errors") = "strict");

    py::class_<spraybus::client::Client>(module, "Client")
        .def(py::init([](std::string host, uint16_t port) {
                 init_networking_once();
                 py::gil_scoped_release release;
                 return std::make_unique<spraybus::client::Client>(
                     std::move(host), port);
             }),
             py::arg("host"), py::arg("port"))
        .def(
            "get_topic_key",
            [](spraybus::client::Client& client, const std::string& topic) {
                py::gil_scoped_release release;
                return client.get_topic_key(topic);
            },
            py::arg("topic"))
        .def(
            "publish",
            [](spraybus::client::Client& client, const std::string& topic,
               py::bytes payload) {
                std::string bytes = payload;
                py::gil_scoped_release release;
                client.publish(topic, byte_span(bytes));
            },
            py::arg("topic"), py::arg("payload"))
        .def(
            "subscribe",
            [](spraybus::client::Client& client, const std::string& topic) {
                py::gil_scoped_release release;
                client.subscribe(topic);
            },
            py::arg("topic"))
        .def("poll", [](spraybus::client::Client& client) -> py::object {
            std::optional<spraybus::client::FanoutMessage> message;
            {
                py::gil_scoped_release release;
                message = client.poll();
            }

            if (!message.has_value()) {
                return py::none();
            }

            return py::cast(*message);
        });
}
