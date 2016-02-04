#include "SoundToColor.h"
#include <iostream>

SoundToColor::SoundToColor(WindowManager *window, float x, float y, float width, float height, SpectrumAnalyzer *spectrum) :
				Drawable(window, x, y, width, height) {
	this->spectrum = spectrum;
	
	filterStrength = 0.;
	bassFreq = 150;
	trebbleFreq = 6800;
	noiseFloor = 70;
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
	int bassIndex = -1, trebbleIndex = -1;
	double m, b;

	frequencyColors.clear();

	//for (int i = 0; i < bins.size(); i++) {
	//	float hue = (3 * i / (bins.size())) * 360.f / 3.f;
	//	frequencyColors.push_back(Color::HSV(hue, 1., 1.));
	//}

	//return;

	for (int i = 0; i < bins.size(); i++) {
		if (bassIndex == -1 && bins[i].GetEndFrequency() >= bassFreq) {
			bassIndex = i + 1;
		}
		if (trebbleIndex == -1 && bins[i].GetEndFrequency() >= trebbleFreq) {
			trebbleIndex = i + 1;
			break;
		}
	}

	if (bassIndex < 0)
		bassIndex = 0;
	if (trebbleIndex <= bassIndex)
		trebbleIndex = bassIndex + 1;

	for (int i = 0; i < bassIndex; i++) {
		frequencyColors.push_back(Color::HSV(0.f, 1.f, 1.f));
	}

	m = 240.f / (trebbleIndex - bassIndex);
	for (int i = bassIndex; i < trebbleIndex; i++) {
		frequencyColors.push_back(Color::HSV((i - bassIndex)*m, 1.f, 1.f));
	}

	for (int i = trebbleIndex; i < bins.size(); i++) {
		frequencyColors.push_back(Color::HSV(240.f, 1.f, 1.f));
	}
}

void SoundToColor::SpectrumCallback(std::vector<FrequencyBin> const& lFreq, std::vector<FrequencyBin> const& rFreq) {
	auto mono = spectrum->GetMonoSpectrum();

	leftColor = RenderColor(lFreq);
	rightColor = RenderColor(rFreq);
	monoColor = RenderColor(mono);
	bassColor = RenderBassColor(mono);

	std::vector<double> graphValues;

	for (int i = 0; i < lFreq.size(); i++) {
		graphValues.push_back(mono[i].GetMagnitudeDB());
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

		if (bins[i].GetStartFrequency() <= bassFreq)
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

	Color c(r, g, b);
	float h = c.GetHue(), s = c.GetHSVSaturation(), v = c.GetValue();

	return Color::HSV(h, max(0.8, s), v);
}

Color SoundToColor::RenderBassColor(const std::vector<FrequencyBin>& bins) const {
	double r = 0, g = 0, b = 0, biggest, scale, avg;
	int count = bins.size();

	scale = 1. / 50;

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