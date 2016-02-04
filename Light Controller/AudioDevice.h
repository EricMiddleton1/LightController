#pragma once

#include <exception>
#include <string>
#include <vector>
#include <functional>

typedef std::function<void(std::vector<short> const&, std::vector<short> const&)> StereoCallback;

class AudioDevice
{
public:
	//class Callback {
		//public: virtual void AD_Callback(std::vector<short>& leftSamples, std::vector<short>& rightSamples) = 0;
	//};

	class Exception : std::exception {
	public:
		Exception(std::string message) { _msg = "Audio Device Exception: " + std::string(message); }

		virtual const char *what() { return _msg.c_str(); }
	private:
		std::string _msg;
	};

	AudioDevice(int blockCount, int bufferSize, int sampleRate) {
		this->blockCount = blockCount;
		this->bufferSize = bufferSize;
		this->sampleRate = sampleRate;
		running = false;
	}

	~AudioDevice() {
	}

	int GetSampleRate() {
		return sampleRate;
	}

	int GetBlockSize() {
		return bufferSize;
	}

	void SetCallback(StereoCallback cb) {
		this->cb = cb;
	}

	virtual bool Start() = 0;
	virtual void Stop() = 0;

	bool IsRunning() {
		return running;
	}

protected:
	StereoCallback cb;
	bool running;
	int bufferSize, blockCount;
	int sampleRate;
};