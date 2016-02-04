#pragma once

#include <Windows.h>
#include "WindowManager.h"
#include "Color.h"

class Drawable {
public:
	Drawable(WindowManager *window, float x, float y, float width, float height);

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	virtual void Draw(HDC hdc, int scrWidth, int scrHeight){}

	void SetColor(Color c);

	~Drawable(){}
protected:
	void StartDraw(HDC hdc, int width, int height);
	void EndDraw(HDC hdc, int x, int y, int width, int height);

	void LocalToGlobal(int windowWidth, int windowHeight, float localX, float localY, int *globalX, int *globalY);
	int LocalToGlobalX(int windowWidth, float localX);
	int LocalToGlobalY(int windowHeight, float localY);

	float x, y, width, height;
	HDC memDC, maskDC;
	HBITMAP memBitmap, maskBitmap, defMemBitmap, defMaskBitmap;
	Color c;
	WindowManager *window;
};