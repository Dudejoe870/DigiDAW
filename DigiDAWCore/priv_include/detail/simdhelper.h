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
		// Based off https://github.com/jhjourdan/SIMD-math-prims/blob/master/simd_math_prims.h
		template<unsigned N, class V>
		static simdpp::float32<N, V> LnVector(const simdpp::float32<N, V>& src)
		{
			simdpp::int32<N> iSrc = simdpp::bit_cast<simdpp::int32<N>>(src);

			// exp = src >> 23;
			simdpp::float32<N> exp = simdpp::to_float32(iSrc >> 23);

			// addcst = src > 0 ? -127 * log(2) + constant term of polynomial : -(float)INFINITY;
			simdpp::float32<N> neg127Log2Poly = simdpp::splat(-89.970756366f);
			simdpp::float32<N> negInf = simdpp::splat(-(float)INFINITY);
			simdpp::float32<N> addcst = simdpp::blend(neg127Log2Poly, negInf, src > 0);

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
			return x * (3.529304993f + x * (-2.461222105f + x * (1.130626167f +
					x * (-0.288739945f + x * 3.110401639e-2f))))
					+ (addcst + 0.6931471805f * exp);
		}

		template<unsigned N, class V>
		static simdpp::float32<N, V> Log10Vector(const simdpp::float32<N, V>& src)
		{
			const float ln10 = 2.30258509299f; // ln(10)
			const float rcpLn10 = 1.0f / ln10; // 1 / ln(10)
			return LnVector(src) * rcpLn10; // ln(src) / ln(10)
		}
	public:
		static void GetBufferRMSAndPeakMultiChannel(const std::vector<std::vector<float>>& src, size_t length, std::vector<float>& rmsOut, std::vector<float>& peakOut)
		{
			const float minimumDecibel = -300.0f;
			simdpp::float32v min = simdpp::splat(minimumDecibel);

			rmsOut.resize(src.size());
			peakOut.resize(src.size());

			for (unsigned int channel = 0; channel < src.size(); ++channel)
			{
				float peak = minimumDecibel;
				rmsOut[channel] = 0.0f;

				size_t i;
				for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
				{
					simdpp::float32v xmmA = simdpp::load(&src[channel][i]);
					xmmA = xmmA * xmmA; // Mean square
					xmmA = 10.0f * Log10Vector(xmmA);
					xmmA = simdpp::blend(xmmA, min, xmmA > min);
					rmsOut[channel] += simdpp::reduce_add(xmmA);

					simdpp::float32v xmmB = simdpp::splat(peak);
					simdpp::float32v xmmC = simdpp::blend(xmmA, xmmB, xmmA > xmmB);
					peak = simdpp::reduce_max(xmmC);
				}
				for (; i < length; ++i) // Calculate the remaining length using scalar code.
				{
					float a = src[channel][i];
					a *= a;
					a = std::max(10.0f * std::log10(a), minimumDecibel);
					rmsOut[channel] += a;

					peak = std::max(peak, a);
				}

				peakOut[channel] = peak;
			}
			for (float& avg : rmsOut)
				avg /= length;
		}

		static void SetBuffer(float* dst, float value, size_t length, size_t offset)
		{
			simdpp::float32v xmmA = simdpp::splat(value); // Load "value" into all the places of the vector.

			size_t i;
			for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
				simdpp::store(&dst[i + offset], xmmA); // Store the vector into the destination buffer.
			for (; i < length; ++i) // Set the remaining length using scalar code.
				dst[i + offset] = value;
		}

		static void CopyBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
		{
			size_t i;
			for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
			{
				simdpp::float32v xmmA = simdpp::load(&src[i + srcOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the source buffer into the vector.
				simdpp::store(&dst[i + dstOffset], xmmA); // Store the SIMDPP_FAST_FLOAT32_SIZE floats into the destination buffer.
			}
			for (; i < length; ++i) // Copy the remaining length using scalar code.
				dst[i + dstOffset] = src[i + srcOffset];
		}

		static void AccumulateBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length)
		{
			size_t i;
			for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
			{
				simdpp::float32v xmmA = simdpp::load(&src[i + srcOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the source buffer into the vector.
				simdpp::float32v xmmB = simdpp::load(&dst[i + dstOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the destination buffer into the vector.
				simdpp::store(&dst[i + dstOffset], xmmA + xmmB); // Store back into the destination buffer the sum of the two vectors.
			}
			for (; i < length; ++i) // Calculate the remaining length using scalar code.
				dst[i + dstOffset] += src[i + srcOffset];
		}

		static void MulScalarBuffer(float scalar, float* buffer, size_t length, size_t offset)
		{
			size_t i;
			for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
			{
				simdpp::float32v xmmA = simdpp::load(&buffer[i + offset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffer into the vector.
				simdpp::store(&buffer[i + offset], xmmA * scalar); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the scalar back into the buffer.
			}
			for (; i < length; ++i) // Calculate the remaining length using scalar code.
				buffer[i + offset] *= scalar;
		}

		static void MulScalarBufferStereo(float leftScalar, float rightScalar, float* buffer, size_t length, size_t leftOffset, size_t rightOffset)
		{
			size_t i;
			for (i = 0; i + SIMDPP_FAST_FLOAT32_SIZE <= length; i += SIMDPP_FAST_FLOAT32_SIZE)
			{
				simdpp::float32v xmmA = simdpp::load(&buffer[i + leftOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffers left channel into the vector.
				simdpp::float32v xmmB = simdpp::load(&buffer[i + rightOffset]); // Load SIMDPP_FAST_FLOAT32_SIZE floats from the buffers right channel into the vector.
				simdpp::store(&buffer[i + leftOffset], xmmA * leftScalar); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the left scalar for the left channel back into the buffer.
				simdpp::store(&buffer[i + rightOffset], xmmB * rightScalar); // Store each of the SIMDPP_FAST_FLOAT32_SIZE floats multiplied by the right scalar for the right channel back into the buffer.
			}
			for (; i < length; ++i) // Calculate the remaining length using scalar code.
			{
				buffer[i + leftOffset] *= leftScalar;
				buffer[i + rightOffset] *= rightScalar;
			}
		}
	};
}
