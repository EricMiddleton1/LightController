#pragma once

#include "Drawable.h"
#include "Drawables.h"
#include "Color.h"

#include <memory>
#include <vector>

class BarGraph : public Drawable
{
public:
	BarGraph(WindowManager *window, Color defaultColor, float x, float y, float width, float height, double min, double max);
	~BarGraph();

	void UpdateGraph(std::vector<double> values, bool redraw = true);
	
	void SetColors(std::vector<Color> &colors);
	void SetLabels(std::vector<std::string> &labelStrings);

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight);
private:
	double ScaleValue(double val);

	std::vector<double> values;
	std::vector<Color> colors;
	std::vector<Text*> labels;
	double min, max;
};