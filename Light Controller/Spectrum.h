#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "kiss_fft.h"
#include <cmath>
#include <functional>
#include <Windows.h>
#include "AudioDevice.h"

#pragma comment(lib, "winmm.lib")

#define PI		3.141592654

class FrequencyBin
{
public:
	FrequencyBin(double fStart, double fEnd);
	~FrequencyBin();

	double Add(double value);
	double Remove(double value);

	double Set(double value);
	double Clear();

	double GetMagnitude() const;
	double GetMagnitudeDB() const;

	double GetStartFrequency() const;
	double GetEndFrequency() const;

	void operator=(const FrequencyBin& rValue);
	FrequencyBin operator/(const FrequencyBin& rValue) const;
	FrequencyBin operator/(const double& rValue) const;
	FrequencyBin operator+(const FrequencyBin& rValue) const;
private:
	static const int MIN_DB = -1000.;
	double fStart, fEnd, mag;
};

typedef std::function<void(std::vector<FrequencyBin> const&, std::vector<FrequencyBin> const&)> SpectrumCallback;

class SpectrumAnalyzer {
public:
	SpectrumAnalyzer(AudioDevice *audioDevice, double fStart, double fEnd, int binsPerOctave);
	~SpectrumAnalyzer();

	bool Start();
	void Stop();

	bool IsRunning() const;

	int GetBinCount() const;

	void SetCallback(SpectrumCallback cb);

	std::vector<FrequencyBin> GetLeftSpectrum() const;
	std::vector<FrequencyBin> GetRightSpectrum() const;
	std::vector<FrequencyBin> GetMonoSpectrum() const;

	std::vector<FrequencyBin> GetLeftDerivative() const;
	std::vector<FrequencyBin> GetRightDerivative() const;
private:
	void AudioCallback(const std::vector<short>& leftSamples, const std::vector<short>& rightSamples);
	void ComputeHanningCoef(int size);
	void ComputeBlackmanCoef(int size);
	void ProcessSpectrum(std::vector<FrequencyBin> &old, kiss_fft_cpx *fft);
	double ActualFrequency(int sampleRate, int binCount, int binNumber, kiss_fft_cpx value) const;
	void AddComponent(std::vector<FrequencyBin> &bins, double frequency, double magnitude) const;

	AudioDevice *input;
	SpectrumCallback cb;

	kiss_fft_cfg fft;
	kiss_fft_cpx *inLCpx, *inRCpx, *outCpx;

	std::vector<FrequencyBin> lFreq, rFreq, lDeriv, rDeriv;
	std::vector<double> hanning, lNoise, rNoise;

	std::vector<int> fftMap;

	int octaveStart;
	int octaveEnd;
	int binCount;
};

#endif