#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SERVO_PERIOD_US       20000U
#define PCA9685_RESOLUTION     4096U
#define SERVO_MAX_ANGLE_DEG     180U

static bool servo_angle_to_count(
    uint16_t angle_deg,
    uint16_t min_pulse_us,
    uint16_t max_pulse_us,
    uint16_t *count_out)
{
    /*
     * TODO 1：检查参数。
     *
     * 以下情况返回false：
     *
     * 1. count_out == NULL
     * 2. angle_deg > 180
     * 3. min_pulse_us == 0
     * 4. min_pulse_us >= max_pulse_us
     * 5. max_pulse_us >= 20000
     *
     * 参数错误不能修改count_out。
     */
    if(count_out==NULL||angle_deg>SERVO_MAX_ANGLE_DEG||min_pulse_us==0U||min_pulse_us>=max_pulse_us||max_pulse_us>=SERVO_PERIOD_US)
    {
        return false;
    }

    /*
     * TODO 2：将角度映射为脉宽。
     *
     * pulse_us =
     *     min_pulse_us +
     *     angle_deg *
     *     (max_pulse_us - min_pulse_us) /
     *     180;
     *
     * 中间计算使用uint32_t。
     */
        uint32_t pulse_range_us =
        (uint32_t)max_pulse_us -
        (uint32_t)min_pulse_us;
    uint32_t pulse_us=(uint32_t)min_pulse_us+(uint32_t)angle_deg*(pulse_range_us)/SERVO_MAX_ANGLE_DEG;
    /*
     * TODO 3：将脉宽转换为PCA9685计数值。
     *
     * count =
     *     pulse_us * 4096 / 20000;
     */
   uint32_t count =
    pulse_us *
    PCA9685_RESOLUTION /
    SERVO_PERIOD_US;

    /*
     * TODO 4：
     *
     * 成功后才修改*count_out，
     * 并返回true。
     */
    *count_out=(uint16_t)count;
    return true;
}

static void test_typical_angles(void)
{
    uint16_t count = 0U;

    assert(servo_angle_to_count(
        0U,
        500U,
        2500U,
        &count
    ));
    assert(count == 102U);

    assert(servo_angle_to_count(
        90U,
        500U,
        2500U,
        &count
    ));
    assert(count == 307U);

    assert(servo_angle_to_count(
        180U,
        500U,
        2500U,
        &count
    ));
    assert(count == 512U);
}

static void test_invalid_arguments(void)
{
    uint16_t count = 0xA5A5U;

    assert(!servo_angle_to_count(
        90U,
        500U,
        2500U,
        NULL
    ));

    assert(!servo_angle_to_count(
        181U,
        500U,
        2500U,
        &count
    ));

    assert(!servo_angle_to_count(
        90U,
        0U,
        2500U,
        &count
    ));

    assert(!servo_angle_to_count(
        90U,
        2500U,
        500U,
        &count
    ));

    assert(!servo_angle_to_count(
        90U,
        500U,
        20000U,
        &count
    ));

    /*
     * 所有失败路径都不能修改输出。
     */
    assert(count == 0xA5A5U);
}

int main(void)
{
    test_typical_angles();
    test_invalid_arguments();

    printf(
        "Servo angle conversion tests passed\n"
    );

    return 0;
}