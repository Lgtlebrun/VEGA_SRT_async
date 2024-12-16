#include <unity.h>
#include <iostream>
#include "utils.cpp"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_isFloat_int()
{
    String toTest("3");
    TEST_ASSERT_TRUE(isFloat(toTest));
}

void test_isFloat_float()
{
    String toTest("3.1415");
    TEST_ASSERT_TRUE(isFloat(toTest));
}

void test_RADEC()
{
    float ra(30);
    float dec(30);
    float timestamp(1704063600);
    float az, el;
    float az_true(185.093283);
    float el_true(73.42930);
    std::tie(az, el) = Equatorial2AzAlt(ra, dec);
    std::cerr << "AZ : " << az << "EL : " << el << std::endl;
    TEST_ASSERT_TRUE(fabs(az - az_true) < 0.1);
    TEST_ASSERT_TRUE(fabs(el - el_true) < 0.1);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_isFloat_int);
    RUN_TEST(test_isFloat_float);
    RUN_TEST(test_RADEC);

    UNITY_END();
}