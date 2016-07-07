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
			int dist = std::sqrt(distX*distX + distY*distY);

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
			coordX = std::min(display.getWidth() - 1, std::max(0, x + coordX));
			coordY = std::min(display.getHeight() - 1, std::max(0, y + coordY));

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