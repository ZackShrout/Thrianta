#pragma once

#include "Defines.h"
#include "MathTypes.h"

#define T_PI 3.14159265358979323846f
#define T_PI_2 2.0f * T_PI
#define T_HALF_PI 0.5f * T_PI
#define T_QUARTER_PI 0.25f * T_PI
#define T_ONE_OVER_PI 1.0f / T_PI
#define T_ONE_OVER_TWO_PI 1.0f / T_PI_2
#define T_SQRT_TWO 1.41421356237309504880f
#define T_SQRT_THREE 1.73205080756887729352f
#define T_SQRT_ONE_OVER_TWO 0.70710678118654752440f
#define T_SQRT_ONE_OVER_THREE 0.57735026918962576450f
#define T_DEG2RAD_MULTIPLIER T_PI / 180.0f
#define T_RAD2DEG_MULTIPLIER 180.0f / T_PI

// The multiplier to convert seconds to milliseconds.
#define T_SEC_TO_MS_MULTIPLIER 1000.0f

// The multiplier to convert milliseconds to seconds.
#define T_MS_TO_SEC_MULTIPLIER 0.001f

// A huge number that should be larger than any valid number used.
#define T_INFINITY 1e30f

// Smallest positive number where 1.0 + FLOAT_EPSILON != 0
#define T_FLOAT_EPSILON 1.192092896e-07f

// ------------------------------------------
// General math functions
// ------------------------------------------
TAPI f32 tsin(f32 x);
TAPI f32 tcos(f32 x);
TAPI f32 ttan(f32 x);
TAPI f32 tacos(f32 x);
TAPI f32 tsqrt(f32 x);
TAPI f32 tabs(f32 x);

/**
 * Indicates if the value is a power of 2. 0 is considered _not_ a power of 2.
 * @param value The value to be interpreted.
 * @returns True if a power of 2, otherwise false.
 */
TINLINE b8 is_power_of_2(u64 value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}

TAPI s32 trandom();
TAPI s32 trandom_in_range(s32 min, s32 max);

TAPI f32 ftrandom();
TAPI f32 ftrandom_in_range(f32 min, f32 max);

// ------------------------------------------
// Vector 2
// ------------------------------------------

/**
 * @brief Creates and returns a new 2-element vector using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @return A new 2-element vector.
 */
TINLINE vec2 vec2_create(f32 x, f32 y) {
    vec2 out_vector;
    out_vector.x = x;
    out_vector.y = y;
    return out_vector;
}

/**
 * @brief Creates and returns a 2-component vector with all components set to 0.0f.
 */
TINLINE vec2 vec2_zero() {
    return (vec2){0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 2-component vector with all components set to 1.0f.
 */
TINLINE vec2 vec2_one() {
    return (vec2){1.0f, 1.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing up (0, 1).
 */
TINLINE vec2 vec2_up() {
    return (vec2){0.0f, 1.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing down (0, -1).
 */
TINLINE vec2 vec2_down() {
    return (vec2){0.0f, -1.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing left (-1, 0).
 */
TINLINE vec2 vec2_left() {
    return (vec2){-1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 2-component vector pointing right (1, 0).
 */
TINLINE vec2 vec2_right() {
    return (vec2){1.0f, 0.0f};
}

/**
 * @brief Adds vector_1 to vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec2 vec2_add(vec2 vector_0, vec2 vector_1) {
    return (vec2){
        vector_0.x + vector_1.x,
        vector_0.y + vector_1.y};
}

/**
 * @brief Subtracts vector_1 from vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec2 vec2_sub(vec2 vector_0, vec2 vector_1) {
    return (vec2){
        vector_0.x - vector_1.x,
        vector_0.y - vector_1.y};
}

/**
 * @brief Multiplies vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec2 vec2_mul(vec2 vector_0, vec2 vector_1) {
    return (vec2){
        vector_0.x * vector_1.x,
        vector_0.y * vector_1.y};
}

/**
 * @brief Divides vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec2 vec2_div(vec2 vector_0, vec2 vector_1) {
    return (vec2){
        vector_0.x / vector_1.x,
        vector_0.y / vector_1.y};
}

/**
 * @brief Returns the squared length of the provided vector.
 * 
 * @param vector The vector to retrieve the squared length of.
 * @return The squared length.
 */
TINLINE f32 vec2_length_squared(vec2 vector) {
    return vector.x * vector.x + vector.y * vector.y;
}

/**
 * @brief Returns the length of the provided vector.
 * 
 * @param vector The vector to retrieve the length of.
 * @return The length.
 */
TINLINE f32 vec2_length(vec2 vector) {
    return tsqrt(vec2_length_squared(vector));
}

/**
 * @brief Normalizes the provided vector in place to a unit vector.
 * 
 * @param vector A pointer to the vector to be normalized.
 */
TINLINE void vec2_normalize(vec2* vector) {
    const f32 length = vec2_length(*vector);
    vector->x /= length;
    vector->y /= length;
}

/**
 * @brief Returns a normalized copy of the supplied vector.
 * 
 * @param vector The vector to be normalized.
 * @return A normalized copy of the supplied vector 
 */
TINLINE vec2 vec2_normalized(vec2 vector) {
    vec2_normalize(&vector);
    return vector;
}

/**
 * @brief Compares all elements of vector_0 and vector_1 and ensures the difference
 * is less than tolerance.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @param tolerance The difference tolerance. Typically K_FLOAT_EPSILON or similar.
 * @return True if within tolerance; otherwise false. 
 */
TINLINE b8 vec2_compare(vec2 vector_0, vec2 vector_1, f32 tolerance) {
    if (tabs(vector_0.x - vector_1.x) > tolerance) {
        return FALSE;
    }

    if (tabs(vector_0.y - vector_1.y) > tolerance) {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Returns the distance between vector_0 and vector_1.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The distance between vector_0 and vector_1.
 */
TINLINE f32 vec2_distance(vec2 vector_0, vec2 vector_1) {
    vec2 d = (vec2){
        vector_0.x - vector_1.x,
        vector_0.y - vector_1.y};
    return vec2_length(d);
}

// ------------------------------------------
// Vector 3
// ------------------------------------------

/**
 * @brief Creates and returns a new 3-element vector using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @param z The z value.
 * @return A new 3-element vector.
 */
TINLINE vec3 vec3_create(f32 x, f32 y, f32 z) {
    return (vec3){x, y, z};
}

/**
 * @brief Returns a new vec3 containing the x, y and z components of the 
 * supplied vec4, essentially dropping the w component.
 * 
 * @param vector The 4-component vector to extract from.
 * @return A new vec3 
 */
TINLINE vec3 vec3_from_vec4(vec4 vector) {
    return (vec3){vector.x, vector.y, vector.z};
}

/**
 * @brief Returns a new vec4 using vector as the x, y and z components and w for w.
 * 
 * @param vector The 3-component vector.
 * @param w The w component.
 * @return A new vec4 
 */
TINLINE vec4 vec3_to_vec4(vec3 vector, f32 w) {
    return (vec4){vector.x, vector.y, vector.z, w};
}

/**
 * @brief Creates and returns a 3-component vector with all components set to 0.0f.
 */
TINLINE vec3 vec3_zero() {
    return (vec3){0.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector with all components set to 1.0f.
 */
TINLINE vec3 vec3_one() {
    return (vec3){1.0f, 1.0f, 1.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing up (0, 1, 0).
 */
TINLINE vec3 vec3_up() {
    return (vec3){0.0f, 1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing down (0, -1, 0).
 */
TINLINE vec3 vec3_down() {
    return (vec3){0.0f, -1.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing left (-1, 0, 0).
 */
TINLINE vec3 vec3_left() {
    return (vec3){-1.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing right (1, 0, 0).
 */
TINLINE vec3 vec3_right() {
    return (vec3){1.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing forward (0, 0, -1).
 */
TINLINE vec3 vec3_forward() {
    return (vec3){0.0f, 0.0f, -1.0f};
}

/**
 * @brief Creates and returns a 3-component vector pointing backward (0, 0, 1).
 */
TINLINE vec3 vec3_back() {
    return (vec3){0.0f, 0.0f, 1.0f};
}

/**
 * @brief Adds vector_1 to vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec3 vec3_add(vec3 vector_0, vec3 vector_1) {
    return (vec3){
        vector_0.x + vector_1.x,
        vector_0.y + vector_1.y,
        vector_0.z + vector_1.z};
}

/**
 * @brief Subtracts vector_1 from vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec3 vec3_sub(vec3 vector_0, vec3 vector_1) {
    return (vec3){
        vector_0.x - vector_1.x,
        vector_0.y - vector_1.y,
        vector_0.z - vector_1.z};
}

/**
 * @brief Multiplies vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec3 vec3_mul(vec3 vector_0, vec3 vector_1) {
    return (vec3){
        vector_0.x * vector_1.x,
        vector_0.y * vector_1.y,
        vector_0.z * vector_1.z};
}

/**
 * @brief Multiplies all elements of vector_0 by scalar and returns a copy of the result.
 * 
 * @param vector_0 The vector to be multiplied.
 * @param scalar The scalar value.
 * @return A copy of the resulting vector.
 */
TINLINE vec3 vec3_mul_scalar(vec3 vector_0, f32 scalar) {
    return (vec3){
        vector_0.x * scalar,
        vector_0.y * scalar,
        vector_0.z * scalar};
}

/**
 * @brief Divides vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec3 vec3_div(vec3 vector_0, vec3 vector_1) {
    return (vec3){
        vector_0.x / vector_1.x,
        vector_0.y / vector_1.y,
        vector_0.z / vector_1.z};
}

/**
 * @brief Returns the squared length of the provided vector.
 * 
 * @param vector The vector to retrieve the squared length of.
 * @return The squared length.
 */
TINLINE f32 vec3_length_squared(vec3 vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

/**
 * @brief Returns the length of the provided vector.
 * 
 * @param vector The vector to retrieve the length of.
 * @return The length.
 */
TINLINE f32 vec3_length(vec3 vector) {
    return tsqrt(vec3_length_squared(vector));
}

/**
 * @brief Normalizes the provided vector in place to a unit vector.
 * 
 * @param vector A pointer to the vector to be normalized.
 */
TINLINE void vec3_normalize(vec3* vector) {
    const f32 length = vec3_length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

/**
 * @brief Returns a normalized copy of the supplied vector.
 * 
 * @param vector The vector to be normalized.
 * @return A normalized copy of the supplied vector 
 */
TINLINE vec3 vec3_normalized(vec3 vector) {
    vec3_normalize(&vector);
    return vector;
}

/**
 * @brief Returns the dot product between the provided vectors. Typically used
 * to calculate the difference in direction.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The dot product. 
 */
TINLINE f32 vec3_dot(vec3 vector_0, vec3 vector_1) {
    f32 p = 0;
    p += vector_0.x * vector_1.x;
    p += vector_0.y * vector_1.y;
    p += vector_0.z * vector_1.z;
    return p;
}

/**
 * @brief Calculates and returns the cross product of the supplied vectors.
 * The cross product is a new vector which is orthoganal to both provided vectors.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The cross product. 
 */
TINLINE vec3 vec3_cross(vec3 vector_0, vec3 vector_1) {
    return (vec3){
        vector_0.y * vector_1.z - vector_0.z * vector_1.y,
        vector_0.z * vector_1.x - vector_0.x * vector_1.z,
        vector_0.x * vector_1.y - vector_0.y * vector_1.x};
}

/**
 * @brief Compares all elements of vector_0 and vector_1 and ensures the difference
 * is less than tolerance.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @param tolerance The difference tolerance. Typically K_FLOAT_EPSILON or similar.
 * @return True if within tolerance; otherwise false. 
 */
TINLINE const b8 vec3_compare(vec3 vector_0, vec3 vector_1, f32 tolerance) {
    if (tabs(vector_0.x - vector_1.x) > tolerance) {
        return FALSE;
    }

    if (tabs(vector_0.y - vector_1.y) > tolerance) {
        return FALSE;
    }

    if (tabs(vector_0.z - vector_1.z) > tolerance) {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Returns the distance between vector_0 and vector_1.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The distance between vector_0 and vector_1.
 */
TINLINE f32 vec3_distance(vec3 vector_0, vec3 vector_1) {
    vec3 d = (vec3){
        vector_0.x - vector_1.x,
        vector_0.y - vector_1.y,
        vector_0.z - vector_1.z};
    return vec3_length(d);
}


// ------------------------------------------
// Vector 4
// ------------------------------------------

/**
 * @brief Creates and returns a new 4-element vector using the supplied values.
 * 
 * @param x The x value.
 * @param y The y value.
 * @param z The z value.
 * @param w The w value.
 * @return A new 4-element vector.
 */
TINLINE vec4 vec4_create(f32 x, f32 y, f32 z, f32 w) {
    vec4 out_vector;
#if defined(TUSE_SIMD)
    out_vector.data = _mm_setr_ps(x, y, z, w);
#else
    out_vector.x = x;
    out_vector.y = y;
    out_vector.z = z;
    out_vector.w = w;
#endif
    return out_vector;
}

/**
 * @brief Returns a new vec3 containing the x, y and z components of the 
 * supplied vec4, essentially dropping the w component.
 * 
 * @param vector The 4-component vector to extract from.
 * @return A new vec3 
 */
TINLINE vec3 vec4_to_vec3(vec4 vector) {
    return (vec3){vector.x, vector.y, vector.z};
}

/**
 * @brief Returns a new vec4 using vector as the x, y and z components and w for w.
 * 
 * @param vector The 3-component vector.
 * @param w The w component.
 * @return A new vec4 
 */
TINLINE vec4 vec4_from_vec3(vec3 vector, f32 w) {
#if defined(TUSE_SIMD)
    vec4 out_vector;
    out_vector.data = _mm_setr_ps(x, y, z, w);
    return out_vector;
#else
    return (vec4){vector.x, vector.y, vector.z, w};
#endif
}

/**
 * @brief Creates and returns a 3-component vector with all components set to 0.0f.
 */
TINLINE vec4 vec4_zero() {
    return (vec4){0.0f, 0.0f, 0.0f, 0.0f};
}

/**
 * @brief Creates and returns a 3-component vector with all components set to 1.0f.
 */
TINLINE vec4 vec4_one() {
    return (vec4){1.0f, 1.0f, 1.0f, 1.0f};
}

/**
 * @brief Adds vector_1 to vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec4 vec4_add(vec4 vector_0, vec4 vector_1) {
    vec4 result;
     for (u64 i = 0; i < 4; ++i) {
        result.elements[i] = vector_0.elements[i] + vector_1.elements[i];
    }
    return result;
}

/**
 * @brief Subtracts vector_1 from vector_0 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec4 vec4_sub(vec4 vector_0, vec4 vector_1) {
    vec4 result;
    for (u64 i = 0; i < 4; ++i) {
        result.elements[i] = vector_0.elements[i] - vector_1.elements[i];
    }
    return result;
}

/**
 * @brief Multiplies vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec4 vec4_mul(vec4 vector_0, vec4 vector_1) {
    vec4 result;
    for (u64 i = 0; i < 4; ++i) {
        result.elements[i] = vector_0.elements[i] * vector_1.elements[i];
    }
    return result;
}

/**
 * @brief Divides vector_0 by vector_1 and returns a copy of the result.
 * 
 * @param vector_0 The first vector.
 * @param vector_1 The second vector.
 * @return The resulting vector. 
 */
TINLINE vec4 vec4_div(vec4 vector_0, vec4 vector_1) {
    vec4 result;
    for (u64 i = 0; i < 4; ++i) {
        result.elements[i] = vector_0.elements[i] / vector_1.elements[i];
    }
    return result;
}

/**
 * @brief Returns the squared length of the provided vector.
 * 
 * @param vector The vector to retrieve the squared length of.
 * @return The squared length.
 */
TINLINE f32 vec4_length_squared(vec4 vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

/**
 * @brief Returns the length of the provided vector.
 * 
 * @param vector The vector to retrieve the length of.
 * @return The length.
 */
TINLINE f32 vec4_length(vec4 vector) {
    return tsqrt(vec4_length_squared(vector));
}

/**
 * @brief Normalizes the provided vector in place to a unit vector.
 * 
 * @param vector A pointer to the vector to be normalized.
 */
TINLINE void vec4_normalize(vec4* vector) {
    const f32 length = vec4_length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
    vector->w /= length;
}

/**
 * @brief Returns a normalized copy of the supplied vector.
 * 
 * @param vector The vector to be normalized.
 * @return A normalized copy of the supplied vector 
 */
TINLINE vec4 vec4_normalized(vec4 vector) {
    vec4_normalize(&vector);
    return vector;
}

TINLINE f32 vec4_dot_f32(
    f32 a0, f32 a1, f32 a2, f32 a3,
    f32 b0, f32 b1, f32 b2, f32 b3) {
    f32 p;
    p =
        a0 * b0 +
        a1 * b1 +
        a2 * b2 +
        a3 * b3;
    return p;
}