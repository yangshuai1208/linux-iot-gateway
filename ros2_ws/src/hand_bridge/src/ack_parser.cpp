
#include "hand_bridge/ack_parser.hpp"

#include <limits>

namespace hand_bridge
{

bool parse_ack(
    const std::string &line,
    std::uint32_t &seq,
    AckStatus &status)
{
    std::string text = line;


    if (!text.empty() && text.back() == '\n')
    {
        text.pop_back();

        if (!text.empty() && text.back() == '\r')
        {
            text.pop_back();
        }
    }


    if (text.compare(0, 4, "ACK:") != 0)
    {
        return false;
    }

    std::size_t pos = 4;

    std::uint32_t parsed_seq = 0;

    bool has_digit = false;

    const std::uint32_t max_seq =
        std::numeric_limits<std::uint32_t>::max();


    while (pos < text.size() &&
           text[pos] >= '0' &&
           text[pos] <= '9')
    {
        has_digit = true;

        std::uint32_t digit =
            static_cast<std::uint32_t>(
                text[pos] - '0');

        if (parsed_seq >
            (max_seq - digit) / 10U)
        {
            return false;
        }

        parsed_seq =
            parsed_seq * 10U + digit;

        ++pos;
    }

    if (!has_digit)
    {
        return false;
    }

    if (pos >= text.size() ||
        text[pos] != ' ')
    {
        return false;
    }

    ++pos;

    const std::string status_text =
        text.substr(pos);

    AckStatus parsed_status;

    if (status_text == "IN_PROGRESS")
    {
        parsed_status = AckStatus::IN_PROGRESS;
    }
    else if (status_text == "OK")
    {
        parsed_status = AckStatus::OK;
    }
    else if (status_text == "PREEMPTED")
    {
        parsed_status = AckStatus::PREEMPTED;
    }
    else if (status_text == "BUSY")
    {
        parsed_status = AckStatus::BUSY;
    }
    else if (status_text == "ERROR")
    {
        parsed_status = AckStatus::ERROR;
    }
    else
    {
        return false;
    }

 
    seq = parsed_seq;
    status = parsed_status;

    return true;
}

const char *ack_status_to_string(
    AckStatus status)
{
    switch (status)
    {
    case AckStatus::IN_PROGRESS:
        return "IN_PROGRESS";

    case AckStatus::OK:
        return "OK";

    case AckStatus::PREEMPTED:
        return "PREEMPTED";

    case AckStatus::BUSY:
        return "BUSY";

    case AckStatus::ERROR:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

} 
