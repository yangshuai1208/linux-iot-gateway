#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace serialproto
{

constexpr std::uint8_t kHeader1 = 0xAA;
constexpr std::uint8_t kHeader2 = 0x55;

constexpr std::uint8_t kTail1 = 0x0D;
constexpr std::uint8_t kTail2 = 0x0A;

constexpr std::uint8_t kPayloadLength = 3;

enum class WireCommand : std::uint8_t
{
    Open = 0x01,
    Grab = 0x02,
    Release = 0x03,
    Stop = 0x04
};

struct Frame
{
    std::uint16_t sequence{0};
    WireCommand command{WireCommand::Open};
};

enum class DecodeError
{
    None,
    InvalidHeader,
    InvalidLength,
    InvalidTail,
    ChecksumMismatch,
    UnknownCommand
};

class FrameCodec
{
public:
    static std::vector<std::uint8_t>
    encode(const Frame &frame);

    static bool decode(
        const std::vector<std::uint8_t> &bytes,
        Frame &outFrame,
        DecodeError &error);

    static std::string toHex(
        const std::vector<std::uint8_t> &bytes);

    static std::string commandToString(
        WireCommand command);

    static std::string errorToString(
        DecodeError error);

private:
    static std::uint8_t calculateChecksum(
        const std::vector<std::uint8_t> &bytes,
        std::size_t begin,
        std::size_t endExclusive);

    static bool isKnownCommand(
        std::uint8_t command);
};

class StreamParser
{
public:
    void append(
        const std::vector<std::uint8_t> &bytes);

    std::vector<Frame> extractFrames();

private:
    std::vector<std::uint8_t> buffer_;
};

} // namespace serialproto