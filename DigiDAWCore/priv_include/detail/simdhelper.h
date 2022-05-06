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
