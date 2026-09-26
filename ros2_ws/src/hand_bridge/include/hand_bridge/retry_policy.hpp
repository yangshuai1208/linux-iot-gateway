
#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "hand_bridge/ack_parser.hpp"

namespace hand_bridge
{

enum class RequestState
{
    QUEUED,
    WAITING_ACK,
    EXECUTING,
    RETRY_QUEUED,
    SUCCEEDED,
    PREEMPTED,
    BUSY,
    ERROR,
    SUPERSEDED,
    RESULT_UNKNOWN
};

struct RetryFrame
{
    std::uint32_t seq;
    std::string frame;
};

class RetryPolicy
{
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    static constexpr int MAX_RETRIES = 2;

    static constexpr auto ACK_TIMEOUT =
        std::chrono::milliseconds(1500);

    static constexpr auto EXEC_TIMEOUT =
        std::chrono::milliseconds(15000);

    struct Request
    {
        std::uint32_t seq = 0;
        std::string frame;
        bool is_stop = false;

        RequestState state =
            RequestState::QUEUED;

        int retries_used = 0;

        TimePoint deadline{};
    };

    bool start(
        std::uint32_t seq,
        const std::string &frame,
        bool is_stop)
    {
        if (seq == 0 || frame.empty())
        {
            return false;
        }

        Request request;

        request.seq = seq;
        request.frame = frame;
        request.is_stop = is_stop;

        auto result =
            requests_.emplace(seq, request);

        return result.second;
    }

    bool mark_sent(
        std::uint32_t seq,
        TimePoint now)
    {
        auto it = requests_.find(seq);

        if (it == requests_.end())
        {
            return false;
        }

        Request &request = it->second;

        if (request.state != RequestState::QUEUED &&
            request.state != RequestState::RETRY_QUEUED)
        {
            return false;
        }

        request.state =
            RequestState::WAITING_ACK;

        request.deadline =
            now + ACK_TIMEOUT;

        return true;
    }

    bool mark_send_failed(std::uint32_t seq)
    {
        auto it = requests_.find(seq);

        if (it == requests_.end())
        {
            return false;
        }

        Request &request = it->second;

        if (request.state != RequestState::QUEUED &&
            request.state != RequestState::RETRY_QUEUED)
        {
            return false;
        }

        request.state =
            RequestState::RESULT_UNKNOWN;

        return true;
    }

    bool on_ack(
        std::uint32_t seq,
        AckStatus status,
        TimePoint now)
    {
        auto it = requests_.find(seq);

        if (it == requests_.end())
        {
            return false;
        }

        Request &request = it->second;

        if (request.state == RequestState::SUPERSEDED)
        {
            if (status == AckStatus::PREEMPTED)
            {
                request.state =
                    RequestState::PREEMPTED;

                return true;
            }

            if (status == AckStatus::OK)
            {
                request.state =
                    RequestState::SUCCEEDED;

                return true;
            }

            if (status == AckStatus::ERROR)
            {
                request.state =
                    RequestState::ERROR;

                return true;
            }

            return false;
        }

        if (request.state != RequestState::WAITING_ACK &&
            request.state != RequestState::EXECUTING &&
            request.state != RequestState::RETRY_QUEUED)
        {
            return false;
        }

        switch (status)
        {
        case AckStatus::IN_PROGRESS:
            request.state =
                RequestState::EXECUTING;

            request.deadline =
                now + EXEC_TIMEOUT;

            break;

        case AckStatus::OK:
            request.state =
                RequestState::SUCCEEDED;

            break;

        case AckStatus::PREEMPTED:
            request.state =
                RequestState::PREEMPTED;

            break;

        case AckStatus::BUSY:
            request.state =
                RequestState::BUSY;

            break;

        case AckStatus::ERROR:
            request.state =
                RequestState::ERROR;

            break;
        }

        return true;
    }

    std::vector<RetryFrame> poll(TimePoint now)
    {
        std::vector<RetryFrame> output;

        for (auto &item : requests_)
        {
            Request &request = item.second;

            if (request.state != RequestState::WAITING_ACK &&
                request.state != RequestState::EXECUTING)
            {
                continue;
            }

            if (now < request.deadline)
            {
                continue;
            }

            if (request.retries_used >= MAX_RETRIES)
            {
                request.state =
                    RequestState::RESULT_UNKNOWN;

                continue;
            }

            ++request.retries_used;

            request.state =
                RequestState::RETRY_QUEUED;

            output.push_back(
                {request.seq, request.frame});
        }

        return output;
    }

    void cancel_non_stop()
    {
        for (auto &item : requests_)
        {
            Request &request = item.second;

            if (request.is_stop)
            {
                continue;
            }

            if (request.state == RequestState::QUEUED ||
                request.state == RequestState::WAITING_ACK ||
                request.state == RequestState::EXECUTING ||
                request.state == RequestState::RETRY_QUEUED)
            {
                request.state =
                    RequestState::SUPERSEDED;
            }
        }
    }

    const Request *find(std::uint32_t seq) const
    {
        auto it = requests_.find(seq);

        if (it == requests_.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    bool should_transmit(std::uint32_t seq) const
    {
        const Request *request = find(seq);

        if (request == nullptr)
        {
            return false;
        }

        return request->state == RequestState::QUEUED ||
               request->state == RequestState::RETRY_QUEUED;
    }

private:
    std::map<std::uint32_t, Request> requests_;
};

}
