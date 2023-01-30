#include <Windows.h>
#include <iostream>
#include "Helpers.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <detours.h>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours.lib")

typedef HRESULT(WINAPI* EndScene)(IDirect3DDevice9*);
typedef HRESULT(WINAPI* Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
EndScene oEndScene = NULL;
Reset oReset = NULL;
WNDPROC oWndProc;
HWND window = NULL;


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall hkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hwnd, msg, wParam, lParam);
}


HRESULT WINAPI hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPARAMETERS)
{
    std::cout << "Reset" << std::endl;
    return oReset(pDevice, pPARAMETERS);
}

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX9_Init(pDevice);
}

bool init = false;
HRESULT WINAPI hkEndScene(IDirect3DDevice9* pDevice)
{
    //std::cout << "Hooked" << std::endl;
    
    if (!init)
    {
        InitImGui(pDevice);
        oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)hkWndProc);
        init = true;
    }


    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Yuki", nullptr, ImGuiWindowFlags_NoSavedSettings);
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

void InitD3D()
{
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D)
        return;

    window = FindWindow("Direct3DWindowClass", NULL);


    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync


    IDirect3DDevice9* pDevice = nullptr;

    HRESULT result = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dpp, &pDevice);
    if (FAILED(result) || !pDevice)
    {
        pD3D->Release();
        return;
    }

    void** vTable = *reinterpret_cast<void***>(pDevice);

    
    oReset = (Reset)(vTable[16]);
    oEndScene = (EndScene)(vTable[42]);
    
    Helpers::HookFunction((PVOID*)(&oEndScene), hkEndScene);
    Helpers::HookFunction((PVOID*)(&oReset), hkReset);

    pDevice->Release();
    pD3D->Release();
    
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    //AllocConsole();
    //FILE* f;
    //freopen_s(&f, "CONOUT$", "w", stdout);
    
    InitD3D();

    while (!GetAsyncKeyState(VK_END))
    {
        Sleep(200);
    }
    Helpers::UnHookFunction((PVOID*)(&oEndScene), hkEndScene);
    Helpers::UnHookFunction((PVOID*)(&oReset), hkReset);

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)oWndProc);
    //fclose(f);
    //FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  dwReason,
                       LPVOID lpReserved
                     )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, 0);
    }

    return TRUE;
}

