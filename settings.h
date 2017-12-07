#pragma once

// renderer 
#define OPTIMIZE 1
#define NUM_OF_THREADS 4
#define EPSILON  0.0001f
#define MAX_DEPTH 3
#define CAST_RAY_EVERY_FRAME 1

// SIMD
#define WITHOUT 0
#define SSE4 1
#define AVX 2

#define SIMD AVX


// camera
#define PRIMARY_SAMPLES 1 //currently not supported

// scene
#define SIMPLE_SCENE 1 // 0 loads simple obj file

#if SIMD == SSE4
#define VEC_SIZE 4
#define ALIGNMENT 16
typedef __m128 __mVec;
typedef __m128i __mVeci;
inline static __mVec _mVec_setr_ps(float a, float b, float c, float d, float e, float f, float g, float h) { return _mm_setr_ps(a, b, c, d); };
#else
#define VEC_SIZE 8
#define ALIGNMENT 32
typedef __m256 __mVec;
typedef __m256i __mVeci;

#define _mm_setzero_ps _mm256_setzero_ps
#define _mm_mul_ps _mm256_mul_ps
#define _mm_div_ps _mm256_div_ps
#define _mm_add_ps _mm256_add_ps
#define _mm_sub_ps _mm256_sub_ps
#define _mm_cmp_ps _mm256_cmp_ps
#define _mm_andnot_ps _mm256_andnot_ps
#define _mm_and_ps _mm256_and_ps
#define _mm_load_ps _mm256_load_ps
#define _mm_loadu_ps _mm256_loadu_ps
#define _mm_store_ps _mm256_store_ps
#define _mVec_storeu_ps _mm256_storeu_ps
#define _mm_setr_ps _mm256_setr_ps
#define _mm_set_ps1 _mm256_set1_ps
#define _mm_sqrt_ps _mm256_sqrt_ps
#define _mm_blendv_ps _mm256_blendv_ps
#define _mm_or_ps _mm256_or_ps
#define _mm_rcp_ps _mm256_rcp_ps
// Integer operators
#define _mm_set_epi32 _mm256_set_epi32
#define _mm_set1_epi32 _mm256_set1_epi32
#define _mm_store_si128 _mm256_store_si256
#define _mm_storeu_si128 _mm256_storeu_si256
#define _mm_cvtps_epi32 _mm256_cvtps_epi32
#define _mm_cvttps_epi32 _mm256_cvttps_epi32
#define _mm_add_epi32 _mm256_add_epi32
inline static __mVec _mVec_setr_ps(float a, float b, float c, float d, float e, float f, float g, float h) { return _mm256_setr_ps(a, b, c, d, e, f, g, h); };
#endif

const __mVec EPSVEC = _mm_set_ps1(EPSILON);
const __mVec MINUSEPSVEC = _mm_set_ps1(-EPSILON);
const __mVec TWOVEC = _mm_set_ps1(2.0f);
const __mVec ONEVEC = _mm_set_ps1(1.0f);
const __mVec MINUSONEVEC = _mm_set_ps1(-1.0f);
const __mVec ZEROVEC = _mm_set_ps1(0.0f);
const __mVeci MINUSONEIVEC = _mm_set1_epi32(-1);
