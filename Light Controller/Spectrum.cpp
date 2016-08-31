#include "Spectrum.h"
#include <fstream>
#include <iostream>

FrequencyBin::FrequencyBin(double fStart, double fEnd) {
	this->fStart = fStart;
	this->fEnd = fEnd;

	mag = 0.;
}

FrequencyBin::~FrequencyBin() {

}

double FrequencyBin::Add(double value) {
	return (mag += value);
}

double FrequencyBin::Remove(double value) {
	return (mag -= value);
}

double FrequencyBin::Set(double value) {
	return (mag = value);
}

double FrequencyBin::Clear() {
	return (mag = 0.);
}

double FrequencyBin::GetMagnitude() const {
	return mag;
}

double FrequencyBin::GetMagnitudeDB() const {
	if (mag == 0)
		return MIN_DB;
	
	double db = 20.*std::log10(mag);

	return (db > MIN_DB) ? db : MIN_DB;
}

double FrequencyBin::GetStartFrequency() const {
	return fStart;
}

double FrequencyBin::GetEndFrequency() const {
	return fEnd;
}

void FrequencyBin::operator=(const FrequencyBin& rValue) {
	fStart = rValue.GetStartFrequency();
	fEnd = rValue.GetEndFrequency();
	mag = rValue.GetMagnitude();
}

FrequencyBin FrequencyBin::operator/(const FrequencyBin& rValue) const {
	FrequencyBin result(*this);

	result.Set(mag / rValue.GetMagnitude());

	return result;
}

FrequencyBin FrequencyBin::operator/(const double& rValue) const {
	FrequencyBin result(*this);

	result.Set(mag / rValue);

	return result;
}

FrequencyBin FrequencyBin::operator+(const FrequencyBin& rValue) const {
	FrequencyBin result(*this);
	
	result.Add(rValue.GetMagnitude());

	return result;
}

SpectrumAnalyzer::SpectrumAnalyzer(AudioDevice *audioDevice, double fStart, double fEnd, int binsPerOctave) {
	auto cbLambda = [this](const std::vector<short>& leftSamples, const std::vector<short>& rightSamples) mutable -> void {
		this->AudioCallback(leftSamples, rightSamples);
	};
	
	this->input = audioDevice;

	audioDevice->SetCallback(cbLambda);

	int blockSize = audioDevice->GetBlockSize();

	inLCpx = inRCpx = outCpx = NULL;

	//ComputeHanningCoef(blockSize);
	ComputeBlackmanCoef(blockSize);

	inLCpx = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * blockSize);
	inRCpx = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * blockSize);
	outCpx = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * blockSize);
	fft = kiss_fft_alloc(blockSize, 0, 0, 0);

	int iMax = std::ceil(binsPerOctave * std::log2(fEnd / fStart));

	for (int i = 0; i < iMax; i++) {
		double f1 = fStart * std::pow(2., (double)i / binsPerOctave),
			f2 = fStart * std::pow(2., (double)(i + 1.) / binsPerOctave);

		lFreq.push_back(FrequencyBin(f1, f2));
		rFreq.push_back(FrequencyBin(f1, f2));

		std::cout << f1 << " - " << f2 << std::endl;
	}

	binCount = 0;
	for (int i = 0; i < input->GetBlockSize()/2; i++) {
		int bin = 0;
		double frequency = (double)i * input->GetSampleRate() / input->GetBlockSize();

		for (; bin < iMax; bin++) {
			if (frequency >= lFreq[bin].GetStartFrequency() && frequency <= lFreq[bin].GetEndFrequency())
				break;
		}
		if (bin == iMax)
			bin = -1;
		fftMap.push_back(bin);

		if (bin > binCount)
			binCount = bin;
	}

	binCount++;

	std::cout << "Bin count: " << binCount << std::endl;
}

SpectrumAnalyzer::~SpectrumAnalyzer() {

}

bool SpectrumAnalyzer::Start() {
	return input->Start();
}

void SpectrumAnalyzer::Stop() {
	input->Stop();
}

bool SpectrumAnalyzer::IsRunning() const {
	return input->IsRunning();
}

int SpectrumAnalyzer::GetBinCount() const {
	return binCount;
}

void SpectrumAnalyzer::SetCallback(SpectrumCallback cb) {
	this->cb = cb;
}

std::vector<FrequencyBin> SpectrumAnalyzer::GetLeftSpectrum() const {
	return lFreq;
}

std::vector<FrequencyBin> SpectrumAnalyzer::GetRightSpectrum() const {
	return rFreq;
}

std::vector<FrequencyBin> SpectrumAnalyzer::GetMonoSpectrum() const {
	std::vector<FrequencyBin> monoSpectrum;
	monoSpectrum.reserve(binCount);

	for (int i = 0; i < binCount; i++) {
		monoSpectrum.push_back(lFreq[i] + rFreq[i]);
	}

	return monoSpectrum;
}

template<typename T>
void DumpCSV(std::vector<T> values) {
	std::ofstream csv;
	csv.open("data.csv");

	csv << "Time, Value\n";

	for (int i = 0; i < values.size(); i++) {
		csv << i << ", " << values[i] << "\n";
	}

	csv.close();
}

void SpectrumAnalyzer::AudioCallback(const std::vector<short>& leftSamples, const std::vector<short>& rightSamples) {
	for (int i = 0; i < leftSamples.size(); i++) {
		inLCpx[i].r = hanning[i] * leftSamples[i] / 32768. / leftSamples.size();
		inRCpx[i].r = hanning[i] * rightSamples[i] / 32768. / rightSamples.size();

		inLCpx[i].i = inRCpx[i].i = 0.;
	}

	kiss_fft(fft, inLCpx, outCpx);

	if (lNoise.size() == 0) {
		for (int i = 0; i < leftSamples.size() / 2; i++) {
			kiss_fft_cpx val = outCpx[i];

			lNoise.push_back(val.i*val.i + val.r*val.r);
		}
	}

	ProcessSpectrum(lFreq, outCpx);

	kiss_fft(fft, inRCpx, outCpx);

	if (rNoise.size() == 0) {
		for (int i = 0; i < leftSamples.size() / 2; i++) {
			kiss_fft_cpx val = outCpx[i];

			rNoise.push_back(val.i*val.i + val.r*val.r);
		}
	}

	ProcessSpectrum(rFreq, outCpx);

	if (cb != nullptr)
		cb(lFreq, rFreq);
}

void SpectrumAnalyzer::ComputeHanningCoef(int size) {
	hanning.clear();

	for (int i = 0; i < size; i++)
		hanning.push_back(0.5f*(1.f - cos(2.f*PI*(float)i / (size - 1.f))));
}

void SpectrumAnalyzer::ComputeBlackmanCoef(int size) {
	hanning.clear();
	double a0 = 0.35875,
		a1 = 0.48829,
		a2 = 0.14128,
		a3 = 0.01168;

	for (int i = 0; i < size; i++) {
		double value = a0 - a1*std::cos(2 * PI*i / (size - 1)) 
			+ a2*std::cos(4 * PI*i / (size - 1)) - a3*cos(6 * PI*i / (size - 1));

		hanning.push_back(value);
	}
}

void SpectrumAnalyzer::ProcessSpectrum(std::vector<FrequencyBin> &old, kiss_fft_cpx *fft) {
	int sampleRate = input->GetSampleRate(), blockSize = input->GetBlockSize();

	for (auto &bin : old) {
		bin.Clear();
	}

	for (int i = 0; i < blockSize/2; i++) {
		kiss_fft_cpx value = fft[i];
		double frequency = ActualFrequency(sampleRate, blockSize, i, value),
			mag = std::sqrt((value.i * value.i) + (value.r * value.r));// -lNoise[i]);

		AddComponent(old, frequency, mag);
	}
}

double SpectrumAnalyzer::ActualFrequency(int sampleRate, int binCount, int binNumber, kiss_fft_cpx value) const {
	double frequency = sampleRate * (double)binNumber / binCount;

	return frequency;
}

void SpectrumAnalyzer::AddComponent(std::vector<FrequencyBin> &bins, double frequency, double mag) const {
	int fMax = input->GetSampleRate() / 2;

	if (frequency >= fMax)
		return;

	int index = frequency * fftMap.size() / fMax,
		bin = fftMap[index];

	if (bin >= 0)
		bins[bin].Add(mag);
}