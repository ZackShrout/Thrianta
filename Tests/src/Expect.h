#pragma once

#include <Core/Logger.h>
#include <Math/TMath.h>

/**
 * @brief Expects expected to be equal to actual.
 */
#define ExpectShouldBe(expected, actual)                                                                \
    if (actual != expected) {                                                                           \
        TERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                   \
    }

/**
 * @brief Expects expected to NOT be equal to actual.
 */
#define ExpectShouldNotBe(expected, actual)                                                                      \
    if (actual == expected) {                                                                                    \
        TERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                            \
    }

/**
 * @brief Expects expected to be actual given a tolerance of T_FLOAT_EPSILON.
 */
#define ExpectFloatToBe(expected, actual)                                                           \
    if (tabs(expected - actual) > 0.001f) {                                                         \
        TERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                               \
    }

/**
 * @brief Expects actual to be true.
 */
#define ExpectToBeTrue(actual)                                                         \
    if (actual != true) {                                                              \
        TERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
    }

/**
 * @brief Expects actual to be false.
 */
#define ExpectToBeFalse(actual)                                                        \
    if (actual != false) {                                                             \
        TERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
    }