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
#include "LightNode.hpp"
#include "LightHub.hpp"

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

	/*
	std::cout << "LightNode IP Address: ";
	std::cin >> ipAddr;

	boost::asio::ip::udp::socket socket(ioService);
	boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(ipAddr), PORT_NUM);
	*/

	//LightNode nodeAnalog("Analog", boost::asio::ip::address().from_string("192.168.1.145"), 54924);

	//SerialPort serialPort;
	RecordingDevice recorder(4, 4092, 48000);
	WindowManager window("LightControl", "Light Controller", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0);

	//StripModeDissipate stripMode(ledCount, 10, 50);
	StripModeSolid solidMode(1, Color(0, 0, 0));

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

	LightNode *nodeAnalog = NULL, *nodeMatrix = NULL, *edge = NULL;

	LightHub hub(54923, 54924, LightHub::BROADCAST);

	hub.addListener(LightHub::NODE_DISCOVER, [&nodeAnalog, &nodeMatrix, &edge](std::shared_ptr<LightNode> node) {
		std::cout << "[Info] New light node discovered: " << node->getName() << std::endl;

		node->addListener(LightNode::STATE_CHANGE, [&nodeAnalog, &nodeMatrix, &edge](LightNode* node, LightNode::State_e old, LightNode::State_e newState) {
			if (newState != LightNode::CONNECTED)
				return;

			auto strip = node->getLightStrip();

			if (strip.GetSize() == 5) {
				std::cout << "[Info] Analog strip connected" << std::endl;
				nodeAnalog = node;
			}
			else if (strip.GetSize() == 24 * 32) {
				std::cout << "[Info] LED Matrix connected" << std::endl;
				nodeMatrix = node;
			}
			else if (strip.GetSize() == (2 * 66 + 2 * 37)) {
				std::cout << "[Info] Edge light connected" << std::endl;
				edge = node;
			}
			node->releaseLightStrip();
		});
	});
	
	double xPos = display.getWidth() / 2;
	double xDir = 0;// 0.5;

	while (1) {
		static int tick = 0;
		
		unsigned char receiveByte;

		//stripMode.PushColors(soundColor.GetLeftColor(), soundColor.GetRightColor());
		//solidMode.SetColor(soundColor.GetMonoColor());
		//solidMode.SetColor(Color(255, 0, 0));

		window.Update();

		//Update graph
		auto values = soundColor.getGraph()->getValues();
		auto colors = soundColor.getGraph()->getColors();

		//updateDisplay(display, soundColor.GetMonoColor(), xPos, display.getHeight()/2, true);

		//std::cout << "[Info] " << hub.getNodeCount() << " connected nodes" << std::endl;

		if (nodeAnalog != NULL) {
			//auto strip = nodeAnalog->getLightStrip();

			nodeAnalog->getLightStrip().Set(0, soundColor.GetLeftColor());
			nodeAnalog->releaseLightStrip();

			nodeAnalog->getLightStrip().Set(1, soundColor.GetCenterColor());
			nodeAnalog->releaseLightStrip();

			nodeAnalog->getLightStrip().Set(2, soundColor.GetRightColor());
			nodeAnalog->releaseLightStrip();

			/*
			nodeAnalog->getLightStrip().SetAll(Color(0, 255, 0));
			nodeAnalog->releaseLightStrip();
			*/

			/*
			strip.Set(0, soundColor.GetMonoColor());
			strip.Set(1, soundColor.GetMonoColor());
			strip.Set(2, soundColor.GetMonoColor());
			*/

			//nodeAnalog->releaseLightStrip();

			nodeAnalog->update();
		}

		if (edge != NULL) {
			//edge->getLightStrip().SetAll(soundColor.GetMonoColor());
			edge->getLightStrip().SetAll(soundColor.GetBassColor());
			edge->releaseLightStrip();

			edge->update();
		}

		if (nodeMatrix != NULL) {
			updateDisplay(display, soundColor.GetMatrixColor(), xPos, display.getHeight() / 2, true);

			nodeMatrix->getLightStrip() = *display.getStrip();
			nodeMatrix->releaseLightStrip();

			nodeMatrix->update();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
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