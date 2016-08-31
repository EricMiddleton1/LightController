#include "DisplayEffect.hpp"

void GetAdjacentCoordinate(int dirX, int dirY, int& coordX, int& coordY);

void updateDisplay(LEDMatrix& display, const Color& c, uint8_t xPos, uint8_t yPos, bool explode) {
	static std::vector<Color> previousColors;

	int centerX = (display.getWidth() + 0.5f) / 2;
	int centerY = (display.getHeight() + 0.5f) / 2;

	int radius = std::sqrt(centerX*centerX + centerY*centerY);

	while (previousColors.size() < display.getWidth()) {
		previousColors.push_back(Color());
	}

	previousColors.push_back(c);

	previousColors.erase(previousColors.begin());


	for (int y = 0; y < display.getHeight(); y++) {
		for (int x = 0; x < display.getWidth(); x++) {
			int distX = (x -xPos), distY = (y - yPos);
			int dist = std::sqrt(distX*distX + distY*distY) + 0.5;

			int colorIndex = explode ? (display.getWidth() - dist - 1) : dist;

			display.setPixel(x, y, previousColors[colorIndex]);
		}
	}
}

void renderField(LEDMatrix& display, const Color& c, uint8_t xPos, uint8_t yPos) {
	//Get a copy of the current matrix
	LEDMatrix copy = display.copy();

	//Render each pixel
	for (int y = 0; y < display.getHeight(); y++) {
		for (int x = 0; x < display.getWidth(); x++) {
			//Calculate vector at current position
			int dirX = xPos - x, dirY = yPos - y;
			
			//Get coordinates of adjacent pixel in direction (dirX, dirY)
			int coordX, coordY;
			GetAdjacentCoordinate(dirX, dirY, coordX, coordY);

			//Convert to screen coordinates
			coordX = min(display.getWidth() - 1, max(0, x + coordX));
			coordY = min(display.getHeight() - 1, max(0, y + coordY));

			//Update new pixel
			Color c = copy.getPixel(coordX, coordY);
			display.setPixel(x, y, c);
		}
	}

	display.setPixel(xPos, yPos, c);
}

void GetAdjacentCoordinate(int dirX, int dirY, int& coordX, int& coordY) {
	//Get angle of vector
	double angle = std::atan2(dirY, dirX) * 180. / 3.141592654;

	if (angle > 157.5 || angle < -157.5) {
		coordX = -1;
		coordY = 0;
	}
	else if (angle > 112.5) {
		coordX = -1;
		coordY = 1;
	}
	else if (angle > 67.5) {
		coordX = 0;
		coordY = 1;
	}
	else if (angle > 22.5) {
		coordX = 1;
		coordY = 1;
	}
	else if (angle > -22.5) {
		coordX = 1;
		coordY = 0;
	}
	else if (angle > -67.5) {
		coordX = 1;
		coordY = -1;
	}
	else if (angle > -112.5) {
		coordX = 0;
		coordY = -1;
	}
	else {
		coordX = -1;
		coordY = -1;
	}
}

void mirrorDisplay(LEDMatrix& display) {
	HDC      dcDesktop;
	HDC         dcMem;
	HBITMAP     hbmpMem;
	HBITMAP     hOriginal;
	BITMAP      bmpDesktopCopy;
	int scrWidth, scrHeight;

	scrWidth = 1920;
	scrHeight = 1080;

	dcDesktop = GetDC(GetDesktopWindow());
	dcMem = CreateCompatibleDC(dcDesktop);
	hbmpMem = CreateCompatibleBitmap(dcDesktop, scrWidth, scrHeight);
	SelectObject(dcMem, hbmpMem);

	BitBlt(dcMem, 0, 0, scrWidth, scrHeight, dcDesktop, 0, 0, SRCCOPY | CAPTUREBLT);

	// Copy the hbmpMem to the desktop copy
	GetObject(hbmpMem, sizeof(BITMAP), (LPSTR)&bmpDesktopCopy);

	HBITMAP bitmapScaled;
	HDC scaledDC;

	scaledDC = CreateCompatibleDC(dcDesktop);
	bitmapScaled = CreateCompatibleBitmap(dcDesktop, display.getWidth(), display.getHeight());
	SelectObject(scaledDC, bitmapScaled);

	StretchBlt(scaledDC, 0, 0, display.getWidth(), display.getHeight(), dcMem, 0, 0, scrWidth, scrHeight, SRCCOPY);

	for (int y = 0; y < display.getHeight(); y++) {
		for (int x = 0; x < display.getWidth(); x++) {
			uint32_t color = GetPixel(scaledDC, x, y);

			Color c(color & 0xFF,
				(color >> 8) & 0xFF,
				(color >> 16) & 0xFF);

			display.setPixel(x, y, c);
		}
	}

	DeleteObject(bitmapScaled);
	DeleteObject(dcMem);

	DeleteDC(scaledDC);
	DeleteDC(dcMem);

	ReleaseDC(GetDesktopWindow(), dcDesktop);
}