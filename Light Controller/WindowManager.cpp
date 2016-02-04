#include "WindowManager.h"
#include "Drawable.h"
//#include "stdafx.h"

std::vector<Drawable*> drawObjects;
std::vector<WindowManager::TimerCallback> timerCallbacks;
WindowManager::Callback *keyCallback;

WindowManager::WindowManager(std::string className, std::string windowName, unsigned int windowStyle, int x, int y, int width, int height) {
	this->hInstance = hInstance;
	this->windowStyle = windowStyle;
	
	this->className = std::string(className);
	this->windowName = std::string(windowName);

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;

	//drawObjects = std::vector<Drawable>;

	keyCallback = nullptr;

	hWnd = NULL;
	hThread = NULL;

	//MakeWindow();
}

WindowManager::~WindowManager() {
	if(hWnd != NULL)
		CloseWindow(hWnd);
}

void WindowManager::Start() {
	//hThread = CreateThread(NULL, 0, WindowManager::ThreadCallback, (void*)this, 0, NULL);
	MakeWindow();
}

HWND WindowManager::GetHWND() {
	return hWnd;
}

int WindowManager::MakeWindow() {
	int retval;
	std::string cnCopy(className);

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowManager::StaticWindowCallback;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCE(IDC_BASESTATION);
	wcex.lpszClassName = className.c_str();
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	retval = RegisterClassEx(&wcex);

	if(!retval) {
		retval = GetLastError();
		//return false;
	}

	hWnd = CreateWindow(className.c_str(), windowName.c_str(), windowStyle, x, y, width, height, NULL, NULL, hInstance, (void*)this);

	if (hWnd == NULL) {
		retval = GetLastError();
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return true;
}


LRESULT WindowManager::StaticWindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static WindowManager *manager = NULL;
	
	if (message == WM_CREATE) {
		manager = (WindowManager*)lParam;
	}

	if (manager != NULL)
		return manager->WindowCallback(hWnd, message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT WindowManager::WindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int width, height;
	PAINTSTRUCT ps;
	HDC hdc, memDC;
	HBITMAP bitmap, defBitmap;
	RECT winRec;

	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		else if (keyCallback != nullptr)
			keyCallback->WindowCallback(CALLBACK_KEYDOWN, wParam);
		break;

	case WM_KEYUP:
		if (keyCallback != nullptr)
			keyCallback->WindowCallback(CALLBACK_KEYUP, wParam);
		break;

	case WM_ERASEBKGND:
		return true;
		break;

	case WM_TIMER:
		SetTimer(hWnd, wParam, timerCallbacks[wParam-1].duration, NULL);

		if (timerCallbacks[wParam -1].cb != nullptr)
			timerCallbacks[wParam-1].cb->WindowCallback(CALLBACK_TIMER, wParam);

		return 0;
	break;

	case WM_PAINT:
		GetClientRect(hWnd, &winRec);
		
		width = winRec.right - winRec.left;
		height = winRec.bottom - winRec.top;

		hdc = BeginPaint(hWnd, &ps);

		memDC = CreateCompatibleDC(hdc);
		bitmap = CreateCompatibleBitmap(hdc, width, height);
		defBitmap = (HBITMAP)SelectObject(memDC, bitmap);

		BitBlt(memDC, 0, 0, width, height, NULL, 0, 0, WHITENESS);

		if (drawObjects.size() > 0) {
			try {
				for (std::vector<Drawable*>::iterator i = drawObjects.begin(); i < drawObjects.end(); i++)
					(*i)->Draw(memDC, width, height);
			}
			catch (std::exception e) {
				MessageBox(hWnd, e.what(), "Drawing Exception", MB_ICONERROR | MB_OK);
			}
		}

		BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

		SelectObject(hdc, defBitmap);
		DeleteObject(bitmap);
		DeleteDC(memDC);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool WindowManager::IsRunning() {
	return IsWindow(hWnd);
}

int WindowManager::AddDrawable(Drawable *obj) {
	drawObjects.push_back(obj);

	return drawObjects.size();
}

int WindowManager::AddTimer(WindowManager::Callback *callback, int duration) {
	timerCallbacks.push_back(TimerCallback(callback, duration));

	SetTimer(hWnd, timerCallbacks.size(), duration, NULL);

	return timerCallbacks.size();
}

void WindowManager::SetKeyboardCallback(Callback *cb) {
	keyCallback = cb;
}

bool WindowManager::Update() {
	MSG msg;
	int retVal;

	if (!IsRunning())
		return false;

	retVal = GetMessage(&msg, NULL, 0, 0);
	
	if (retVal == -1)
		return false;
	
	TranslateMessage(&msg);
	DispatchMessage(&msg);

	return retVal;
}

void WindowManager::ForceDraw() {
	InvalidateRect(hWnd, NULL, false);
}