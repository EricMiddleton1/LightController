#include "Drawable.h"

Drawable::Drawable(WindowManager *window, float x, float y, float width, float height) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->window = window;
}

void Drawable::SetPosition(float x, float y) {
	this->x = x;
	this->y = y;
}

void Drawable::SetSize(float width, float height) {
	this->width = width;
	this->height = height;
}

void Drawable::SetColor(Color c) {
	this->c = c;
}

void Drawable::StartDraw(HDC hdc, int width, int height) {
	memDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, width, height);
	defMemBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

	maskDC = CreateCompatibleDC(hdc);
	maskBitmap = CreateBitmap(width, height, 1, 1, NULL);
	defMaskBitmap = (HBITMAP)SelectObject(maskDC, maskBitmap);

	BitBlt(memDC, 0, 0, width, height, NULL, 0, 0, BLACKNESS);
	BitBlt(maskDC, 0, 0, width, height, NULL, 0, 0, WHITENESS);
}

void Drawable::EndDraw(HDC hdc, int x, int y, int width, int height) {
	BitBlt(hdc, x, y, width, height, maskDC, 0, 0, SRCAND);
	BitBlt(hdc, x, y, width, height, memDC, 0, 0, SRCPAINT);

	//BitBlt(hdc, x, y, width, height, memDC, 0, 0, SRCCOPY);

	SelectObject(memDC, defMemBitmap);
	SelectObject(maskDC, defMaskBitmap);

	DeleteObject(memBitmap);
	DeleteObject(maskBitmap);

	DeleteDC(memDC);
	DeleteDC(maskDC);
}

void Drawable::LocalToGlobal(int windowWidth, int windowHeight, float localX, float localY, int *globalX, int *globalY) {
	*globalX = (windowWidth - 1) * (x + localX*width);
	*globalY = (windowHeight - 1) * (y + localY*height);
}

int Drawable::LocalToGlobalX(int windowWidth, float localX) {
	return (windowWidth - 1) * (x + localX*width);
}

int Drawable::LocalToGlobalY(int windowHeight, float localY) {
	return (windowHeight - 1) * (y + localY*height);
}