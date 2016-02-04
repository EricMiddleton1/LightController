#pragma once

#include <Windows.h>
#include <string>
#include <vector>

//#include "Drawable.h"
class Drawable;

class WindowManager
{
public:
	static const int CALLBACK_TIMER = 1;
	static const int CALLBACK_KEYDOWN = 2;
	static const int CALLBACK_KEYUP = 3;
	static const int CALLBACK_MOUSE = 4;

	class Callback {
		public: virtual void WindowCallback(int type, int id) = 0;
	};

	class TimerCallback {
	public:
		TimerCallback(Callback *cb, int duration) {
			this->cb = cb;
			this->duration = duration;
		}
		Callback *cb;
		int duration;
	};

	WindowManager(std::string className, std::string windowName, unsigned int windowStyle, int x, int y, int width, int height);
	WindowManager() {}

	int DestroyWindow();

	void Start();

	int AddDrawable(Drawable *obj);
	int RemoveDrawable(Drawable *obj);

	int AddTimer(Callback *callback, int duration);

	void SetKeyboardCallback(Callback *callback);

	static LRESULT CALLBACK StaticWindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool IsRunning();

	bool Update();

	void ForceDraw();

	HWND GetHWND();

	~WindowManager();
private:
	HWND hWnd;
	HINSTANCE hInstance;
	HANDLE hThread;

	//std::vector<Drawable*> drawObjects;
	//Callback *keyCallback;

	std::string className, windowName;
	int x, y, width, height;
	unsigned int windowStyle;

	int MakeWindow();
	LRESULT WindowUpdate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};