#pragma once

#include "Drawable.h"
#include "Color.h"
#include <string>

class Background : public Drawable
{
public:
	//Background() : Drawable(0, 0, 0, 0){}
	Background(Color c);

	~Background();

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight);

	void SetColor(Color c);
	Color GetColor();

private:
	void StartDraw(HDC hdc, int width, int height);
	void EndDraw(HDC hdc, int x, int y, int width, int height);
};

class Grid : public Drawable
{
public:
	Grid(WindowManager *window, Color c, int hTicks, int vTicks);
	Grid(WindowManager *window, Color c, float x, float y, float width, float height, float hTickDist, float vTickDist);
	Grid(WindowManager *window, Color c, float x, float y, float width, float height,
		float xStart, float xEnd, float yStart, float yEnd, float hTickDist, float vTickDist);

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight);

	void SetColor(Color c);
	Color GetColor();

	float MapToGridX(float x);
	float MapToGridY(float y);

	float ScaleX(float x);
	float ScaleY(float y);
private:
	int hTicks, vTicks;
	float xStart, xEnd, yStart, yEnd;
};

class Text : public Drawable
{
public:
	//Text() : Drawable(0, 0, 0, 0) {}
	Text(WindowManager *window, Color c, float x, float y, float width, float height);
	~Text();

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight);

	void SetFont(std::string name, int size);
	void SetString(std::string str);
	void SetFormat(unsigned int format);

private:
	void MakeFont(HDC hdc, HFONT *font);
	void Setup(HDC hdc);
	void Cleanup();

	HFONT font;
	int fullWidth, fullHeight;
	std::string fontName, str;
	int size;
	unsigned int format;
	bool setup;
};

class Trail : public Drawable
{
public:
	Trail(WindowManager *window, Color c, float x, float y, float width, float height, float threshold);
	~Trail() {}

	bool Add(float x, float y);

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight);

private:
	struct Point {
		float x, y;
		static Point MakePoint(float x, float y) {
			Point p = { x, y };
			return p;
		}
	};

	float Distance(Point p1, Point p2);

	std::vector<Point> points;
	float threshold;
};