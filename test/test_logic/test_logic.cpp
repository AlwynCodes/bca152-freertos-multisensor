#include <unity.h>

#include "alarm.h"
#include "display.h"
#include "system_state.h"
#include "config.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_alarm_below_lower_limit(void)
{
    TEST_ASSERT_EQUAL(
        LOW_TEMPERATURE,
        evaluateTemperature(17.9f)
    );
}

void test_alarm_at_lower_limit(void)
{
    TEST_ASSERT_EQUAL(
        NORMAL,
        evaluateTemperature(18.0f)
    );
}

void test_alarm_normal_value(void)
{
    TEST_ASSERT_EQUAL(
        NORMAL,
        evaluateTemperature(25.0f)
    );
}

void test_alarm_at_upper_limit(void)
{
    TEST_ASSERT_EQUAL(
        NORMAL,
        evaluateTemperature(30.0f)
    );
}

void test_alarm_above_upper_limit(void)
{
    TEST_ASSERT_EQUAL(
        HIGH_TEMPERATURE,
        evaluateTemperature(30.1f)
    );
}

void test_navigation_forward(void)
{
    TEST_ASSERT_EQUAL(
        DISPLAY_HUMIDITY,
        nextDisplayMode(DISPLAY_TEMPERATURE)
    );
}

void test_navigation_forward_wraparound(void)
{
    TEST_ASSERT_EQUAL(
        DISPLAY_TEMPERATURE,
        nextDisplayMode(DISPLAY_MOTION)
    );
}

void test_navigation_reverse(void)
{
    TEST_ASSERT_EQUAL(
        DISPLAY_LIGHT,
        previousDisplayMode(DISPLAY_MOTION)
    );
}

void test_navigation_reverse_wraparound(void)
{
    TEST_ASSERT_EQUAL(
        DISPLAY_MOTION,
        previousDisplayMode(DISPLAY_TEMPERATURE)
    );
}

void test_state_active_without_timeout(void)
{
    TEST_ASSERT_EQUAL(
        SYSTEM_ACTIVE,
        evaluateSystemState(
            SYSTEM_ACTIVE,
            false,
            1000
        )
    );
}

void test_state_active_after_timeout(void)
{
    TEST_ASSERT_EQUAL(
        SYSTEM_INACTIVE,
        evaluateSystemState(
            SYSTEM_ACTIVE,
            false,
            INACTIVITY_TIMEOUT_MS
        )
    );
}

void test_state_inactive_without_motion(void)
{
    TEST_ASSERT_EQUAL(
        SYSTEM_INACTIVE,
        evaluateSystemState(
            SYSTEM_INACTIVE,
            false,
            1000
        )
    );
}

void test_state_inactive_with_motion(void)
{
    TEST_ASSERT_EQUAL(
        SYSTEM_ACTIVE,
        evaluateSystemState(
            SYSTEM_INACTIVE,
            true,
            0
        )
    );
}

extern "C" void app_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_alarm_below_lower_limit);
    RUN_TEST(test_alarm_at_lower_limit);
    RUN_TEST(test_alarm_normal_value);
    RUN_TEST(test_alarm_at_upper_limit);
    RUN_TEST(test_alarm_above_upper_limit);

    RUN_TEST(test_navigation_forward);
    RUN_TEST(test_navigation_forward_wraparound);
    RUN_TEST(test_navigation_reverse);
    RUN_TEST(test_navigation_reverse_wraparound);

    RUN_TEST(test_state_active_without_timeout);
    RUN_TEST(test_state_active_after_timeout);
    RUN_TEST(test_state_inactive_without_motion);
    RUN_TEST(test_state_inactive_with_motion);

    UNITY_END();
}
