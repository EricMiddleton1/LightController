#include "BarGraph.h"

BarGraph::BarGraph(WindowManager *window, Color defaultColor, float x, float y, float width, float height, double min, double max) :
			Drawable(window, x, y, width, height) {
	this->c = defaultColor;
	this->min = min;
	this->max = max;
}

BarGraph::~BarGraph() {

}

void BarGraph::UpdateGraph(std::vector<double> values, bool redraw) {
	//this->values = values;

	if (values.size() != this->values.size()) {
		this->values = values;
	}
	else {
		for (int i = 0; i < values.size(); i++) {
			//this->values[i] = 0.5*this->values[i] + 0.5*values[i];
			double difference = values[i] - this->values[i];
			if (difference >= 0)
				this->values[i] = values[i];
			else
				this->values[i] += max(-1, difference);
		}
	}

	if (redraw)
		window->ForceDraw();
}

void BarGraph::SetColors(std::vector<Color> &colors) {
	this->colors = colors;
}

void BarGraph::SetLabels(std::vector<std::string> &labelStrings) {
	for (auto &t : labels) {
		delete t;
	}

	labels.clear();

	for (int i = 0; i < labelStrings.size(); i++) {
		Color tColor = (i < colors.size()) ? (colors[i]) : c;
		
		Text *t = new Text(window, tColor, 0, 0, 0, 0);
		t->SetFont("Calibri", 16);
		t->SetFormat(DT_CENTER | DT_VCENTER);

		labels.push_back(t);
	}
}

void BarGraph::Draw(HDC hdc, int scrWidth, int scrHeight) {
	if (values.size() == 0)
		return;

	HBRUSH brush = CreateSolidBrush(c.GetWin32Color()), defBrush;
	HPEN pen = CreatePen(PS_SOLID, 1, c.GetWin32Color()), defPen;

	defBrush = (HBRUSH)SelectObject(hdc, brush);
	defPen = (HPEN)SelectObject(hdc, pen);

	float dx = 1. / values.size();

	for (int i = 0; i < values.size(); i++) {
		HBRUSH tempBrush = NULL;
		HPEN tempPen = NULL;
		double scaled = ScaleValue(values[i]);
		int x1 = LocalToGlobalX(scrWidth, i*dx), x2 = LocalToGlobalX(scrWidth, i*dx + dx/2.);
		int y1 = LocalToGlobalY(scrHeight, 1. - scaled), y2 = LocalToGlobalY(scrHeight, 1);

		if ((y2 - y1) < 1)
			y1 = y1 - 1;

		if (i < colors.size()) {
			tempBrush = CreateSolidBrush(colors[i].GetWin32Color());
			tempPen = CreatePen(PS_SOLID, 1, colors[i].GetWin32Color());

			SelectObject(hdc, tempBrush);
			SelectObject(hdc, tempPen);
		}

		Rectangle(hdc, x1, y1, x2, y2);

		if (tempBrush != NULL) {
			SelectObject(hdc, brush);
			SelectObject(hdc, pen);

			DeleteObject(tempBrush);
			DeleteObject(tempPen);
		}
	}

	SelectObject(hdc, defBrush);
	SelectObject(hdc, defPen);

	DeleteObject(brush);
	DeleteObject(pen);
}

double BarGraph::ScaleValue(double val) {
	double scaled = (val - min) / (max - min);

	return (scaled < 0) ? 0 : scaled;
}