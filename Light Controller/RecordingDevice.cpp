#include "RecordingDevice.h"
#include <fstream>

RecordingDevice::WaveBuffer::WaveBuffer(int size) {
	samples = new short[2 * size];

	ZeroMemory(&wHeader, sizeof(wHeader));

	wHeader.lpData = (LPSTR)samples;
	wHeader.dwBufferLength = 2 * size * sizeof(short);
	wHeader.dwBytesRecorded = 0;
	wHeader.dwUser = 0;
	wHeader.dwFlags = 0;
	wHeader.dwLoops = 0;
	wHeader.lpNext = 0;
}

RecordingDevice::WaveBuffer::~WaveBuffer() {
	if (samples != nullptr)
		delete[] samples;
}

RecordingDevice::RecordingDevice(int blockCount, int bufferSize, int sampleRate) :
					AudioDevice(blockCount, bufferSize, sampleRate) {
	UINT retVal;
	DWORD threadID;

	for (int i = 0; i < blockCount; i++) {
		buffers.push_back(new WaveBuffer(bufferSize / blockCount));
	}

	WAVEFORMATEX wFormat;

	ZeroMemory(&wFormat, sizeof(wFormat));

	wFormat.wFormatTag = WAVE_FORMAT_PCM;
	wFormat.nChannels = 2;
	wFormat.nSamplesPerSec = sampleRate;
	wFormat.wBitsPerSample = 16;
	wFormat.nBlockAlign = wFormat.nChannels * wFormat.wBitsPerSample >> 3;
	wFormat.nAvgBytesPerSec = wFormat.nSamplesPerSec*wFormat.nBlockAlign;
	wFormat.cbSize = 0;

	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StaticThread, (void*)this, 0, &threadID);

	retVal = waveInOpen(&waveIn, WAVE_MAPPER, &wFormat, threadID, NULL, WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE | CALLBACK_THREAD);

	if (retVal != MMSYSERR_NOERROR)
		throw Exception("Unable to open the default recording device.");

	for (auto &buffer : buffers) {
		retVal = waveInPrepareHeader(waveIn, &buffer->wHeader, sizeof(WAVEHDR));
		if (retVal != MMSYSERR_NOERROR)
			throw Exception("Unable to prepare header.");

		retVal = waveInAddBuffer(waveIn, &buffer->wHeader, sizeof(WAVEHDR));
		if (retVal != MMSYSERR_NOERROR)
			throw Exception("Unable to add wave buffer.");

	}
}

RecordingDevice::~RecordingDevice() {
	for (auto& b : buffers)
		delete b;
}

bool RecordingDevice::Start() {
	UINT retVal = waveInStart(waveIn);

	return running = (retVal == MMSYSERR_NOERROR);
}

void RecordingDevice::Stop() {
	if (running)
		waveInStop(waveIn);
}

DWORD RecordingDevice::StaticThread(void *c) {
	RecordingDevice *that = static_cast<RecordingDevice*>(c);

	return that->Thread();
}

DWORD RecordingDevice::Thread() {
	MSG msg;
	WAVEHDR finishedHdr;

	//while (true) {
	while (GetMessage(&msg, 0, 0, 0) != 0) {
		//std::cout << msg.message << std::endl;
		switch (msg.message) {
		case MM_WIM_OPEN:
			break;

		case MM_WIM_CLOSE:
			//return 0;
			break;

		case MM_WIM_DATA: {
			WAVEHDR* finishedHdr = (WAVEHDR*)msg.lParam;
			int bytesRecorded = finishedHdr->dwBytesRecorded;

			ProcessBuffer((short*)finishedHdr->lpData, bytesRecorded);

			waveInAddBuffer(waveIn, finishedHdr, sizeof(WAVEHDR));

			if (leftSamples.size() >= bufferSize)
				cb(leftSamples, rightSamples);
			}
			break;
		}
	}

	std::cout << "Thread complete." << std::endl;

	return 0;
}

void RecordingDevice::ProcessBuffer(short* buffer, int byteCount) {
	int shortCount = byteCount >> 1;
	int overflow = (leftSamples.size() + (shortCount >> 1)) - bufferSize;

	if (overflow > 0) {
		//std::move(leftSamples.begin() + overflow, leftSamples.end(), leftSamples.begin());
		//std::move(rightSamples.begin() + overflow, rightSamples.end(), rightSamples.begin());
		std::copy(leftSamples.begin() + overflow, leftSamples.end(), leftSamples.begin());
		std::copy(rightSamples.begin() + overflow, rightSamples.end(), rightSamples.begin());

		leftSamples.resize(bufferSize - (shortCount >> 1));
		rightSamples.resize(bufferSize - (shortCount >> 1));
	}

	for (int i = 0; i < shortCount; i += 2) {
		leftSamples.push_back(buffer[i]);
		rightSamples.push_back(buffer[i + 1]);
	}
}

