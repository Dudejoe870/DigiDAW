#include "digidaw/core/detail/simdhelper.h"

#include <simdpp/simd.h>

namespace DigiDAW::Core::Detail
{
	void SimdHelper::CopyBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
	{
		if ((length & 0b111) != 0) return; // TODO: Handle non power of 8 copies.

		const size_t iterations = length / 8;
		for (size_t i = 0; i < iterations; ++i)
		{
			simdpp::float32x8 xmmA = simdpp::load(&src[(i * 8) + srcOffset]); // Load 8 floats from the source buffer into the vector.
			simdpp::store(&dst[(i * 8) + dstOffset], xmmA); // Store the 8 floats into the destination buffer.
		}
	}

	void SimdHelper::MulScalarBuffer(float scalar, float* buffer, size_t length)
	{
		if ((length & 0b111) != 0) return; // TODO: Handle non power of 8 multiplication.

		const size_t iterations = length / 8;
		for (size_t i = 0; i < iterations; ++i)
		{
			simdpp::float32x8 xmmA = simdpp::load(&buffer[i * 8]);
			simdpp::store(&buffer[i * 8], scalar * xmmA);
		}
	}

	void SimdHelper::MulScalarBufferStereo(float leftScalar, float rightScalar, float* buffer, size_t length, size_t leftOffset, size_t rightOffset)
	{
		if ((length & 0b111) != 0) return; // TODO: Handle non power of 8 multiplication.

		const size_t iterations = length / 8;
		for (size_t i = 0; i < iterations; ++i)
		{
			simdpp::float32x8 xmmA = simdpp::load(&buffer[(i * 8) + leftOffset]);
			simdpp::float32x8 xmmB = simdpp::load(&buffer[(i * 8) + rightOffset]);
			simdpp::store(&buffer[(i * 8) + leftOffset], leftScalar * xmmA);
			simdpp::store(&buffer[(i * 8) + rightOffset], rightScalar * xmmB);
		}
	}

	void SimdHelper::AddBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
	{
		if ((length & 0b111) != 0) return; // TODO: Handle non power of 8 addition.

		const size_t iterations = length / 8;
		for (size_t i = 0; i < iterations; ++i)
		{
			simdpp::float32x8 xmmA = simdpp::load(&src[(i * 8) + srcOffset]);
			simdpp::float32x8 xmmB = simdpp::load(&dst[(i * 8) + dstOffset]);
			simdpp::store(&dst[(i * 8) + dstOffset], xmmA + xmmB);
		}
	}
}
