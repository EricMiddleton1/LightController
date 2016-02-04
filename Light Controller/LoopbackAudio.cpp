#include "LoopbackAudio.h"

LoopbackAudio::LoopbackAudio(Callback *cb, int deviceID, int blockCount, int bufferSize) : AudioDevice(cb, blockCount, bufferSize) {
	HRESULT hr;
	
	WAVEFORMATEX *waveFormat = nullptr;
	
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSTX_ALL, IID_IMMDeviceEnumerator, (void**)&devEnumerator);

	hr = devEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &renderDevice);

	if (FAILED(hr))
		throw new Exception("Unable to initialize default render device.");

	hr = renderDevice->Activate(IID_IAudioClient, CLSTX_ALL, NULL, (void**)&audioClient);

	if (FAILED(hr))
		throw new Exception("Unable to initialize audio client.");
	
	hr = audioClient->GetMixFormat(&waveFormat);

	if (FAILED(hr))
		throw new Exception("Unable to get waveformat.");

	sampleRate = waveFormat->nSamplesPerSec;

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferSize / (sampleRate * blockCount), 0, waveFormat, NULL);

	if (FAILED(hr))
		throw new Exception("Unable to initialize audio client.");

	hr = audioClient->GetBufferSize(&actualPacketSize);

	if (FAILED(hr))
		throw new Exception("Unable to get packet length.");

	hr = audioClient->GetService(IID_IAudioCaptureClient, (void**)&captureClient);

	if (FAILED(hr))
		throw new Exception("Unable to get get audio client service.");

	CoTaskMemFree(waveFormat);
}

bool LoopbackAudio::Start() {
	HRESULT hr;

	//hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StaticThread,
}

LoopbackAudio::~LoopbackAudio() {
	if (devEnumerator != nullptr)
		devEnumerator->Release();

	if (audioClient != nullptr)
		audioClient->Release();

	if (captureClient != nullptr)
		captureClient->Release();

	if (renderDevice != nullptr)
		renderDevice->Release();
}