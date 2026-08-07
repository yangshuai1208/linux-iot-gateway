#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace reliable
{

constexpr std::uint8_t kHeader1 = 0xAA;
constexpr std::uint8_t kHeader2 = 0x55;
constexpr std::uint8_t kTail1 = 0x0D;
constexpr std::uint8_t kTail2 = 0x0A;

enum class FrameType : std::uint8_t
{
    Request = 0x01,
    Ack = 0x02
};

enum class CommandCode : std::uint8_t
{
    Open = 0x01,
    Grab = 0x02,
    Release = 0x03,
    Stop = 0x04
};

enum class ResultCode : std::uint8_t
{
    Ok = 0x00,
    InvalidCommand = 0x01,
    ExecutionFailed = 0x02
};

struct RequestFrame
{
    std::uint16_t sequence;
    CommandCode command;
};

struct AckFrame
{
    std::uint16_t sequence;
    ResultCode result;
};

class ProtocolCodec
{
public:
    static std::vector<std::uint8_t>
    encodeRequest(const RequestFrame &frame);

    static std::vector<std::uint8_t>
    encodeAck(const AckFrame &frame);

    static bool decodeRequest(
        const std::vector<std::uint8_t> &bytes,
        RequestFrame &frame);

    static bool decodeAck(
        const std::vector<std::uint8_t> &bytes,
        AckFrame &frame);

    static std::string commandToString(
        CommandCode command);

private:
    static std::uint8_t calculateChecksum(
        const std::vector<std::uint8_t> &bytes,
        std::size_t begin,
        std::size_t end);
};

} // namespace reliable