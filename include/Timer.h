#pragma once
#include <chrono>

namespace Gino
{

	class Timer
	{
	public:
		Timer();
		float TimeElapsed() const;

	private:
		std::chrono::system_clock::time_point m_start;

	};

}
