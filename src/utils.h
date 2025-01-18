#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef size_t usize;
typedef intptr_t iptr;
typedef float f32;
typedef double f64;

typedef union v2 {
    struct {
        f32 x, y;
    };
    f32 e[2];
} v2;

v2 v2_add(v2 a, v2 b) { return (v2){ a.x + b.x, a.y + b.y }; }
v2 v2_div(v2 vector, float scalar) {
    return (scalar == 0.0f) ? (v2){0} : (v2){ vector.x / scalar, vector.y / scalar };
}
v2 v2_mult(v2 vector, float scalar) { return (v2){ vector.x * scalar, vector.y * scalar }; }
float v2_mag(v2 vector) { return sqrtf(vector.x * vector.x + vector.y * vector.y); }
v2 v2_normalized(v2 vector) { return v2_div(vector, v2_mag(vector)); }
