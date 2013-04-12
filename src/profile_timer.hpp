#pragma once

#ifdef _MSC_VER
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef linux
#include <sys/time.h>
#endif

#include <iostream>
#include <string>

namespace profile 
{
#ifdef _MSC_VER
	struct manager
	{
		LARGE_INTEGER frequency;
		LARGE_INTEGER t1, t2;
		double elapsedTime;
		std::string name;

		manager(const std::string& str) : name(str)
		{
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&t1);
		}

		~manager()
		{
			QueryPerformanceCounter(&t2);
			elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000000.0 / frequency.QuadPart;
			std::cerr << name << ":" << elapsedTime << std::endl;
		}
	};

	struct timer
	{
		LARGE_INTEGER frequency;
		LARGE_INTEGER t1;
		double elapsedTime;

		timer()
		{
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&t1);
		}

		double elapsed_time_microseconds()
		{
			LARGE_INTEGER t2;
			QueryPerformanceCounter(&t2);
			return elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000000.0 / frequency.QuadPart;
		}
	};

#elif defined(linux)
	struct manager
	{
		timeval t1, t2;
		double elapsedTime;
		std::string name;

		manager(const std::string& str) : name(str)
		{
			gettimeofday(&t1, NULL);
		}

		~manager()
		{
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
			elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
			elapsedTime *= 1000.0;
			std::cerr << name << ":" << elapsedTime << std::endl;
		}
	};
#endif
}
