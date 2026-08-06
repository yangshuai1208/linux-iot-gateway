#include "protocol_parser.h"
#include "serial_frame.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace
{

bool toWireCommand(
    Command command,
    serialproto::WireCommand &wireCommand)
{
    switch (command)
    {
    case Command::Open:
        wireCommand =
            serialproto::WireCommand::Open;
        return true;

    case Command::Grab:
        wireCommand =
            serialproto::WireCommand::Grab;
        return true;

    case Command::Release:
        wireCommand =
            serialproto::WireCommand::Release;
        return true;

    case Command::Stop:
        wireCommand =
            serialproto::WireCommand::Stop;
        return true;

    case Command::Unknown:
    default:
        return false;
    }
}

} // namespace

int main()
{
    ProtocolParser parser;

    const std::vector<std::string> inputs{
        "OPEN",
        "GRAB",
        "STOP",
        "RELEASE"};

    std::vector<
        std::vector<std::uint8_t>>
        encodedFrames;

    std::uint16_t sequence = 0;

    for (const std::string &input : inputs)
    {
        ++sequence;

        const Command command =
            parser.parse(input);

        serialproto::WireCommand
            wireCommand;

        if (!toWireCommand(
                command,
                wireCommand))
        {
            continue;
        }

        const serialproto::Frame frame{
            sequence,
            wireCommand};

        const auto bytes =
            serialproto::FrameCodec::encode(
                frame);

        encodedFrames.push_back(bytes);

        std::cout
            << "[ENCODE] seq="
            << sequence
            << " command="
            << input
            << " bytes="
            << serialproto::FrameCodec::
                   toHex(bytes)
            << '\n';

        serialproto::Frame decodedFrame;

        serialproto::DecodeError error;

        if (serialproto::FrameCodec::decode(
                bytes,
                decodedFrame,
                error))
        {
            std::cout
                << "[DECODE] seq="
                << decodedFrame.sequence
                << " command="
                << serialproto::FrameCodec::
                       commandToString(
                           decodedFrame.command)
                << '\n';
        }
    }

    /*
     * 校验错误实验。
     */
    if (!encodedFrames.empty())
    {
        auto corrupted =
            encodedFrames.front();

        corrupted[5] ^= 0x01;

        serialproto::Frame frame;
        serialproto::DecodeError error;

        const bool success =
            serialproto::FrameCodec::decode(
                corrupted,
                frame,
                error);

        std::cout
            << "[CHECKSUM_TEST] success="
            << std::boolalpha
            << success
            << " error="
            << serialproto::FrameCodec::
                   errorToString(error)
            << '\n';
    }

    /*
     * 模拟：
     * 噪声 + 拆包 + 两帧粘在一起。
     */
    if (encodedFrames.size() >= 2)
    {
        std::vector<std::uint8_t> stream{
            0x00,
            0x7E};

        stream.insert(
            stream.end(),
            encodedFrames[0].begin(),
            encodedFrames[0].end());

        stream.insert(
            stream.end(),
            encodedFrames[1].begin(),
            encodedFrames[1].end());

        const std::size_t splitPosition = 6;

        std::vector<std::uint8_t> chunk1(
            stream.begin(),
            stream.begin() +
                splitPosition);

        std::vector<std::uint8_t> chunk2(
            stream.begin() +
                splitPosition,
            stream.end());

        serialproto::StreamParser
            streamParser;

        streamParser.append(chunk1);

        const auto firstBatch =
            streamParser.extractFrames();

        std::cout
            << "[STREAM] first_chunk_frames="
            << firstBatch.size()
            << '\n';

        streamParser.append(chunk2);

        const auto secondBatch =
            streamParser.extractFrames();

        std::cout
            << "[STREAM] second_chunk_frames="
            << secondBatch.size()
            << '\n';

        for (const auto &frame :
             secondBatch)
        {
            std::cout
                << "[STREAM_FRAME] seq="
                << frame.sequence
                << " command="
                << serialproto::FrameCodec::
                       commandToString(
                           frame.command)
                << '\n';
        }
    }

    return 0;
}