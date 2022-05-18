#pragma once

#include "imgui.h"

#include <string>

namespace DigiDAW::UI
{
	class Window
	{
	public:
		bool open;

		Window(bool open)
		{
			this->open = open;
		}

		virtual void Render() = 0;

		virtual std::string GetName() = 0;
	};
}
