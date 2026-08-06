#include "serial_frame.h"

#include <iomanip>
#include <sstream>

namespace serialproto
{

std::uint8_t FrameCodec::calculateChecksum(
    const std::vector<std::uint8_t> &bytes,
    std::size_t begin,
    std::size_t endExclusive)
{
    std::uint8_t checksum = 0;

    for (std::size_t index = begin;
         index < endExclusive;
         ++index)
    {
        checksum ^= bytes[index];
    }

    return checksum;
}

bool FrameCodec::isKnownCommand(
    std::uint8_t command)
{
    return command ==
               static_cast<std::uint8_t>(
                   WireCommand::Open) ||
           command ==
               static_cast<std::uint8_t>(
                   WireCommand::Grab) ||
           command ==
               static_cast<std::uint8_t>(
                   WireCommand::Release) ||
           command ==
               static_cast<std::uint8_t>(
                   WireCommand::Stop);
}

std::vector<std::uint8_t>
FrameCodec::encode(const Frame &frame)
{
    std::vector<std::uint8_t> bytes;

    bytes.push_back(kHeader1);
    bytes.push_back(kHeader2);

    bytes.push_back(kPayloadLength);

    bytes.push_back(
        static_cast<std::uint8_t>(
            (frame.sequence >> 8) & 0xFF));

    bytes.push_back(
        static_cast<std::uint8_t>(
            frame.sequence & 0xFF));

    bytes.push_back(
        static_cast<std::uint8_t>(
            frame.command));

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

bool FrameCodec::decode(
    const std::vector<std::uint8_t> &bytes,
    Frame &outFrame,
    DecodeError &error)
{
    error = DecodeError::None;

    if (bytes.size() < 3)
    {
        error = DecodeError::InvalidLength;
        return false;
    }

    if (bytes[0] != kHeader1 ||
        bytes[1] != kHeader2)
    {
        error = DecodeError::InvalidHeader;
        return false;
    }

    const std::uint8_t payloadLength =
        bytes[2];

    if (payloadLength != kPayloadLength)
    {
        error = DecodeError::InvalidLength;
        return false;
    }

    const std::size_t expectedSize =
        static_cast<std::size_t>(
            payloadLength) +
        6;

    if (bytes.size() != expectedSize)
    {
        error = DecodeError::InvalidLength;
        return false;
    }

    if (bytes[expectedSize - 2] != kTail1 ||
        bytes[expectedSize - 1] != kTail2)
    {
        error = DecodeError::InvalidTail;
        return false;
    }

    const std::size_t checksumIndex =
        3 + payloadLength;

    const std::uint8_t calculatedChecksum =
        calculateChecksum(
            bytes,
            2,
            checksumIndex);

    if (bytes[checksumIndex] !=
        calculatedChecksum)
    {
        error = DecodeError::ChecksumMismatch;
        return false;
    }

    const std::uint8_t commandByte =
        bytes[5];

    if (!isKnownCommand(commandByte))
    {
        error = DecodeError::UnknownCommand;
        return false;
    }

    outFrame.sequence =
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(
                 bytes[3])
             << 8) |
            bytes[4]);

    outFrame.command =
        static_cast<WireCommand>(
            commandByte);

    return true;
}

std::string FrameCodec::toHex(
    const std::vector<std::uint8_t> &bytes)
{
    std::ostringstream stream;

    stream << std::hex
           << std::uppercase
           << std::setfill('0');

    for (std::size_t index = 0;
         index < bytes.size();
         ++index)
    {
        stream
            << std::setw(2)
            << static_cast<int>(
                   bytes[index]);

        if (index + 1 < bytes.size())
        {
            stream << ' ';
        }
    }

    return stream.str();
}

std::string FrameCodec::commandToString(
    WireCommand command)
{
    switch (command)
    {
    case WireCommand::Open:
        return "OPEN";

    case WireCommand::Grab:
        return "GRAB";

    case WireCommand::Release:
        return "RELEASE";

    case WireCommand::Stop:
        return "STOP";

    default:
        return "UNKNOWN";
    }
}

std::string FrameCodec::errorToString(
    DecodeError error)
{
    switch (error)
    {
    case DecodeError::None:
        return "NONE";

    case DecodeError::InvalidHeader:
        return "INVALID_HEADER";

    case DecodeError::InvalidLength:
        return "INVALID_LENGTH";

    case DecodeError::InvalidTail:
        return "INVALID_TAIL";

    case DecodeError::ChecksumMismatch:
        return "CHECKSUM_MISMATCH";

    case DecodeError::UnknownCommand:
        return "UNKNOWN_COMMAND";

    default:
        return "UNKNOWN_ERROR";
    }
}

void StreamParser::append(
    const std::vector<std::uint8_t> &bytes)
{
    buffer_.insert(
        buffer_.end(),
        bytes.begin(),
        bytes.end());
}

std::vector<Frame>
StreamParser::extractFrames()
{
    std::vector<Frame> frames;

    while (true)
    {
        if (buffer_.size() < 2)
        {
            break;
        }

        std::size_t headerPosition =
            buffer_.size();

        for (std::size_t index = 0;
             index + 1 < buffer_.size();
             ++index)
        {
            if (buffer_[index] == kHeader1 &&
                buffer_[index + 1] == kHeader2)
            {
                headerPosition = index;
                break;
            }
        }

        if (headerPosition == buffer_.size())
        {
            if (buffer_.back() == kHeader1)
            {
                const std::uint8_t lastByte =
                    buffer_.back();

                buffer_.clear();
                buffer_.push_back(lastByte);
            }
            else
            {
                buffer_.clear();
            }

            break;
        }

        if (headerPosition > 0)
        {
            buffer_.erase(
                buffer_.begin(),
                buffer_.begin() +
                    headerPosition);
        }

        if (buffer_.size() < 3)
        {
            break;
        }

        const std::uint8_t payloadLength =
            buffer_[2];

        if (payloadLength != kPayloadLength)
        {
            buffer_.erase(
                buffer_.begin());

            continue;
        }

        const std::size_t expectedSize =
            static_cast<std::size_t>(
                payloadLength) +
            6;

        if (buffer_.size() < expectedSize)
        {
            break;
        }

        const std::vector<std::uint8_t>
            candidate(
                buffer_.begin(),
                buffer_.begin() +
                    expectedSize);

        Frame frame;
        DecodeError error;

        if (FrameCodec::decode(
                candidate,
                frame,
                error))
        {
            frames.push_back(frame);

            buffer_.erase(
                buffer_.begin(),
                buffer_.begin() +
                    expectedSize);
        }
        else
        {
            /*
             * 当前帧非法，丢弃一个字节，
             * 重新寻找下一组 AA 55。
             */
            buffer_.erase(
                buffer_.begin());
        }
    }

    return frames;
}

} // namespace serialproto