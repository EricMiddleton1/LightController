#pragma once

#include <cmath>
#include <algorithm>
#include <string>

typedef unsigned char byte;

class Color {
public:
	Color();
	Color(unsigned long c);
	Color(int r, int g, int b);

	Color operator=(Color c);
	bool operator!=(Color rhs);
	bool operator==(Color rhs);

	std::string ToString();

	void Scale(float scale);
	void Filter(Color c, float strength);

	byte GetRed();
	byte GetGreen();
	byte GetBlue();

	Color gammaCorrected();

	int GetLargestComponent(byte *component);

	void SetRed(byte red);
	void SetGreen(byte green);
	void SetBlue(byte blue);

	float GetHue();
	float GetHSLSaturation();
	float GetLightness();

	float GetHSVSaturation();
	float GetValue();

	void SetSaturation(float value);

	void Print();

	unsigned long GetWin32Color();

	static Color ColorWheel(float pos);
	static Color HSL(float hue, float saturation, float lightness);
	static Color HSV(float hue, float saturation, float value);

private:
	int r, g, b;
};