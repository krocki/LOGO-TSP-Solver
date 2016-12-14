/*
 *   Logo TSP Solver ver. 0.62  Copyright (C) 2013  Kamil Rocki
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef USE_CPU_SIMD
#include <defs.h>
#include <headers.h>
#include <x86intrin.h>
#ifdef USE_MIC
#include <micvec.h>
#endif


#define USE_ALIGNED_LOADS_OPTIMIZATION

#ifdef USE_MIC

#define USE_512BIT_VECTORS

inline __m512
_m512_vec_loadu_ps (float const *p_p) {
    __m512          v = _mm512_setzero_ps();
    v = _mm512_loadunpacklo_ps (v, p_p);
    v = _mm512_loadunpackhi_ps (v, p_p + 16);
    return v;
}


#else
#ifdef USE_AVX
#define USE_256BIT_VECTORS
#else
#define USE_128BIT_VECTORS
#endif
#endif


// SSE/AVX/MIC code
#ifndef _CPUSIMD_
#define _CPUSIMD_

#ifdef __INTEL_COMPILER

#define PREFETCH_T0(addr,nrOfBytesAhead) _mm_prefetch(((char *)(addr))+nrOfBytesAhead,_MM_HINT_T0)
#define PREFETCH_T1(addr,nrOfBytesAhead) _mm_prefetch(((char *)(addr))+nrOfBytesAhead,_MM_HINT_T1)
#define PREFETCH_T2(addr,nrOfBytesAhead) _mm_prefetch(((char *)(addr))+nrOfBytesAhead,_MM_HINT_T2)
#define PREFETCH_NTA(addr,nrOfBytesAhead) _mm_prefetch(((char *)(addr))+nrOfBytesAhead,_MM_HINT_NTA)

#ifdef USE_128BIT_VECTORS
#define VEC4F __m128
#define SUB4F(x,y) _mm_sub_ps(x,y)
#define ADD4F(x,y) _mm_add_ps(x,y)
#define MUL4F(x,y) _mm_mul_ps(x,y)
#define SQRT4F(x) _mm_sqrt_ps(x)
#define FAST_SQRT4F(x) _mm_mul_ps(x,_mm_rsqrt_ps(x))
#define LOAD4F(x) _mm_load_ps(x)
#define BCAST4F(x) _mm_broadcast_ss(x)
#define INIT4F(e0,e1,e2,e3) \
			_mm_set_ps(e0, e1, e2, e3);

#endif

#ifdef USE_256BIT_VECTORS
#define VEC8F __m256
#define SUB8F(x,y) _mm256_sub_ps(x,y)
#define ADD8F(x,y) _mm256_add_ps(x,y)
#define MUL8F(x,y) _mm256_mul_ps(x,y)
#define SQRT8F(x) _mm256_sqrt_ps(x)
#define FAST_SQRT8F(x) _mm256_mul_ps(x,_mm256_rsqrt_ps(x))
#define LOAD8F(x) _mm256_load_ps(x)
#define BCAST8F(x) _mm256_broadcast_ss(x)
#define INIT8F(e0,e1,e2,e3,e4,e5,e6,e7) \
			_mm256_set_ps(e0, e1, e2, e3, e4, e5, e6, e7);

#endif

#ifdef USE_512BIT_VECTORS
#define VEC16F __m512
#define SUB16F(x,y) _mm512_sub_ps(x,y)
#define ADD16F(x,y) _mm512_add_ps(x,y)
#define MUL16F(x,y) _mm512_mul_ps(x,y)
#define MIN16F(x) _mm512_reduce_gmin_ps(x)
#define SQRT16F(x) _mm512_sqrt_ps(x)
#define FAST_SQRT16F(x) _mm512_mul_ps(x,_mm512_rsqrt23_ps(x))

#define LOAD_UNALIGNED(v, x) v = _mm512_loadunpacklo_ps(v,x); \
							 		 v = _mm512_loadunpackhi_ps(v,x+16);

#define LOADA16F(x) _mm512_load_ps(x)
#define LOAD16F(x) _m512_vec_loadu_ps(x)

#define FMAD16F(x, y, z) _mm512_fmadd_ps(x, y, z)
// #define LOAD16F(x) INIT16F(*x, *(x+1), *(x+2), *(x+3), *(x+4), *(x+5), *(x+6),
// *(x+7), *(x+8), *(x+9), *(x+10), *(x+11), *(x+12), *(x+13), *(x+14), *(x+15))
#define BCAST16F(x) _mm512_set1_ps(x)
#define INIT16F(e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15) \
			_mm512_set_ps(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15);

#endif

#else

#ifdef USE_128BIT_VECTORS
#define VEC4F __v4sf
#define SUB4F(x,y) __builtin_ia32_subps(x,y)
#define ADD4F(x,y) __builtin_ia32_addps(x,y)
#define MUL4F(x,y) __builtin_ia32_mulps(x,y)
#define SQRT4F(x) __builtin_ia32_sqrtps(x)
#define FAST_SQRT4F(x) __builtin_ia32_mulps(x,__builtin_ia32_rsqrtps(x))
#define LOAD4F(x) __builtin_ia32_loadups(x)
#define BCAST4F(x) __builtin_ia32_vbroadcastss(x)
#define INIT4F(e0,e1,e2,e3) \
					{e0, e1, e2, e3}

#endif

#ifdef USE_256BIT_VECTORS
#define VEC8F __v8sf
#define SUB8F(x,y) __builtin_ia32_subps256(x,y)
#define ADD8F(x,y) __builtin_ia32_addps256(x,y)
#define MUL8F(x,y) __builtin_ia32_mulps256(x,y)
#define SQRT8F(x) __builtin_ia32_sqrtps256(x)
#define FAST_SQRT8F(x) __builtin_ia32_mulps256(x,__builtin_ia32_rsqrtps256(x))
#define LOAD8F(x) __builtin_ia32_loadups256(x)
#define BCAST8F(x) __builtin_ia32_vbroadcastss256(x)
#define INIT8F(e0,e1,e2,e3,e4,e5,e6,e7) \
					{e0, e1, e2, e3, e4, e5, e6, e7}

#endif
#endif

#if defined(__INTEL_COMPILER) && defined(USE_512BIT_VECTORS)

#define VECF VEC16F
#define SUBF(x,y) SUB16F(x,y)
#define ADDF(x,y) ADD16F(x,y)
#define MULF(x,y) MUL16F(x,y)
#define FAST_SQRTF(x) FAST_SQRT16F(x)
#define NORMAL_SQRTF(x) SQRT16F(x)
#define LOADF(x) LOAD16F(x)
#define LOAD_CONST(c)  	BCAST16F(c)

#define VECTOR_LENGTH 16

#elif defined(USE_256BIT_VECTORS)

#define VECF VEC8F
#define SUBF(x,y) SUB8F(x,y)
#define ADDF(x,y) ADD8F(x,y)
#define MULF(x,y) MUL8F(x,y)
#define FAST_SQRTF(x) FAST_SQRT8F(x)
#define NORMAL_SQRTF(x) SQRT8F(x)
#define LOADF(x) LOAD8F(x)
#define BCASTF(x) BCAST8F(x)
#define LOAD_CONST(c)    BCAST8F(&c)

#define VECTOR_LENGTH 8

#else

#define VECF VEC4F
#define SUBF(x,y) SUB4F(x,y)
#define ADDF(x,y) ADD4F(x,y)
#define MULF(x,y) MUL4F(x,y)
#define FAST_SQRTF(x) FAST_SQRT4F(x)
#define NORMAL_SQRTF(x) SQRT4F(x)
#define LOADF(x) LOAD4F(x)
#define BCASTF(x) BCAST4F(x)

#define LOAD_CONST(c)  INIT4F(c, c, c, c)

#define VECTOR_LENGTH 4

#endif

#ifdef USE_FAST_SQRT
#define SQRTF(x) FAST_SQRTF(x)
#else
#define SQRTF(x) NORMAL_SQRTF(x)
#endif

#endif

typedef union OUTPUT_VECTOR {

    VECF            v;
    float           f[VECTOR_LENGTH];

} OUTPUT_VECTOR;

inline void
print_vector (OUTPUT_VECTOR o) {
    for (int i = 0; i < VECTOR_LENGTH; i++)
        printf ("%.1f, ", o.f[i]);

    printf ("\n");
}
#else
#define VECTOR_LENGTH 1
#define VECF float
#endif
