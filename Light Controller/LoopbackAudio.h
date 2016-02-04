#pragma once

#include "AudioDevice.h"
#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

class LoopbackAudio : public AudioDevice
{
public:
	LoopbackAudio(Callback *cb, int deviceID, int blockCount = 4, int bufferSize = 4092);
	~LoopbackAudio();

	virtual bool Start() override;
	virtual void Stop() override;


	static DWORD StaticThread(LPVOID context);
	DWORD Thread();
private:
	HANDLE hThread;
	IMMDevice *renderDevice;
	IMMDeviceEnumerator *devEnumerator = nullptr;
	IAudioClient *audioClient = nullptr;
	IAudioCaptureClient *captureClient = nullptr;

	unsigned int actualPacketSize;
};
