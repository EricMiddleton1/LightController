#include "Drawables.h"

//-----------------------------//
//--Background class methods---//
//-----------------------------//
Background::Background(Color c) : Drawable(nullptr, 0, 0, 1, 1) {
	this->c = c;
}

Background::~Background() {
}

void Background::SetColor(Color c) {
	this->c = c;
}

Color Background::GetColor() {
	return c;
}

void Background::Draw(HDC hdc, int scrWidth, int scrHeight) {
	HPEN invisPen, defPen;
	HBRUSH bkBrush, defBrush;
	COLORREF winColor = RGB(c.GetRed(), c.GetGreen(), c.GetBlue());
	int width = this->width*scrWidth, height = this->height*scrHeight;

	invisPen = CreatePen(PS_SOLID, 1, winColor);
	bkBrush = CreateSolidBrush(winColor);

	defPen = (HPEN)SelectObject(hdc, invisPen);
	defBrush = (HBRUSH)SelectObject(hdc, bkBrush);

	Rectangle(hdc, 0, 0, width, height);

	SelectObject(hdc, defPen);
	SelectObject(hdc, defBrush);

	DeleteObject(invisPen);
	DeleteObject(bkBrush);
}

Grid::Grid(WindowManager *window, Color c, int hTicks, int vTicks) : Drawable(window, 0, 0, 1, 1) {
	this->hTicks = hTicks;
	this->vTicks = vTicks;
	this->c = c;
}

Grid::Grid(WindowManager *window, Color c, float x, float y, float width, float height, float hTickDist, float vTickDist) : Drawable(window, x, y, width, height) {
	this->c = c;

	this->hTicks = 1. / hTickDist;
	this->vTicks = 1. / vTicks;
}

Grid::Grid(WindowManager *window, Color c, float x, float y, float width, float height, 
		float xStart, float xEnd, float yStart, float yEnd, float hTickDist, float vTickDist) : Drawable(window, x, y, width, height) {
	this->c = c;

	this->hTicks = (xEnd - xStart) / hTickDist;
	this->vTicks = (yEnd - yStart) / vTickDist;

	this->xStart = xStart;
	this->xEnd = xEnd;
	this->yStart = yStart;
	this->yEnd = yEnd;
}

void Grid::Draw(HDC hdc, int scrWidth, int scrHeight) {
	HPEN gridPen = CreatePen(PS_DOT, 1, c.GetWin32Color()), defPen;
	float segment;
	int bkMode;

	defPen = (HPEN)SelectObject(hdc, gridPen);

	bkMode = GetBkMode(hdc);
	SetBkMode(hdc, TRANSPARENT);

	segment = 1.f / (hTicks);
	for (int i = 0; i < (hTicks + 1); i++) {
		int x, y;

		LocalToGlobal(scrWidth, scrHeight, segment*i, 0, &x, &y);
		MoveToEx(hdc, x, y, NULL);

		LocalToGlobal(scrWidth, scrHeight, segment*i, 1, &x, &y);
		LineTo(hdc, x, y);
	}

	segment = 1.f / (vTicks);
	for (int i = 0; i < (vTicks + 1); i++) {
		int x, y;

		LocalToGlobal(scrWidth, scrHeight, 0, segment*i, &x, &y);
		MoveToEx(hdc, x, y, NULL);

		LocalToGlobal(scrWidth, scrHeight, 1, segment*i, &x, &y);
		LineTo(hdc, x, y);
	}

	SelectObject(hdc, defPen);
	DeleteObject(gridPen);

	SetBkMode(hdc, bkMode);
}

float Grid::MapToGridX(float x) {
	float width = xEnd - xStart;

	return (x - xStart) / width;
}

float Grid::MapToGridY(float y) {
	float width = yEnd - yStart;

	return (y - yStart) / width;
}

float Grid::ScaleX(float x) {
	return x / (xEnd - xStart);
}

float Grid::ScaleY(float y) {
	return y / (yEnd - yStart);
}

//-----------------------------//
//-----Text class methods------//
//-----------------------------//
Text::Text(WindowManager *window, Color c, float x, float y, float width, float height) : Drawable(window, x, y, width, height) {
	this->c = c;

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;

	fontName = "";
	size = 0;

	font = NULL;

	setup = false;
}

Text::~Text() {
}

void Text::Draw(HDC hdc, int scrWidth, int scrHeight) {
	//HDC memDC, maskDC;
	//HBITMAP bitmap, mask;
	HFONT defFont;
	RECT rect;
	int bkMode;

	MakeFont(hdc, &font);

	defFont = (HFONT)SelectObject(hdc, font);
	SetTextColor(hdc, c.GetWin32Color());

	bkMode = GetBkMode(hdc);
	SetBkMode(hdc, TRANSPARENT);

	rect.left = scrWidth*x;
	rect.top = scrHeight*y;
	rect.right = scrWidth*(x + width);
	rect.bottom = scrHeight*(y + height);

	DrawText(hdc, str.c_str(), -1, &rect, format);

	SelectObject(hdc, defFont);
	DeleteObject(font);
	SetBkMode(hdc, bkMode);
}

void Text::SetFont(std::string name, int size) {
	fontName = name;
	this->size = size;
}

void Text::SetString(std::string str) {
	this->str = str;
}

void Text::SetFormat(unsigned int format) {
	this->format = format;
}

void Text::MakeFont(HDC hdc, HFONT *font) {
	LOGFONT lf;
	int devcaps = GetDeviceCaps(hdc, LOGPIXELSY);

	lf.lfHeight = MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = false;
	lf.lfUnderline = false;
	lf.lfStrikeOut = false;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = NONANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	strcpy(lf.lfFaceName, fontName.c_str());

	*font = CreateFontIndirect(&lf);
}

Trail::Trail(WindowManager *window, Color c, float x, float y, float width, float height, float threshold) : Drawable(window, x, y, width, height) {
	this->c = c;
	this->threshold = threshold;
}

bool Trail::Add(float x, float y) {
	Point p1 = Point::MakePoint(x, y), p2;

	if (points.size() == 0) {
		points.push_back(p1);
		return true;
	}

	p2 = points[points.size() - 1];

	if (Distance(p1, p2) >= threshold) {
		points.push_back(p1);
		return true;
	}
	else
		return false;
}

float Trail::Distance(Point p1, Point p2) {
	float dx = p1.x - p2.x,
		dy = p1.y - p2.y;

	return std::sqrt(dx*dx + dy*dy);
}

void Trail::Draw(HDC hdc, int scrWidth, int scrHeight) {
	if (points.size() < 2)
		return;

	HPEN linePen = CreatePen(PS_SOLID, 2, c.GetWin32Color()), defPen;
	int offsetX = scrWidth*x, offsetY = scrHeight*y;

	defPen = (HPEN)SelectObject(hdc, linePen);

	MoveToEx(hdc, offsetX + scrWidth*points[0].x, offsetY + scrHeight*points[0].y, NULL);

	for (int i = 1; i < points.size(); i++) {
		LineTo(hdc, offsetX + scrWidth*points[i].x, offsetY + scrHeight*points[i].y);
	}

	SelectObject(hdc, defPen);
	DeleteObject(linePen);
}