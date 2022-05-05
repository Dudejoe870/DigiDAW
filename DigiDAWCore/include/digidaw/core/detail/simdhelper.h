#pragma once

#include "digidaw/core/common.h"

namespace DigiDAW::Core::Detail::SimdHelper
{
	void CopyBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length);

	void MulScalarBuffer(float scalar, float* buffer, size_t length);
	void MulScalarBufferStereo(float leftScalar, float rightScalar, float* buffer, size_t length, size_t leftOffset, size_t rightOffset);

	void AddBuffer(float* src, float* dst, size_t srcOffset, size_t dstOffset, size_t length);
}
