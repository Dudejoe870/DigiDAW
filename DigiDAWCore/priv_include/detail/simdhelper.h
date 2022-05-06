#pragma once

#include "digidaw/core/common.h"

#include <simdpp/simd.h>

namespace DigiDAW::Core::Detail
{
	/*
	 * A helper class to help with the implementation of optimized Audio-related SIMD functions.
	 */
	class SimdHelper
	{
	private:
		const float ln10 = 2.30258509299f;

		// Based off https://github.com/jhjourdan/SIMD-math-prims/blob/master/simd_math_prims.h
		template<unsigned N, class V>
		static void LnVector(simdpp::any_float32<N, V> src, simdpp::any_float32<N, V> dst)
		{
			simdpp::int32<N> iSrc = simdpp::bit_cast<simdpp::int32<N>>(src);

			// exp = src >> 23;
			simdpp::float32<N> exp = iSrc >> 23;

			// addcst = src > 0 ? -127 * log(2) + constant term of polynomial : -(float)INFINITY;
			simdpp::float32<N> addcst = simdpp::blend(-89.970756366f, -(float)INFINITY, src > 0);

			// src = (src & 0x7FFFFF) | 0x3F800000;
			iSrc = (iSrc & 0x7FFFFF) | 0x3F800000;
			// x = (float32)src;
			simdpp::float32<N> x = simdpp::bit_cast<simdpp::float32<N>>(iSrc);

			/* (From original scalar reference code)
			 *  Generated in Sollya using:
			 *	> f = remez(log(x)-(x-1)*log(2),
			 *			[|1,(x-1)*(x-2), (x-1)*(x-2)*x, (x-1)*(x-2)*x*x,
			 *			  (x-1)*(x-2)*x*x*x|], [1,2], 1, 1e-8);
			 *	> plot(f+(x-1)*log(2)-log(x), [1,2]);
			 *	> f+(x-1)*log(2)
			 */
			dst = x * (3.529304993f + x * (-2.461222105f + x * (1.130626167f +
					x * (-0.288739945f + x * 3.110401639e-2f))))
					+ (addcst + 0.6931471805f * exp);
		}

		template<unsigned N, class V>
		static void Log10Vector(simdpp::any_float32<N, V> src, simdpp::any_float32<N, V> dst)
		{
			LnVector(src, dst);
			dst /= ln10;
		}
	public:
		static void SetBuffer(float* dst, unsigned char value, size_t length, size_t offset)
		{
			simdpp::int8v xmmA = simdpp::splat(value); // Load "value" into all the places of the vector which is SIMDPP_FAST_INT8_SIZE bytes long.

			const size_t iterations = (length * sizeof(float)) / SIMDPP_FAST_INT8_SIZE;
			for (size_t i = 0; i < iterations; ++i)
				simdpp::store(&dst[(i * (SIMDPP_FAST_INT8_SIZE / sizeof(float))) + offset], xmmA); // Store the vector into the destination buffer.
		}

		static void CopyBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
		{
			const size_t iterations = length / SIMDPP_FAST_FLOAT32_SIZE;
			for (size_t i = 0; i < iterations; ++i)
			{
				simdpp::float32v xmmA = simdpp::load(&src[(i * SIMDPP_FAST_FLOAT32_SIZE) + srcOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the source buffer into the vector.
				simdpp::store(&dst[(i * SIMDPP_FAST_FLOAT32_SIZE) + dstOffset], xmmA); // Store the SIMDPP_FAST_FLOAT32_SIZE floats into the destination buffer.
			}
		}

		static void AddBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
		{
			const size_t iterations = length / SIMDPP_FAST_FLOAT32_SIZE;
			for (size_t i = 0; i < iterations; ++i)
			{
				simdpp::float32v xmmA = simdpp::load(&src[(i * SIMDPP_FAST_FLOAT32_SIZE) + srcOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the source buffer into the vector.
				simdpp::float32v xmmB = simdpp::load(&dst[(i * SIMDPP_FAST_FLOAT32_SIZE) + dstOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the destination buffer into the vector.
				simdpp::store(&dst[(i * SIMDPP_FAST_FLOAT32_SIZE) + dstOffset], xmmA + xmmB); // Store back into the destination buffer the sum of the two vectors.
			}
		}

		static void MulScalarBuffer(float scalar, float* buffer, size_t length, size_t offset)
		{
			const size_t iterations = length / SIMDPP_FAST_FLOAT32_SIZE;
			for (size_t i = 0; i < iterations; ++i)
			{
				simdpp::float32v xmmA = simdpp::load(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + offset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffer into the vector.
				simdpp::store(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + offset], scalar * xmmA); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the scalar back into the buffer.
			}
		}

		static void MulScalarBufferStereo(float leftScalar, float rightScalar, float* buffer, size_t length, size_t leftOffset, size_t rightOffset)
		{
			const size_t iterations = length / SIMDPP_FAST_FLOAT32_SIZE;
			for (size_t i = 0; i < iterations; ++i)
			{
				simdpp::float32v xmmA = simdpp::load(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + leftOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffers left channel into the vector.
				simdpp::float32v xmmB = simdpp::load(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + rightOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffers right channel into the vector.
				simdpp::store(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + leftOffset], leftScalar * xmmA); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the left scalar for the left channel back into the buffer.
				simdpp::store(&buffer[(i * SIMDPP_FAST_FLOAT32_SIZE) + rightOffset], rightScalar * xmmB); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the right scalar for the right channel back into the buffer.
			}
		}
	};
}
