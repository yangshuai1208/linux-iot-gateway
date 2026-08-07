#include "reliable_protocol.h"

namespace reliable
{

std::uint8_t ProtocolCodec::calculateChecksum(
    const std::vector<std::uint8_t> &bytes,
    std::size_t begin,
    std::size_t end)
{
    std::uint8_t checksum = 0;

    for (std::size_t index = begin;
         index < end;
         ++index)
    {
        checksum ^= bytes[index];
    }

    return checksum;
}

std::vector<std::uint8_t>
ProtocolCodec::encodeRequest(
    const RequestFrame &frame)
{
    std::vector<std::uint8_t> bytes{
        kHeader1,
        kHeader2,
        0x04,
        static_cast<std::uint8_t>(
            (frame.sequence >> 8) & 0xFF),
        static_cast<std::uint8_t>(
            frame.sequence & 0xFF),
        static_cast<std::uint8_t>(
            FrameType::Request),
        static_cast<std::uint8_t>(
            frame.command)};

    const std::uint8_t checksum =
        calculateChecksum(
            bytes,
            2,
            bytes.size());

    bytes.push_back(checksum);
    bytes.push_back(kTail1);
    bytes.push_back(kTail2);

    return bytes;
}

std::vector<std::uint8_t>
ProtocolCodec::encodeAck(
    const AckFrame &frame)
{
    std::vector<std::uint8_t> bytes{
        kHeader1,
        kHeader2,
        0x04,
        static_cast<std::uint8_t>(
            (frame.sequence >> 8) & 0xFF),
        static_cast<std::uint8_t>(
            frame.sequence & 0xFF),
        static_cast<std::uint8_t>(
            FrameType::Ack),
        static_cast<std::uint8_t>(
            frame.result)};

    const std::uint8_t checksum =
        calculateChecksum(
            bytes,
            2,
            bytes.size());

    bytes.push_back(checksum);
    bytes.push_back(kTail1);
    bytes.push_back(kTail2);

    return bytes;
}

bool ProtocolCodec::decodeRequest(
    const std::vector<std::uint8_t> &bytes,
    RequestFrame &frame)
{
    if (bytes.size() != 10)
    {
        return false;
    }

    if (bytes[0] != kHeader1 ||
        bytes[1] != kHeader2)
    {
        return false;
    }

    if (bytes[8] != kTail1 ||
        bytes[9] != kTail2)
    {
        return false;
    }

    if (bytes[5] !=
        static_cast<std::uint8_t>(
            FrameType::Request))
    {
        return false;
    }

    const std::uint8_t checksum =
        calculateChecksum(
            bytes,
            2,
            7);

    if (checksum != bytes[7])
    {
        return false;
    }

    frame.sequence =
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(
                 bytes[3])
             << 8) |
            bytes[4]);

    frame.command =
        static_cast<CommandCode>(
            bytes[6]);

    return true;
}

bool ProtocolCodec::decodeAck(
    const std::vector<std::uint8_t> &bytes,
    AckFrame &frame)
{
    if (bytes.size() != 10)
    {
        return false;
    }

    if (bytes[0] != kHeader1 ||
        bytes[1] != kHeader2)
    {
        return false;
    }

    if (bytes[8] != kTail1 ||
        bytes[9] != kTail2)
    {
        return false;
    }

    if (bytes[5] !=
        static_cast<std::uint8_t>(
            FrameType::Ack))
    {
        return false;
    }

    const std::uint8_t checksum =
        calculateChecksum(
            bytes,
            2,
            7);

    if (checksum != bytes[7])
    {
        return false;
    }

    frame.sequence =
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(
                 bytes[3])
             << 8) |
            bytes[4]);

    frame.result =
        static_cast<ResultCode>(
            bytes[6]);

    return true;
}

std::string ProtocolCodec::commandToString(
    CommandCode command)
{
    switch (command)
    {
    case CommandCode::Open:
        return "OPEN";

    case CommandCode::Grab:
        return "GRAB";

    case CommandCode::Release:
        return "RELEASE";

    case CommandCode::Stop:
        return "STOP";

    default:
        return "UNKNOWN";
    }
}

} // namespace reliable