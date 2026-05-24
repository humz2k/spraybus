#include <spraybus/client/client.hpp>
#include <spraybus/networking/networking.hpp>
#include <spraybus/networking/protocol.hpp>

#include <cstddef>

int main() {
    spraybus::networking::init();

    spraybus::networking::protocol::Constructor constructor;
    auto packet = constructor.topic_request("alerts");

    if (packet.size() <= sizeof(spraybus::networking::protocol::Header)) {
        return 1;
    }

    return 0;
}
