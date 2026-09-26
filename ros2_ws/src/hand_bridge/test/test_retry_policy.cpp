
#include <cassert>
#include <iostream>

#include "hand_bridge/retry_policy.hpp"

int main()
{
    using namespace hand_bridge;

    using Clock = RetryPolicy::Clock;

    const auto t0 = Clock::now();

    RetryPolicy policy;

    /* 测试1：注册请求 */
    assert(policy.start(
        101,
        "SEQ:101 CMD:HAND_GRAB\r\n",
        false));

    /* 重复注册相同SEQ应失败 */
    assert(!policy.start(
        101,
        "SEQ:101 CMD:HAND_RELEASE\r\n",
        false));

    /* 尚未真正发送，不应该发生超时重试 */
    assert(policy.poll(
        t0 + std::chrono::seconds(20)).empty());

    /* 实际发送成功 */
    assert(policy.mark_sent(101, t0));

    /* 1499ms时尚未超时 */
    assert(policy.poll(
        t0 + std::chrono::milliseconds(1499)).empty());

    /* 1500ms触发第一次重试 */
    auto retry1 = policy.poll(
        t0 + std::chrono::milliseconds(1500));

    assert(retry1.size() == 1);

    assert(retry1[0].seq == 101);

    assert(
        retry1[0].frame ==
        "SEQ:101 CMD:HAND_GRAB\r\n");

    /* 原SEQ重发成功后重新计时 */
    const auto t1 =
        t0 + std::chrono::milliseconds(1500);

    assert(policy.mark_sent(101, t1));

    /* 收到IN_PROGRESS */
    assert(policy.on_ack(
        101,
        AckStatus::IN_PROGRESS,
        t1 + std::chrono::milliseconds(100)));

    /* 不能按照原来的1500ms立即重发 */
    assert(policy.poll(
        t1 + std::chrono::seconds(2)).empty());

    /* 收到最终OK */
    assert(policy.on_ack(
        101,
        AckStatus::OK,
        t1 + std::chrono::seconds(3)));

    assert(
        policy.find(101)->state ==
        RequestState::SUCCEEDED);

    /* 已成功的请求不再重试 */
    assert(policy.poll(
        t1 + std::chrono::seconds(30)).empty());


    /* 测试2：重试耗尽 */
    assert(policy.start(
        102,
        "SEQ:102 CMD:HAND_OPEN\r\n",
        false));

    assert(policy.mark_sent(102, t0));

    auto retry2 = policy.poll(
        t0 + std::chrono::milliseconds(1500));

    assert(retry2.size() == 1);

    assert(policy.mark_sent(
        102,
        t0 + std::chrono::milliseconds(1500)));

    auto retry3 = policy.poll(
        t0 + std::chrono::milliseconds(3000));

    assert(retry3.size() == 1);

    assert(policy.mark_sent(
        102,
        t0 + std::chrono::milliseconds(3000)));

    /* 第三次等待超时：不再重试 */
    auto exhausted = policy.poll(
        t0 + std::chrono::milliseconds(4500));

    assert(exhausted.empty());

    assert(
        policy.find(102)->state ==
        RequestState::RESULT_UNKNOWN);


    /* 测试3：STOP取消旧命令的重试 */
    assert(policy.start(
        103,
        "SEQ:103 CMD:HAND_GRAB\r\n",
        false));

    assert(policy.mark_sent(103, t0));

    policy.cancel_non_stop();

    assert(
        policy.find(103)->state ==
        RequestState::SUPERSEDED);

    assert(policy.start(
        104,
        "SEQ:104 CMD:HAND_STOP\r\n",
        true));

    assert(policy.should_transmit(104));

    assert(!policy.should_transmit(103));

    std::cout
        << "retry_policy passed\n";

    return 0;
}
