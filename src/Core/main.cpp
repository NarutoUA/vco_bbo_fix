#include "pch.h"

#include <wtsapi32.h>

#include <simpleini.h>
#include <wil/resource.h>

#include "resource.h"

#define CORE_INI L"core.ini"
#define CORE_DLL L"Core.dll"
#define CORE_CLASSNAME L"Core Windows"

LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	CSimpleIni ini;
	if (ini.LoadFile(CORE_INI) != SI_OK)
	{
		OutputDebugString(L"Failed to load " CORE_INI);
		return EXIT_FAILURE;
	}
 
	wil::unique_hmodule hCore(LoadLibrary(CORE_DLL));

	if (hCore.is_valid() == false)
	{
		OutputDebugString(L"Failed to load " CORE_DLL);
		return EXIT_FAILURE;
	}
	
	auto _ST_EngineSetup = reinterpret_cast<int(_cdecl*)(HWND)>(GetProcAddress(hCore.get(), "ST_EngineSetup"));
	auto _ST_EngineFrame = reinterpret_cast<int(_cdecl*)()>(GetProcAddress(hCore.get(), "ST_EngineFrame"));
	auto _ST_EngineShutdown = reinterpret_cast<int(_cdecl*)()>(GetProcAddress(hCore.get(), "ST_EngineShutdown"));

	if (_ST_EngineSetup == nullptr || _ST_EngineFrame == nullptr || _ST_EngineShutdown == nullptr)
	{
		OutputDebugString(L"Failed to import methods from " CORE_DLL);
		return EXIT_FAILURE;
	}

	wchar_t empty_section = 0;
	auto pTitle = ini.GetValue(&empty_section, L"Title", L"VoyageCentury");

	WNDCLASSEX wndclass;

	auto hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VCO_ICON));

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(WORD);
	wndclass.hInstance = hInstance;
	wndclass.hIcon = hIcon;
	wndclass.hCursor = LoadCursor(0, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_PEN);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = CORE_CLASSNAME;
	wndclass.hIconSm = hIcon;
	if (RegisterClassEx(&wndclass) == NULL)
		return 0;

	auto bWindowMode = ini.GetLongValue(&empty_section, L"window", 1);
	auto screen_x = ini.GetLongValue(&empty_section, L"screen_x", 800);
	auto screen_y = ini.GetLongValue(&empty_section, L"screen_y", 600);

	auto width = screen_x;
	auto height = screen_y;

	RECT rcScreen;
	if (bWindowMode)
	{
		auto hDesktopWnd = GetDesktopWindow();
		GetClientRect(hDesktopWnd, &rcScreen);

		width = rcScreen.right - rcScreen.left;
		height = rcScreen.bottom - rcScreen.top;

		if (width <= screen_x || height <= screen_y)
		{
			ini.SetLongValue(&empty_section, L"screen_x", width);
			ini.SetLongValue(&empty_section, L"screen_y", height);
			ini.SaveFile(CORE_INI, false);
		}
		else
		{
			width = screen_x;
			height = screen_y;
		}
	}

	rcScreen.left = 0;
	rcScreen.top = 0;
	rcScreen.right = width;
	rcScreen.bottom = height;

	wil::unique_hwnd hWnd;

	if (bWindowMode)
	{
		AdjustWindowRect(&rcScreen, WS_CAPTION | WS_SYSMENU | WS_GROUP, FALSE);

		hWnd.reset(CreateWindowEx(0,
			CORE_CLASSNAME,
			pTitle,
			WS_CAPTION | WS_SYSMENU | WS_GROUP,
			(rcScreen.left + GetSystemMetrics(SM_CXFULLSCREEN) - rcScreen.right) / 2,
			(rcScreen.top + GetSystemMetrics(SM_CYFULLSCREEN) - rcScreen.bottom) / 2,
			rcScreen.right - rcScreen.left,
			rcScreen.bottom - rcScreen.top,
			0, 0, hInstance, 0));
	}
	else
	{
		hWnd.reset(CreateWindowEx(0,
			CORE_CLASSNAME,
			pTitle,
			WS_POPUP,
			0,
			0,
			rcScreen.right - rcScreen.left,
			rcScreen.bottom - rcScreen.top,
			0, 0, hInstance, 0));
	}

	if (hWnd.is_valid() == false)
	{
		OutputDebugString(L"Failed to create main window");
		return EXIT_FAILURE;
	}

	_ST_EngineSetup(hWnd.get());

	ShowWindow(hWnd.get(), SW_SHOW);
	UpdateWindow(hWnd.get());

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	CSimpleIni core_fix_ini;

	auto bUsePeekMessageLoop = false;
	auto bRenderEachLoopIter = true;
	auto dwLoopSleep = 8;
	
	if (core_fix_ini.LoadFile(L"vco_bbo_fix.ini") == SI_OK)
	{
		bUsePeekMessageLoop = core_fix_ini.GetLongValue(L"Core", L"use_peekmessage_loop", bUsePeekMessageLoop);
		bRenderEachLoopIter = core_fix_ini.GetLongValue(L"Core", L"render_each_loop_iter", bRenderEachLoopIter);
		dwLoopSleep = core_fix_ini.GetLongValue(L"Core", L"loop_sleep_ms", dwLoopSleep);
		OutputDebugString(L"Loaded settings from fix ini");
	}
	else
	{
		OutputDebugString(L"Failed to load settings from fix ini");
	}

	auto _processMessage = [](MSG& msg, HWND hWnd)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
		else if (msg.message == WM_SETFOCUS || msg.message == WM_CAPTURECHANGED)
			SetCapture(hWnd);

		return true;
	};

	while (true)
	{
		bool bGotMessage = false;

		if (bUsePeekMessageLoop)
		{
			bool bExitLoop = false;
			while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				bGotMessage = true;

				bExitLoop = !_processMessage(msg, hWnd.get());
				if (bExitLoop)
					break;
			}

			if (bExitLoop)
				break;
		}
		else
		{
			if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
			{
				bGotMessage = true;
				if (!_processMessage(msg, hWnd.get()))
					break;
			}
		}

		if (bRenderEachLoopIter || bGotMessage == false)
		{
			if (_ST_EngineFrame() == 0)
				break;
		}
			

		if (dwLoopSleep)
			Sleep(dwLoopSleep);
	}

	_ST_EngineShutdown();
	WTSRegisterSessionNotification(hWnd.get(), 0);

	// Looks like Core.dll destroys window itself so release ownership
	hWnd.release();

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_SYSKEYUP && (wParam == WM_QUIT || wParam == VK_F10))
		return 0;

	if (Msg == WM_DESTROY)
	{
		WTSUnRegisterSessionNotification(hWnd);
		PostQuitMessage(0);
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}