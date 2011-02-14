#pragma once

#include <windows.h>

class NukyTimer
{
public:
	NukyTimer()
	{
		LARGE_INTEGER proc_freq;
		::QueryPerformanceFrequency(&proc_freq);
		frequency_ = static_cast<double>(proc_freq.QuadPart);

		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		::QueryPerformanceCounter(&start_);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
	}

	double elapsed()
	{
		LARGE_INTEGER now;
		DWORD_PTR oldmask = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
		::QueryPerformanceCounter(&now);
		::SetThreadAffinityMask(::GetCurrentThread(), oldmask);
		return ((now.QuadPart - start_.QuadPart) / frequency_);
	}

private:
	double frequency_;
	LARGE_INTEGER start_;
};
