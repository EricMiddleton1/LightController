#include "SoundToColor.h"
#include <iostream>

SoundToColor::SoundToColor(WindowManager *window, float x, float y, float width, float height, SpectrumAnalyzer *spectrum) :
				Drawable(window, x, y, width, height) {
	this->spectrum = spectrum;
	
	filterStrength = 0.;
	bassEnd = 200;
	noiseFloor = 50;
	bassBoost = 10.;

	auto cbLambda = [this](std::vector<FrequencyBin> const& l, std::vector<FrequencyBin> const& r) {
		this->SpectrumCallback(l, r);
	};

	spectrum->SetCallback(cbLambda);
	spectrumPrevState = spectrum->IsRunning();

	SetColors();

	graph.reset(new BarGraph(window, Color(0, 0, 0), x, y, width, height, -noiseFloor, 0));
	graph->SetColors(frequencyColors);

	if (!spectrumPrevState)
		spectrum->Start();
}

SoundToColor::~SoundToColor() {
	if (!spectrumPrevState)
		spectrum->Stop();
}

Color SoundToColor::GetLeftColor() const {
	return leftColor;
}

Color SoundToColor::GetRightColor() const {
	return rightColor;
}

Color SoundToColor::GetMonoColor() const {
	return monoColor;
}

Color SoundToColor::GetCenterColor() const {
	return centerColor;
}

Color SoundToColor::GetBassColor() const {
	return bassColor;
}

void SoundToColor::SetCallback(StCCallback cb) {
	this->cb = cb;
}

void SoundToColor::Draw(HDC hdc, int scrWidth, int scrHeight) {
	graph->Draw(hdc, scrWidth, scrHeight);
}

void SoundToColor::SetColors() {
	auto bins = spectrum->GetLeftSpectrum();
	int bassIndex = 0;
	double m, b;

	frequencyColors.clear();

	for (int i = 0; i < bins.size(); i++) {
		if (bins[i].GetEndFrequency() >= bassEnd) {
			bassIndex = i + 1;
			break;
		}
	}

	if (bassIndex == 0)
		bassIndex = bins.size();

	m = (bassIndex > 1) ? (30. / (bassIndex - 1)) : 0;
	b = 0.;
	for (int i = 0; i < bassIndex; i++) {
		float hue = m*i + b;

		frequencyColors.push_back(Color::HSL(hue, 1., 0.5));
	}

	m = ((bins.size() - bassIndex) > 0) ? (210. / (bins.size() - bassIndex)) : 0;
	b = 30.;
	for (int i = bassIndex; i < bins.size(); i++) {
		float hue = (i - bassIndex + 1)*m + b;

		frequencyColors.push_back(Color::HSL(hue, 1., 0.5));
	}
}

void SoundToColor::SpectrumCallback(std::vector<FrequencyBin> const& lFreq, std::vector<FrequencyBin> const& rFreq) {
	//static std::vector<double> noise;
	//static bool first = true;
	//static double avg = 0.;

	auto mono = spectrum->GetMonoSpectrum();
	//static float average = 1.;
	//float r = 0, g = 0, b = 0, level = 0.;
	//Color c;
	//int count = lFreq.size();
	////float max = -1000000;
	//double biggest, scale;

	//scale = 1. / (50 * mono.size());

	//avg = 0.5*avg + 0.5*AverageDB(mono);

	////std::cout << avg << std::endl;

	//if (first) {
	//	first = false;

	//	for (auto &bin : lFreq) {
	//		double db = bin.GetMagnitudeDB() + noiseFloor;
	//		noise.push_back((db > 0) ? db : 0);
	//	}
	//}

	//for (int i = 0; i < lFreq.size() && i < frequencyColors.size(); i++) {
	//	Color cFreq = frequencyColors[i];
	//	double db = mono[i].GetMagnitudeDB() + noiseFloor - avg;// -noise[i];

	//	if (mono[i].GetStartFrequency() <= bassEnd)
	//		db += bassBoost;

	//	if (db < 0)
	//		db = 0;

	//	db *= (1 + avg/2);

	//	r += db*cFreq.GetRed();
	//	g += db*cFreq.GetGreen();
	//	b += db*cFreq.GetBlue();
	//}

	//r *= scale;
	//g *= scale;
	//b *= scale;

	//biggest = max(r, max(g, b));

	//std::cout << biggest << "\n";

	//if (biggest > 255) {
	//	biggest /= 255;

	//	r /= biggest;
	//	g /= biggest;
	//	b /= biggest;
	//}

	//leftColor = Color(r, g, b);

	leftColor = RenderColor(lFreq);
	rightColor = RenderColor(rFreq);
	monoColor = RenderColor(mono);
	bassColor = RenderBassColor(mono);

	std::vector<double> graphValues;

	for (int i = 0; i < lFreq.size(); i++) {
		graphValues.push_back(mono[i].GetMagnitudeDB());// -noise[i]);
	}

	graph->UpdateGraph(graphValues);
}

Color SoundToColor::RenderColor(const std::vector<FrequencyBin>& bins) const {
	double r = 0, g = 0, b = 0, biggest, scale, avg;
	int count = bins.size();

	scale = 1. / (50 * count);

	avg = AverageDB(bins);

	for (int i = 0; i < count && i < frequencyColors.size(); i++) {
		Color cFreq = frequencyColors[i];
		double db = bins[i].GetMagnitudeDB() + noiseFloor - avg;

		if (bins[i].GetStartFrequency() <= bassEnd)
			db += bassBoost;

		if (db < 0)
			db = 0;

		db *= (1 + avg / 2);

		r += db*cFreq.GetRed();
		g += db*cFreq.GetGreen();
		b += db*cFreq.GetBlue();
	}

	r *= scale;
	g *= scale;
	b *= scale;

	biggest = max(r, max(g, b));

	if (biggest > 255) {
		biggest /= 255;

		r /= biggest;
		g /= biggest;
		b /= biggest;
	}

	return Color(r, g, b);
}

Color SoundToColor::RenderBassColor(const std::vector<FrequencyBin>& bins) const {
	double r = 0, g = 0, b = 0, biggest, scale, avg;
	int count = bins.size();

	scale = 1. / 50;// (50 * count);

	avg = AverageDB(bins);

	for (int i = 0; i < count && i < frequencyColors.size() && bins[i].GetStartFrequency() < 80; i++) {
		Color cFreq = frequencyColors[i];
		double db = bins[i].GetMagnitudeDB() + noiseFloor - avg;

		if (db < 0)
			db = 0;

		if (db > 0)
			db += bassBoost;

		db *= (1 + avg);

		r += db*cFreq.GetRed();
		g += db*cFreq.GetGreen();
		b += db*cFreq.GetBlue();
	}

	r *= scale;
	g *= scale;
	b *= scale;

	biggest = max(r, max(g, b));

	if (biggest > 255) {
		biggest /= 255;

		r /= biggest;
		g /= biggest;
		b /= biggest;
	}

	return Color(r, g, b);
}

double SoundToColor::AverageDB(const std::vector<FrequencyBin>& spectrum) const {
	double sum = 0.;

	for (auto &bin : spectrum) {
		double mag = bin.GetMagnitudeDB() + noiseFloor;
		
		if (mag > 0)
			sum += mag;
	}

	return sum / spectrum.size();
}