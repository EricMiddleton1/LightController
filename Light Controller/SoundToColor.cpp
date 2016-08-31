#include "SoundToColor.h"
#include <iostream>


SoundToColor::SoundToColor(WindowManager *window, float x, float y, float width, float height, SpectrumAnalyzer *spectrum) :
				Drawable(window, x, y, width, height) {
	this->spectrum = spectrum;
	
	filterStrength = 0.;
	bassFreq = 150;
	trebbleFreq = 9600; //6800
	noiseFloor = 70; //70
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

Color SoundToColor::GetMatrixColor() const {
	return matrixColor;
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
		frequencyColors.push_back(Color::HSV(30.*i / bassIndex, 1.f, 1.f));
	}

	m = 210.f / (trebbleIndex - bassIndex);
	for (int i = bassIndex; i < trebbleIndex; i++) {
		frequencyColors.push_back(Color::HSV(30 + (i - bassIndex)*m, 1.f, 1.f));
	}

	for (int i = trebbleIndex; i < bins.size(); i++) {
		frequencyColors.push_back(Color::HSV(240.f, 1.f, 1.f));
	}
}

void SoundToColor::SpectrumCallback(std::vector<FrequencyBin> const& lFreq, std::vector<FrequencyBin> const& rFreq) {
	auto mono = spectrum->GetMonoSpectrum();

	RenderColor(leftColor, lFreq, 2);
	RenderColor(rightColor, rFreq, 2);
	RenderColor(monoColor, mono);
	centerColor = monoColor;

	RenderMatrixColor(matrixColor, mono);

	RenderBassColor(bassColor, mono);

	std::vector<double> graphValues;

	for (int i = 0; i < lFreq.size(); i++) {
		graphValues.push_back(mono[i].GetMagnitudeDB());
	}

	graph->UpdateGraph(graphValues);
}

void SoundToColor::RenderColor(Color &color, const std::vector<FrequencyBin>& bins, double multiplier) const {
	double r = 0, g = 0, b = 0, biggest, scale;
	int count = bins.size();
	static double limiter = 0., avg = 0;

	scale = 1. / (60 * count); //50

	double curAvg = AverageDB(bins);

	if (avg > curAvg) {
		avg -= min(0.2, avg - curAvg);
		//avg = curAvg;
	}
	else {
		avg = curAvg;
	}

	double dbMult = 20. * std::log10(multiplier);
	

	for (int i = 0; i < count && i < frequencyColors.size(); i++) {
		Color cFreq = frequencyColors[i];
		double db = bins[i].GetMagnitudeDB() + dbMult + noiseFloor - avg;

	
		if (bins[i].GetStartFrequency() <= bassFreq)
			db += bassBoost;
	

		if (db < 0)
			db = 0;
		
		db *= (0.6 + + 0.5*avg);

		r += db*cFreq.GetRed();
		g += db*cFreq.GetGreen();
		b += db*cFreq.GetBlue();
	}

	double avgFactor = 1.;// = (1 + avg / 2);

	r *= scale * avgFactor;
	g *= scale * avgFactor;
	b *= scale * avgFactor;

	biggest = max(r, max(g, b));

	if (biggest < limiter) {
		//limiter -= min(0.1, limiter - biggest);
		limiter = biggest;
	}
	else {
		limiter = biggest;
	}

	if (limiter > 255) {
		double scale = 255. / limiter;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	Color c = Color(r, g, b);
	float h = c.GetHue(), s = c.GetHSVSaturation(), v = c.GetValue();

	c = Color::HSV(h, max(0.5, s), v);

	//Filter the color
	color.Filter(c, 0.6);
	//color.SubFiler(c, 10);
}

void SoundToColor::RenderMatrixColor(Color &color, const std::vector<FrequencyBin>& bins, double multiplier) const {
	double r = 0, g = 0, b = 0, biggest, scale;
	int count = bins.size();
	static double limiter = 0., avg = 0;

	scale = 1. / (50 * count); //50

	double curAvg = AverageDB(bins);

	if (avg > curAvg) {
		avg -= min(0.2, avg - curAvg);
	}
	else {
		avg = curAvg;
	}

	double dbMult = 20. * std::log10(multiplier);


	for (int i = 0; i < count && i < frequencyColors.size(); i++) {
		Color cFreq = frequencyColors[i];
		double db = bins[i].GetMagnitudeDB() + dbMult + noiseFloor - avg;
	
		if (bins[i].GetStartFrequency() <= bassFreq)
			db += bassBoost;
		

		if (db < 0)
			db = 0;

		db *= (1 + curAvg / 2);

		r += db*cFreq.GetRed();
		g += db*cFreq.GetGreen();
		b += db*cFreq.GetBlue();
	}

	r *= scale;
	g *= scale;
	b *= scale;

	biggest = max(r, max(g, b));

	if (biggest < limiter) {
		limiter -= min(1, limiter - biggest);
		//limiter = biggest;
	}
	else {
		limiter = biggest;
	}

	if (limiter > 255) {
		double scale = 255. / limiter;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	Color c = Color(r, g, b).gammaCorrected();
	float h = c.GetHue(), s = c.GetHSVSaturation(), v = c.GetValue();

	//kc = Color::HSV(h, max(0.5, s), v);

	//Filter the color
	color.Filter(c, 0.);
}

void SoundToColor::RenderBassColor(Color& color, const std::vector<FrequencyBin>& bins) const {
	double r = 0, g = 0, b = 0, biggest, scale;
	int count = bins.size();
	static double limiter = 0., avg = 0;

	scale = 1. / (55 * count); //50

	double curAvg = AverageDB(bins);

	if (avg > curAvg) {
		avg -= min(0.2, avg - curAvg);
		//avg = curAvg;
	}
	else {
		avg = curAvg;
	}

	double dbMult = 3.;


	for (int i = 0; i < count && i < frequencyColors.size() && bins[i].GetStartFrequency() < bassFreq; i++) {
		Color cFreq = frequencyColors[i];
		double db = bins[i].GetMagnitudeDB() + dbMult + noiseFloor - avg;

		db += bassBoost;

		if (db < 0)
			db = 0;

		db *= (0.6 + +0.5*avg);

		r += db*cFreq.GetRed();
		g += db*cFreq.GetGreen();
		b += db*cFreq.GetBlue();
	}

	double avgFactor = 1.;// = (1 + avg / 2);

	r *= scale * avgFactor;
	g *= scale * avgFactor;
	b *= scale * avgFactor;

	biggest = max(r, max(g, b));

	if (biggest < limiter) {
		//limiter -= min(0.1, limiter - biggest);
		limiter = biggest;
	}
	else {
		limiter = biggest;
	}

	if (limiter > 255) {
		double scale = 255. / limiter;

		r *= scale;
		g *= scale;
		b *= scale;
	}

	Color c = Color(r, g, b);
	float h = c.GetHue(), s = c.GetHSVSaturation(), v = c.GetValue();

	//c = Color::HSV(h, max(0.5, s), v);

	//Filter the color
	color.Filter(c, 0.6);
	//color.SubFiler(c, 10);
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

BarGraph* SoundToColor::getGraph() {
	return graph.get();
}