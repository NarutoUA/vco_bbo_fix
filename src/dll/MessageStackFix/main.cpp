#include "pch.h"

#include <mutex>

#include <SimpleIni.h>
#include <engine.h>

CORE* g_pCore = nullptr;
WNDPROC g_origWndProc = nullptr;

BOOL g_bIgnore_WM_NCHITTEST = TRUE;
UINT g_WM_MOUSEMOVE_rate = 30;

class MessageStackFix : public VMA
{
public:
    bool Service() override { return false; };
    const char* GetName() override { return "MessageStackFix"; }
    void* CreateClass() override { return nullptr; }
    void RefDec() override { nReference--; };
    long GetReference() override { return nReference; }
    void Clear() override { nReference = 0; };
    bool ScriptLibriary() override { return false; }
} g_MessageStackFix;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == WM_NCHITTEST && g_bIgnore_WM_NCHITTEST)
        return DefWindowProc(hWnd, Msg, wParam, lParam);

    if (Msg == WM_MOUSEMOVE && g_WM_MOUSEMOVE_rate)
    {
        static auto last_time = std::chrono::high_resolution_clock::now();
        static auto rate = std::chrono::duration_cast<decltype(last_time)::clock::duration>(std::chrono::seconds(1)) / g_WM_MOUSEMOVE_rate;

        auto elapsed_time = decltype(last_time)::clock::now() - last_time;

	    if (elapsed_time < rate)
        {
            return DefWindowProc(hWnd, Msg, wParam, lParam);
	    }

        last_time = decltype(last_time)::clock::now();

        g_origWndProc(hWnd, Msg, wParam, lParam);
    }

    return g_origWndProc(hWnd, Msg, wParam, lParam);
}

VMA* DMAInterface(VAPI* api, VSYSTEM_API* sys_api)
{
    static std::once_flag flag;

    std::call_once(flag, [](VAPI* api)
        {
            CSimpleIni ini;
            if (ini.LoadFile(L"vco_bbo_fix.ini") == SI_OK)
            {
                g_bIgnore_WM_NCHITTEST = ini.GetLongValue(L"MessageStackFix", L"ignore_hittest_event", g_bIgnore_WM_NCHITTEST);
                g_WM_MOUSEMOVE_rate = ini.GetLongValue(L"MessageStackFix", L"mousemove_rate", g_WM_MOUSEMOVE_rate);
                OutputDebugString(L"Loaded settings from fix ini");
            }
            else
            {
                OutputDebugString(L"Failed ot load fix ini");
            }

            g_pCore = reinterpret_cast<CORE*>(api);
            g_origWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_pCore->App_Hwnd, GWL_WNDPROC, (LONG_PTR)WndProc));
        }, api);

    return &g_MessageStackFix;
}
