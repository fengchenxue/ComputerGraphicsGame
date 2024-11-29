#pragma once
#include<iostream>
#include<Windows.h>
class Timer {
private:
	LARGE_INTEGER StartTime;
	LARGE_INTEGER LastTime;
	LARGE_INTEGER CurrentTime;
	LARGE_INTEGER Frequency;
public:
	Timer() {
		QueryPerformanceFrequency(&Frequency);
		QueryPerformanceCounter(&StartTime);
		QueryPerformanceCounter(&CurrentTime);
		QueryPerformanceCounter(&LastTime);

	}
	float dt() {
		QueryPerformanceCounter(&CurrentTime);
		float dt = static_cast<float>(CurrentTime.QuadPart - LastTime.QuadPart) / static_cast<float>(Frequency.QuadPart);
		LastTime = CurrentTime;
		return dt;
	}
	float time() {
		QueryPerformanceCounter(&CurrentTime);
		return static_cast<float>(CurrentTime.QuadPart - StartTime.QuadPart) / static_cast<float>(Frequency.QuadPart);
	}

};