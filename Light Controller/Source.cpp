#include "StripModes.h"
#include "SerialPort.h"
#include "Spectrum.h"
#include "RecordingDevice.h"
#include "WindowManager.h"
#include "BarGraph.h"
#include "Drawables.h"
#include "SoundToColor.h"
#include <cmath>
#include <iostream>

#define FRONT_PIXEL_COUNT		300
#define DUMB_STRIP_COUNT		1
#define TOTAL_COLOR_COUNT		(FRONT_PIXEL_COUNT + DUMB_STRIP_COUNT)
#define BAUD					2000000L
#define COM_STARTBYTE			0xAA

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

__int64 GetTimerFrequency() {
	__int64 freq;

	QueryPerformanceFrequency((LARGE_INTEGER*)(&freq));

	return freq;
}

__int64 GetTimerTime() {
	__int64 time;

	QueryPerformanceCounter((LARGE_INTEGER*)(&time));

	return time;
}

double EllapsedMillis(__int64 timer) {
	static __int64 freq = GetTimerFrequency();

	__int64 current;
	QueryPerformanceCounter((LARGE_INTEGER*)(&current));

	return 1000. * (current - timer) / freq;
}

int main() {
	int comPort;
	__int64 timerFrequency;
	unsigned char stripData[3 * TOTAL_COLOR_COUNT + 1];

	SerialPort serialPort;
	RecordingDevice recorder(4, 4092, 48000);
	WindowManager window("LightControl", "Light Controller", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0);
	
	StripModeDissipate stripMode(FRONT_PIXEL_COUNT, 10, 50);
	StripModeSolid solidMode(FRONT_PIXEL_COUNT, Color(0, 0, 0));

	Background background(Color(0, 0, 0));
	SpectrumAnalyzer spectrum(&recorder, 30., 20000., 3);
	SoundToColor soundColor(&window, 0.1, 0.1, 0.8, 0.8, &spectrum);

	window.AddDrawable(&background);
	window.AddDrawable(&soundColor);

	std::cout << "COM Port Number: ";
	std::cin >> comPort;

	serialPort = SerialPort();
	serialPort.Connect(comPort, BAUD);

	stripData[0] = COM_STARTBYTE;

	window.Start();
	float hue = 0.;
	while (1) {
		static int tick = 0;
		Color bass = soundColor.GetBassColor();
		//Color bass(255, 64, 0);
		
		unsigned char receiveByte;

		stripMode.PushColors(soundColor.GetLeftColor(), soundColor.GetRightColor());
		solidMode.SetColor(soundColor.GetMonoColor());

		stripMode.GetRawColors(stripData + 1);
		
		stripData[3 * FRONT_PIXEL_COUNT + 1] = bass.GetRed();
		stripData[3 * FRONT_PIXEL_COUNT + 2] = bass.GetGreen() / 2;
		stripData[3 * FRONT_PIXEL_COUNT + 3] = bass.GetBlue();

		serialPort.WriteBytes(3 * TOTAL_COLOR_COUNT + 1, stripData);
		
		__int64 time = GetTimerTime();
		while (!serialPort.ReadBytes(1, &receiveByte) && (EllapsedMillis(time) < 100));

		window.Update();
		
		tick++;

		if (tick > 100) {
			tick = 0;
			if (!serialPort.IsReady()) {
				serialPort.Connect(comPort, BAUD);
				std::cout << "Connecting to COM" << comPort << std::endl;
			}
		}

		hue += 1.;
		hue = std::fmod(hue, 360.);
	}

	delete[] stripData;
}