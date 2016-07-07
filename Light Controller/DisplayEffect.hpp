#pragma once

#include "LEDMatrix.hpp"

void updateDisplay(LEDMatrix& display, const Color& c, uint8_t xPos, uint8_t yPos, bool explode = false);

void renderField(LEDMatrix& display, const Color& c, uint8_t xPos, uint8_t yPos);