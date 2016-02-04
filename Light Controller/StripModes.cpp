#include "StripModes.h"
#include <iostream>

StripModeSolid::StripModeSolid(int size, Color const& c) : StripMode(size) {
	strip->SetAll(c);
}

void StripModeSolid::SetColor(Color const& c) {
	strip->SetAll(c);
}

Color StripModeSolid::GetColor() {
	return strip->GetColor(0);
}

StripModeDissipate::StripModeDissipate(int size, int centerSize, double speed) : 
					StripMode(size) {
	this->centerSize = (centerSize > size) ? size : centerSize;
	this->speed = speed;
	
	lastTime = 0;
	distanceRemainder = 0.;

	QueryPerformanceFrequency((LARGE_INTEGER*)(&timerFrequency));
	std::cout << "Performance counter frequency: " << timerFrequency << std::endl;
}

StripModeDissipate::~StripModeDissipate() {

}

void StripModeDissipate::PushColor(Color const& c) {
	int middle = strip->GetSize() / 2;
	__int64 newTime;

	QueryPerformanceCounter((LARGE_INTEGER*)(&newTime));

	if (lastTime != 0) {
		Color lastColor = strip->GetColor(middle);
		double dt = ((double)newTime - lastTime) / timerFrequency;
		double distance = GetMoveDistance(dt, speed) + distanceRemainder;

		int moveCount = distance;
		int max = (middle - centerSize / 2) / 2;

		if (moveCount >= max) {
			strip->SetAll(lastColor);
			moveCount = max;
		}
		else {
			int downStart = middle - centerSize / 2;
			int upStart = downStart + centerSize;

			strip->ShiftUp(upStart, moveCount);
			strip->ShiftDown(downStart, moveCount);

			for (int i = 0; i < moveCount; i++)
				strip->Set(upStart + i, lastColor);

			for (int i = 0; i < moveCount; i++)
				strip->Set(downStart - i - 1, lastColor);
		}

		distanceRemainder = distance - moveCount;
	}

	for (int i = 0; i < centerSize; i++) {
		strip->Set(middle - centerSize / 2 + i, c);
	}

	lastTime = newTime;
}

void StripModeDissipate::PushColors(Color const& up, Color const& down) {
	int middle = strip->GetSize() / 2;
	int downStart = middle - centerSize / 2;
	int upStart = downStart + centerSize;
	__int64 newTime;

	QueryPerformanceCounter((LARGE_INTEGER*)(&newTime));

	if (lastTime != 0) {
		double dt = ((double)newTime - lastTime) / timerFrequency;
		double distance = GetMoveDistance(dt, speed) + distanceRemainder;

		int moveCount = distance;
		int max = (middle - centerSize / 2) / 2;

		if (moveCount >= max) {
			for (int i = 0; i < middle; i++)
				strip->Set(i, down);
			for (int i = middle; i < strip->GetSize(); i++)
				strip->Set(i, up);
			moveCount = max;
		}
		else {
			strip->ShiftUp(upStart, moveCount);
			strip->ShiftDown(downStart, moveCount);

			for (int i = 0; i < moveCount; i++)
				strip->Set(upStart + i, lastUp);

			for (int i = 0; i < moveCount; i++)
				strip->Set(downStart - i - 1, lastDown);
		}

		distanceRemainder = distance - moveCount;
	}

	for (int i = downStart; i < middle; i++)
		strip->Set(i, down);
	for (int i = middle; i < upStart; i++)
		strip->Set(i, up);

	lastTime = newTime;
	lastUp = up;
	lastDown = down;
}

double StripModeDissipate::GetMoveDistance(double dt, double speed) {
	return dt*speed;
}