#pragma once

#include <memory>
#include <cstdint>
#include "LightStrip.h"


class LEDMatrix
{
public:
	LEDMatrix(uint8_t width, uint8_t height);

	LEDMatrix copy();

	uint8_t getWidth() const;
	uint8_t getHeight() const;

	void clear();
	void clear(const Color& c);

	bool setPixel(uint8_t x, uint8_t height, const Color& c);
	Color getPixel(uint8_t x, uint8_t y);

	std::shared_ptr<LightStrip> getStrip();

private:
	std::shared_ptr<LightStrip> strip;

	uint8_t width, height;
};