#pragma once

#include "digidaw/core/common.h"

#include "RtAudio.h"

namespace DigiDAW::Core::Audio 
{
	// TODO: More descriptive error codes
	enum class ReturnCode
	{
		Success = 0,
		Error = 1
	};
}
