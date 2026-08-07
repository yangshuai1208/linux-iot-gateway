#include "reliable_protocol.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

namespace
{

struct DeviceStats
{
    int received{0};
    int executed{0};
    int duplicates{0};
};

class SimulatedDevice
{
public:
    std::vector<std::uint8_t> handleRequest(
        const std::vector<std::uint8_t> &bytes)
    {
        reliable::RequestFrame request;

        if (!reliable::ProtocolCodec::
                decodeRequest(
                    bytes,
                    request))
        {
            return {};
        }

        ++stats_.received;

        const auto iterator =
            ackCache_.find(
                request.sequence);

        if (iterator != ackCache_.end())
        {
            ++stats_.duplicates;

            std::cout
                << "[DUPLICATE] seq="
                << request.sequence
                << " skip execution"
                << '\n';

            return reliable::ProtocolCodec::
                encodeAck(iterator->second);
        }

   
        ++stats_.executed;

        std::cout
            << "[EXECUTE] seq="
            << request.sequence
            << " command="
            << reliable::ProtocolCodec::
                   commandToString(
                       request.command)
            << '\n';

        reliable::AckFrame ack{
            request.sequence,
            reliable::ResultCode::Ok};

       
        ackCache_[request.sequence] =
            ack;

        return reliable::ProtocolCodec::
            encodeAck(ack);
    }

    const DeviceStats &getStats() const
    {
        return stats_;
    }

private:
    std::map<
        std::uint16_t,
        reliable::AckFrame>
        ackCache_;

    DeviceStats stats_;
};

} // namespace
int main()
{
    SimulatedDevice device;

    const reliable::RequestFrame request{
        3,
        reliable::CommandCode::Grab};

    const auto requestBytes =
        reliable::ProtocolCodec::
            encodeRequest(request);

    std::cout
        << "[GATEWAY] send seq=3 attempt=1"
        << '\n';

    /*
     * Device 第一次执行成功。
     */
    const auto firstAck =
        device.handleRequest(
            requestBytes);

    /*
     * 模拟 ACK 在链路中丢失。
     * Gateway 没有收到 firstAck。
     */
    (void)firstAck;

    std::cout
        << "[GATEWAY] ACK timeout seq=3"
        << '\n';

    std::cout
        << "[GATEWAY] retry seq=3 attempt=2"
        << '\n';

    /*
     * 同一个 sequence 再次发送。
     */
    const auto secondAck =
        device.handleRequest(
            requestBytes);

    reliable::AckFrame decodedAck;

    if (reliable::ProtocolCodec::
            decodeAck(
                secondAck,
                decodedAck))
    {
        std::cout
            << "[GATEWAY] ACK received seq="
            << decodedAck.sequence
            << '\n';
    }

    const DeviceStats &stats =
        device.getStats();

    std::cout
        << "[RESULT] received="
        << stats.received
        << '\n';

    std::cout
        << "[RESULT] executed="
        << stats.executed
        << '\n';

    std::cout
        << "[RESULT] duplicates="
        << stats.duplicates
        << '\n';

    return 0;
}