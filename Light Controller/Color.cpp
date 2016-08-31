#include "Color.h"

unsigned char gamma[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

Color::Color() {
	r = g = b = 0;
}

Color::Color(int r, int g, int b) {
	this->r = r;
	this->g = g;
	this->b = b;
}

Color::Color(unsigned long color) {
	r = (color & 0x000000FF) << 4;
	g = (color & 0x0000FF00) >> 4;
	b = (color & 0x00FF0000) >> 12;
}

Color Color::operator=(Color c) {
	r = c.r;
	g = c.g;
	b = c.b;

	return *this;
}

bool Color::operator!=(Color rhs) {
	bool res = (r != rhs.r) || (g != rhs.g) || (b != rhs.b);

	return res;
}

bool Color::operator==(Color rhs) {
	bool res = (r == rhs.r) && (g == rhs.g) && (b == rhs.b);

	return res;
}

Color Color::gammaCorrected() {
	return Color(gamma[r], gamma[g], gamma[b]);
}

std::string Color::ToString() {
	return "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
}

void Color::Scale(float scale) {
	r *= scale;
	g *= scale;
	b *= scale;
}

void Color::Filter(Color c, float strength) {
	strength = std::min(1.f, std::max(0.f, strength)); 

	r = r*strength + c.GetRed()*(1 - strength);

	g = g*strength + c.GetGreen()*(1 - strength);

	b = b*strength + c.GetBlue()*(1 - strength);
}

Color Color::SubFiler(Color c, float sub) {
	if (c.r > r)
		r = c.r;
	else
		r -= std::min(sub, (float)r - c.r);

	if (c.g > g)
		g = c.g;
	else
		g -= std::min(sub, (float)g - c.g);

	if (c.b > b)
		b = c.b;
	else
		b -= std::min(sub, (float)b - c.b);

	return *this;
}

byte Color::GetRed() const {
	return r;
}

byte Color::GetGreen() const {
	return g;
}

byte Color::GetBlue() const {
	return b;
}

float Color::GetHue() {
	int min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float hprime, hue;

	if (chroma == 0)
		hprime = 0;
	else if (max == r)
		hprime = std::fmod(((float)g - b) / chroma, 6.);
	else if (max == g)
		hprime = ((float)b - r) / chroma + 2;
	else
		hprime = ((float)r - g) / chroma + 4;

	hue = 60.*hprime;

	if (hue < 0)
		hue += 360;
	else if (hue > 360)
		hue -= 360;

	return hue;
}

float Color::GetHSLSaturation() {
	float min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float saturation, lightness = GetLightness();
	
	if (chroma == 0)
		saturation = 0;
	else
		saturation = (float)chroma / (1 - std::abs(2.*lightness - 1));

	return saturation / 255.;
}

float Color::GetLightness() {
	byte min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));

	return ((float)min + max) / (2. * 255.);
}

float Color::GetHSVSaturation() {
	float min = std::min(r, std::min(g, b)), max = std::max(r, std::max(g, b));
	float chroma = max - min;
	float saturation, value = GetValue();

	if (chroma == 0)
		saturation = 0;
	else
		saturation = chroma / value;

	return saturation / 255.;
}

float Color::GetValue() {
	return std::max({ r, g, b }) / 255.f;
}

void Color::SetSaturation(float value) {
}

int Color::GetLargestComponent(byte *component) {
	int largest = std::max(r, std::max(g, b));

	if (component != NULL)
		*component = (r > g) ? (r > b ? 1 : 3) : (g > b) ? 2 : 3;

	return largest;
}

void Color::Print() {
	printf("(%d, %d, %d)\n", r, g, b);
}

Color Color::ColorWheel(float pos) {
	const byte reds[] = { 255, 255, 0, 0, 0, 255 },
		greens[] = { 0, 255, 255, 255, 0, 0 },
		blues[] = { 0, 0, 0, 255, 255, 255 };

	pos = fmod(pos, 1.);

	int index = (int)(pos * 6) % 6, next = (index + 1) % 6;
	float mult = 6 * pos - index;

	return Color((byte)(reds[index] * (1 - mult) + reds[next] * mult),
		(byte)(greens[index] * (1 - mult) + greens[next] * mult),
		(byte)(blues[index] * (1 - mult) + blues[next] * mult));
}

Color Color::HSL(float hue, float saturation, float lightness) {
	float chroma, hprime, x, m, r, g, b;

	hue = std::fmod(hue, 360.);

	hprime = hue / 60.f;

	chroma = (1 - std::abs(2 * lightness - 1)) * saturation;
	x = (1 - std::abs(std::fmod(hprime, 2) - 1)) * chroma;

	if (hprime < 1) {
		r = chroma;
		g = x;
		b = 0;
	}
	else if (hprime < 2) {
		r = x;
		g = chroma;
		b = 0;
	}
	else if (hprime < 3) {
		r = 0;
		g = chroma;
		b = x;
	}
	else if (hprime < 4) {
		r = 0;
		g = x;
		b = chroma;
	}
	else if (hprime < 5) {
		r = x;
		g = 0;
		b = chroma;
	}
	else {
		r = chroma;
		g = 0;
		b = x;
	}

	m = lightness - chroma / 2.;

	r += m;
	g += m;
	b += m;

	return Color(255 * r, 255 * g, 255 * b);
}

Color Color::HSV(float hue, float saturation, float value) {
	float chroma, hprime, x, m, r, g, b;

	hue = std::fmod(hue, 360.);

	hprime = hue / 60.f;

	chroma = value * saturation;
	x = (1 - std::abs(std::fmod(hprime, 2) - 1)) * chroma;

	if (hprime < 1) {
		r = chroma;
		g = x;
		b = 0;
	}
	else if (hprime < 2) {
		r = x;
		g = chroma;
		b = 0;
	}
	else if (hprime < 3) {
		r = 0;
		g = chroma;
		b = x;
	}
	else if (hprime < 4) {
		r = 0;
		g = x;
		b = chroma;
	}
	else if (hprime < 5) {
		r = x;
		g = 0;
		b = chroma;
	}
	else {
		r = chroma;
		g = 0;
		b = x;
	}

	m = value - chroma;

	r += m;
	g += m;
	b += m;

	return Color(255 * r, 255 * g, 255 * b);
}

unsigned long Color::GetWin32Color() {
	unsigned long win32 = r;

	win32 |= ((unsigned long)g) << 8;
	win32 |= ((unsigned long)b) << 16;

	return win32;
}