#include "SerialPort.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

SerialPort::SerialPort() {
	comHandle = INVALID_HANDLE_VALUE;
	ready = false;
}

int SerialPort::Connect(byte id, DWORD baudRate) {
	DCB dcb;
	COMMTIMEOUTS timeouts;

	char portName[16], buffer[16];

	if (ready || (comHandle != INVALID_HANDLE_VALUE))
		Disconnect();

	strcpy(portName, "\\\\.\\COM");
	strcat(portName, _itoa(id, buffer, 10));

	comHandle = CreateFile(portName , GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (comHandle == INVALID_HANDLE_VALUE)
		return ERROR_OPENING_COMM_PORT;

	GetCommState(comHandle, &dcb);

	dcb.BaudRate = baudRate;
	dcb.fBinary = 1;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;

	if (!SetCommState(comHandle, &dcb)) {
		Disconnect();
		return ERROR_SETTING_COMM_SETTINGS;
	}

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.WriteTotalTimeoutConstant = 1;
	timeouts.WriteTotalTimeoutMultiplier = 1;

	if (!SetCommTimeouts(comHandle, &timeouts)) {
		Disconnect();
		return ERROR_SETTING_COMM_TIMEOUTS;
	}

	ready = true;

	Sleep(1000);

	return ready;
}

void SerialPort::Disconnect() {
	if (!ready)
		return;

	CloseHandle(comHandle);
	
	comHandle = INVALID_HANDLE_VALUE;
	ready = false;
}

bool SerialPort::IsReady() {
	return ready;
}

int SerialPort::WriteBytes(int length, byte *data) {
	if (!ready || comHandle == INVALID_HANDLE_VALUE)
		return ERROR_PORT_NOT_READY;

	if (data == NULL)
		return ERROR_GIVEN_NULL_POINTER;

	DWORD nWritten;

	if (!WriteFile(comHandle, (LPCVOID)data, length, &nWritten, NULL)) {
		ready = false;
		return ERROR_GENERIC_WRITE_FAIL;
	}

	return nWritten;
}

int SerialPort::ReadBytes(int length, byte *data) {
	if (!ready || comHandle == INVALID_HANDLE_VALUE)
		return ERROR_PORT_NOT_READY;

	if (data == NULL)
		return ERROR_GIVEN_NULL_POINTER;

	DWORD nRead;

	if (!ReadFile(comHandle, (LPVOID)data, length, &nRead, NULL)) {
		ready = false;
		return ERROR_GENERIC_READ_FAIL;
	}

	return nRead;
}

SerialPort::~SerialPort() {
	Disconnect();
}