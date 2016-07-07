#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "StripModes.h"
#include "SerialPort.h"
#include "Spectrum.h"
#include "RecordingDevice.h"
#include "WindowManager.h"
#include "BarGraph.h"
#include "Drawables.h"
#include "SoundToColor.h"
#include "LEDMatrix.hpp"
#include "DisplayEffect.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#define FRONT_PIXEL_COUNT		300
#define DUMB_STRIP_COUNT		1
#define TOTAL_COLOR_COUNT		(FRONT_PIXEL_COUNT + DUMB_STRIP_COUNT)
#define BAUD					2000000L
#define COM_STARTBYTE			0xAA

#define IP_ADDR			"192.168.2.8"
#define PORT_NUM			30962

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

int InitStrip(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& endpoint);
void UpdateStrip(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& endpoint, LightStrip *strip, int ledCount);

boost::asio::io_service ioService;

int main() {
	int comPort;
	int ledCount;
	__int64 timerFrequency;
	std::string ipAddr;

	std::cout << "LightNode IP Address: ";
	std::cin >> ipAddr;

	boost::asio::ip::udp::socket socket(ioService);
	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddr), PORT_NUM);

	//SerialPort serialPort;
	RecordingDevice recorder(4, 4092, 48000);
	WindowManager window("LightControl", "Light Controller", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0);
	
	ledCount = InitStrip(socket, endpoint);

	StripModeDissipate stripMode(ledCount, 10, 50);
	StripModeSolid solidMode(ledCount, Color(0, 0, 0));

	Background background(Color(0, 0, 0));
	SpectrumAnalyzer spectrum(&recorder, 30., 20000., 3);
	SoundToColor soundColor(&window, 0.1, 0.1, 0.8, 0.8, &spectrum);

	window.AddDrawable(&background);
	window.AddDrawable(&soundColor);

//	std::cout << "COM Port Number: ";
//	std::cin >> comPort;

//	serialPort = SerialPort();
//	serialPort.Connect(comPort, BAUD);

//	stripData[0] = COM_STARTBYTE;

	window.Start();
	float hue = 0.;

	//LightStrip display(32 * 24);
	LEDMatrix display(32, 24);
	
	double xPos = display.getWidth() / 2;
	double xDir = 0;// 0.5;

	while (1) {
		static int tick = 0;
		
		unsigned char receiveByte;

		stripMode.PushColors(soundColor.GetLeftColor(), soundColor.GetRightColor());
		solidMode.SetColor(soundColor.GetMonoColor());
		//solidMode.SetColor(Color(255, 0, 0));

		window.Update();

		//UpdateStrip(socket, endpoint, stripMode, ledCount);
		//UpdateStrip(socket, endpoint, solidMode.GetStrip(), ledCount);

		//Update graph
		auto values = soundColor.getGraph()->getValues();
		auto colors = soundColor.getGraph()->getColors();

		updateDisplay(display, soundColor.GetMonoColor(), xPos, display.getHeight()/2, true);
		//renderField(display, soundColor.GetMonoColor(), xPos, display.getHeight() / 2);

		xPos += xDir;

		if (xPos >= display.getWidth()*3/4 || xPos < display.getWidth()/4) {
			xDir *= -1;
		}

		/*
		display.clear();

		for (int x = 0; x < values.size(); x += 3) {
			double top = 0;

			int max = std::min((int)values.size(), x + 3);
			for (int temp = x; temp < max; temp++) {
				top += std::max(23 - (values[temp - (temp % 2)] + 70) * 0.4, 0.);
			}

			top /= (max - x);

			if (top < 0)
				top = 0;

			for (int i = 0; i < 2; ++i) {
				int xx = x + i;
				int scrX = xx + 2;

				//auto raw = colors[x];

				auto raw = Color::HSV(240.f * ((1. + x) / values.size()), 1.f, 1.f).gammaCorrected();

				for (int y = 23; y >= top; y--) {
					//int index = (y % 2) ? (32 * y + 31 - scrX) : (32 * y + scrX);
					//display.Set(index, c);
					display.setPixel(scrX, y, raw);
				}
			}
		}
		*/

		UpdateStrip(socket, endpoint, display.getStrip().get(), 32 * 24);

		hue += 1.;
		hue = std::fmod(hue, 360.);

		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		//Color::HSV(hue, 1, 1).Print();
	}

//	delete[] stripData;
}

int InitStrip(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& endpoint) {
	socket.open(boost::asio::ip::udp::v4());
	boost::asio::ip::udp::endpoint recEndpoint;

	int ledCount;

	std::vector<unsigned char> data;
	boost::array<unsigned char, 128> receiveData;

	std::cout << "Sending ID_INIT" << std::endl;
	data.push_back(0xAA);
	data.push_back(0x55);
	data.push_back(0x00);

	socket.send_to(boost::asio::buffer(data), endpoint);

	std::cout << "Receiving..." << std::endl;
	int recSize = socket.receive_from(boost::asio::buffer(receiveData), recEndpoint);

	std::cout << "Received " << recSize << " bytes" << std::endl;

	if (recSize == 5) {
		int id = receiveData[2];
		ledCount = receiveData[3] << 8 | receiveData[4];

		std::cout << "Message ID: " << id << std::endl;
		std::cout << "LED count: " << ledCount << std::endl;
	}

	return ledCount;
}

void UpdateStrip(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& endpoint, LightStrip *rawStrip, int ledCount) {
	unsigned char *colors = new unsigned char[ledCount * 3];

	//strip.GetRawColors(colors);
	//auto rawStrip = strip.GetStrip();

	std::vector<unsigned char> vec;

	vec.push_back(0xAA);
	vec.push_back(0x55);
	vec.push_back(0x02);

	for (int i = 0; i < ledCount; i++) {
		Color c = rawStrip->GetColor(i);

		vec.push_back(c.GetRed());
		vec.push_back(c.GetGreen());
		vec.push_back(c.GetBlue());

		/*vec.push_back(0);
		vec.push_back(255);
		vec.push_back(0);*/

		//c.Print();
	}

	socket.send_to(boost::asio::buffer(vec), endpoint);
}