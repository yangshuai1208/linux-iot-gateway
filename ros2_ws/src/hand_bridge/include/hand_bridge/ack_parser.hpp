#pragma once

#include <cstdint>
#include <string>

namespace hand_bridge
{

enum class AckStatus
{
    IN_PROGRESS,
    OK,
    PREEMPTED,
    BUSY,
    ERROR
};

bool parse_ack(
    const std::string &line,
    std::uint32_t &seq,
    AckStatus &status);

const char *ack_status_to_string(
    AckStatus status);

}