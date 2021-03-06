#pragma once

#include "Spectrum.h"
#include "Color.h"
#include "Drawable.h"
#include "BarGraph.h"
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

typedef std::function<void(Color const&, Color const&)> StCCallback;

class SoundToColor : public Drawable
{
public:
	SoundToColor(WindowManager *window, float x, float y, float width, float height, SpectrumAnalyzer *spectrum);
	~SoundToColor();

	Color GetLeftColor() const;
	Color GetRightColor() const;
	Color GetMonoColor() const;
	Color GetCenterColor() const;
	Color GetBassColor() const;
	Color GetMatrixColor() const;

	void SetCallback(StCCallback cb);

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight) override;

	BarGraph* getGraph();

protected:
	void SpectrumCallback(std::vector<FrequencyBin> const& lFreq, std::vector<FrequencyBin> const& rFreq);
	
	void RenderColor(Color &color, const std::vector<FrequencyBin>& bins, double multiplier = 1.) const;
	void RenderMatrixColor(Color &color, const std::vector<FrequencyBin>& bins, double multiplier = 1.) const;
	void RenderBassColor(Color& color, const std::vector<FrequencyBin>& bins) const;
	void SetColors();
	double AverageDB(const std::vector<FrequencyBin>& spectrum) const;

	bool spectrumPrevState;
	SpectrumAnalyzer *spectrum;
	StCCallback cb;
	std::unique_ptr<BarGraph> graph;

	Color leftColor, rightColor, monoColor, centerColor, bassColor, matrixColor;
	std::vector<Color> frequencyColors;
	
	double filterStrength;
	double bassFreq, trebbleFreq;
	double noiseFloor;
	double bassBoost;
};