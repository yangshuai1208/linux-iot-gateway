#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    /*
     * 第一次重连的基础等待时间。
     */
    uint32_t base_delay_ms;

    /*
     * 最大等待时间。
     */
    uint32_t max_delay_ms;

    /*
     * 最大随机抖动时间。
     */
    uint32_t max_jitter_ms;

    /*
     * 当前连续失败次数。
     *
     * 0：当前连接正常
     * 1：第一次失败
     * 2：第二次连续失败
     */
    uint32_t failure_count;
} MqttRetryPolicy;

static bool mqtt_retry_init(
    MqttRetryPolicy *policy,
    uint32_t base_delay_ms,
    uint32_t max_delay_ms,
    uint32_t max_jitter_ms)
{
    /*
     * TODO 1：检查参数。
     *
     * 以下情况返回false：
     *
     * 1. policy == NULL
     * 2. base_delay_ms == 0
     * 3. max_delay_ms < base_delay_ms
     * 4. max_jitter_ms > max_delay_ms
     * 5. max_jitter_ms == UINT32_MAX
     *
     * 参数错误时不能修改policy。
     */
    if(policy==NULL||base_delay_ms == 0||max_delay_ms < base_delay_ms||max_jitter_ms > max_delay_ms|| max_jitter_ms == UINT32_MAX)
    {
        return false;
    }
    /*
     * TODO 2：
     *
     * 使用局部临时结构体完成初始化，
     * 成功后一次性赋值给*policy。
     *
     * failure_count初始化为0。
     */
  MqttRetryPolicy temp =
{
    base_delay_ms,
    max_delay_ms,
    max_jitter_ms,
    0U
};

*policy = temp;

return true;
  
}

static void mqtt_retry_record_failure(
    MqttRetryPolicy *policy)
{
    /*
     * TODO 3：
     *
     * 1. policy为NULL时直接返回。
     * 2. failure_count增加。
     * 3. 已经等于UINT32_MAX时保持不变，
     *    防止加一回绕为0。
     */

    if (policy == NULL)
    {
        return;
    }

    if (policy->failure_count < UINT32_MAX)
    {
    ++policy->failure_count;
    }


}

static void mqtt_retry_record_success(
    MqttRetryPolicy *policy)
{
    /*
     * TODO 4：
     *
     * 连接成功后把failure_count清零。
     * policy为NULL时直接返回。
     */
    if(policy==NULL)
    {
        return;
    }
  
  
        policy->failure_count=0;
  
  
}

static bool mqtt_retry_calculate_delay(
    const MqttRetryPolicy *policy,
    uint32_t random_value,
    uint32_t *delay_ms)
{
    /*
     * TODO 5：检查policy和delay_ms。
     *
     * 参数错误返回false，
     * 并且不能修改delay_ms。
     */
    if(policy==NULL||delay_ms==NULL)
    {
        return false;
    }
    /*
     * TODO 6：
     *
     * failure_count == 0时：
     *
     * 当前连接正常，不需要等待重连，
     * 将结果设置为0并返回true。
     */
    if (policy->failure_count == 0U)
    {
        *delay_ms = 0U;
        return true;
    }

    /*
     * TODO 7：计算指数退避基础时间。
     *
     * failure_count：
     *
     * 1 -> base_delay
     * 2 -> base_delay * 2
     * 3 -> base_delay * 4
     * 4 -> base_delay * 8
     *
     * 结果不得超过max_delay_ms。
     *
     * 必须防止乘2发生uint32_t溢出。
     *
     * 可以在乘2前判断：
     *
     * delay > max_delay_ms / 2U
     *
     * 如果成立，直接设置为max_delay_ms。
     */

 uint32_t base_delay =
    policy->base_delay_ms;

for (uint32_t step = 1U;
     step < policy->failure_count &&
     base_delay < policy->max_delay_ms;
     ++step)
{
    /*
     * 乘2之前检查是否会超过上限，
     * 同时防止uint32_t溢出。
     */
    if (base_delay >
        policy->max_delay_ms / 2U)
    {
        base_delay =
            policy->max_delay_ms;
        break;
    }

    base_delay *= 2U;
}

    /*
     * TODO 8：计算随机抖动。
     *
     * 抖动不能使最终延迟超过max_delay_ms。
     *
     * 可先计算剩余空间：
     *
     * remaining =
     *     max_delay_ms - base_delay;
     *
     * 实际抖动上限是：
     *
     * min(max_jitter_ms, remaining)
     *
     * jitter_limit不为0时：
     *
     * jitter =
     *     random_value % (jitter_limit + 1U);
     * 
     */
    uint32_t remaining =
    policy->max_delay_ms - base_delay;
    uint32_t jitter_limit =
    policy->max_jitter_ms;

    if (jitter_limit > remaining)
    {
    jitter_limit = remaining;
    }
    uint32_t jitter = 0U;

    if (jitter_limit != 0U)
    {
    jitter =
        random_value %
        (jitter_limit + 1U);
    }
    *delay_ms = base_delay + jitter;

    return true;




    /*
     * TODO 9：
     *
     * 最终延迟 =
     * 指数退避基础时间 + 抖动
     *
     * 成功时才修改delay_ms并返回true。
     */


}

static void test_initial_state(void)
{
    MqttRetryPolicy policy;
    uint32_t delay = 999U;

    assert(mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        500U
    ));

    assert(policy.base_delay_ms == 1000U);
    assert(policy.max_delay_ms == 8000U);
    assert(policy.max_jitter_ms == 500U);
    assert(policy.failure_count == 0U);

    assert(mqtt_retry_calculate_delay(
        &policy,
        123U,
        &delay
    ));

    /*
     * 尚未发生连接失败，不需要重连。
     */
    assert(delay == 0U);
}

static void test_exponential_backoff(void)
{
    MqttRetryPolicy policy;
    uint32_t delay = 0U;

    assert(mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        500U
    ));

    /*
     * 第一次失败：
     *
     * 基础值1000
     * 抖动123
     * 结果1123
     */
    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        123U,
        &delay
    ));

    assert(delay == 1123U);

    /*
     * 第二次连续失败：
     *
     * 基础值2000
     * 600 % 501 = 99
     * 结果2099
     */
    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        600U,
        &delay
    ));

    assert(delay == 2099U);

    /*
     * 第三次连续失败：
     *
     * 基础值4000
     * 抖动500
     * 结果4500
     */
    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        500U,
        &delay
    ));

    assert(delay == 4500U);
}

static void test_maximum_cap(void)
{
    MqttRetryPolicy policy;
    uint32_t delay = 0U;

    assert(mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        500U
    ));

    for (uint32_t count = 0U;
         count < 10U;
         ++count)
    {
        mqtt_retry_record_failure(&policy);
    }

    assert(mqtt_retry_calculate_delay(
        &policy,
        500U,
        &delay
    ));

    /*
     * 指数结果已经达到上限，
     * 不能再增加抖动。
     */
    assert(delay == 8000U);
}

static void test_success_resets_backoff(void)
{
    MqttRetryPolicy policy;
    uint32_t delay = 0U;

    assert(mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        500U
    ));

    mqtt_retry_record_failure(&policy);
    mqtt_retry_record_failure(&policy);
    mqtt_retry_record_failure(&policy);

    assert(policy.failure_count == 3U);

    mqtt_retry_record_success(&policy);

    assert(policy.failure_count == 0U);

    assert(mqtt_retry_calculate_delay(
        &policy,
        400U,
        &delay
    ));

    assert(delay == 0U);

    /*
     * 再次失败时应从基础延迟重新开始。
     */
    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        0U,
        &delay
    ));

    assert(delay == 1000U);
}

static void test_integer_overflow_protection(void)
{
    MqttRetryPolicy policy;
    uint32_t delay = 0U;

    assert(mqtt_retry_init(
        &policy,
        (UINT32_MAX / 2U) + 1U,
        UINT32_MAX,
        0U
    ));

    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        0U,
        &delay
    ));

    assert(delay ==
           (UINT32_MAX / 2U) + 1U);

    /*
     * 第二次失败如果直接乘2会溢出，
     * 正确结果应饱和到UINT32_MAX。
     */
    mqtt_retry_record_failure(&policy);

    assert(mqtt_retry_calculate_delay(
        &policy,
        0U,
        &delay
    ));

    assert(delay == UINT32_MAX);
}

static void test_failure_count_saturation(void)
{
    MqttRetryPolicy policy;

    assert(mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        100U
    ));

    policy.failure_count = UINT32_MAX;

    mqtt_retry_record_failure(&policy);

    assert(policy.failure_count ==
           UINT32_MAX);
}

static void test_invalid_arguments(void)
{
    MqttRetryPolicy policy =
    {
        111U,
        222U,
        33U,
        44U
    };

    uint32_t delay = 0xA5A5A5A5UL;

    assert(!mqtt_retry_init(
        NULL,
        1000U,
        8000U,
        500U
    ));

    assert(!mqtt_retry_init(
        &policy,
        0U,
        8000U,
        500U
    ));

    /*
     * 初始化失败不能修改原结构体。
     */
    assert(policy.base_delay_ms == 111U);
    assert(policy.failure_count == 44U);

    assert(!mqtt_retry_init(
        &policy,
        8000U,
        1000U,
        500U
    ));

    assert(!mqtt_retry_init(
        &policy,
        1000U,
        8000U,
        UINT32_MAX
    ));

    assert(!mqtt_retry_calculate_delay(
        NULL,
        100U,
        &delay
    ));

    /*
     * 参数错误不能修改输出。
     */
    assert(delay == 0xA5A5A5A5UL);

    assert(!mqtt_retry_calculate_delay(
        &policy,
        100U,
        NULL
    ));

    mqtt_retry_record_failure(NULL);
    mqtt_retry_record_success(NULL);
}

int main(void)
{
    test_initial_state();
    test_exponential_backoff();
    test_maximum_cap();
    test_success_resets_backoff();
    test_integer_overflow_protection();
    test_failure_count_saturation();
    test_invalid_arguments();

    printf("MQTT retry policy tests passed\n");
    return 0;
}