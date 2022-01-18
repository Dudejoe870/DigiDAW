#include "audio/mixer.h"

namespace DigiDAW::Audio
{
	Mixer::Mixer()
	{
		this->doTestTone = false;
		this->testToneStartTime = 0.0;
		this->currentTime = 0.0;
	}

	void Mixer::updateCurrentTime(double time)
	{
		this->currentTime = time;
	}

	void Mixer::mix(
		float* outputBuffer,
		float* inputBuffer, 
		double time, 
		unsigned int nFrames, 
		unsigned int nOutChannels, unsigned int nInChannels, unsigned int sampleRate)
	{
		currentTime = time;

		if (!doTestTone)
		{
		}
		else
		{
			std::vector<float> monoOutput;
			for (unsigned int frame = 0; frame < nFrames; ++frame)
			{
				double sampleTime = (time + ((double)frame / (double)sampleRate)) - testToneStartTime;
				monoOutput.push_back((float)((0.10 * (std::clamp(1.0 - sampleTime, 0.0, 1.0))) * std::sin(2 * pi * 440.0 * sampleTime)));
			}

			std::memcpy(&outputBuffer[0 * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
			if (nOutChannels > 1) std::memcpy(&outputBuffer[1 * nFrames], monoOutput.data(), sizeof(float) * monoOutput.size());
		}
	}

	void Mixer::startTestTone()
	{
		testToneStartTime = currentTime;
		doTestTone = true;
	}

	void Mixer::endTestTone()
	{
		testToneStartTime = 0.0;
		doTestTone = false;
	}
}
