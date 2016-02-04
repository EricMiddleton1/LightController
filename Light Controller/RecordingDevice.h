#pragma once

#include "AudioDevice.h"
#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#include <algorithm>
#include <iostream>

class RecordingDevice : public AudioDevice
{
public:
	RecordingDevice(int blockCount, int bufferSize, int sampleRate);
	~RecordingDevice();

	virtual bool Start();

	virtual void Stop();
private:
	static DWORD StaticThread(void *c);
	DWORD Thread();
	void ProcessBuffer(short *buffer, int byteCount);

	class WaveBuffer
	{
	public:
		WaveBuffer(int size);
		~WaveBuffer();

		WAVEHDR wHeader;
		short *samples = nullptr;
	};

	std::vector<WaveBuffer*> buffers;
	std::vector<short> leftSamples, rightSamples;
	HANDLE thread;
	HWAVEIN waveIn;
};