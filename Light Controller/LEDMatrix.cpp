#include "LEDMatrix.hpp"

LEDMatrix::LEDMatrix(uint8_t _width, uint8_t _height)
	: width(_width)
	, height(_height)
	, strip(std::make_shared<LightStrip>(_width*_height)) {
}

LEDMatrix LEDMatrix::copy() {
	LEDMatrix newMatrix(width, height);

	//Manually copy pixels
	for (int i = 0; i < (width*height); i++) {
		newMatrix.strip->Set(i, strip->GetColor(i));
	}

	return newMatrix;
}

uint8_t LEDMatrix::getWidth() const {
	return width;
}

uint8_t LEDMatrix::getHeight() const {
	return height;
}

void LEDMatrix::clear() {
	strip->SetAll(Color());
}

void LEDMatrix::clear(const Color& c) {
	strip->SetAll(c);
}

bool LEDMatrix::setPixel(uint8_t x, uint8_t y, const Color& c) {
	if (x >= width || y >= height) {
		//Pixel coordinates out of bounds
		return false;
	}

	//Correct for layout of LED strips
	uint8_t corX = (y % 2) ? (width - x - 1) : x;

	//Set the color
	strip->Set(width*y + corX, c);

	return true;
}

Color LEDMatrix::getPixel(uint8_t x, uint8_t y) {
	if (x >= width || y >= height) {
		//Pixel coordinates out of bounds
		return Color();
	}

	//Correct for layout of LED strips
	uint8_t corX = (y % 2) ? (width - x - 1) : x;

	return strip->GetColor(width*y + corX);
}

std::shared_ptr<LightStrip> LEDMatrix::getStrip() {
	return strip;
}