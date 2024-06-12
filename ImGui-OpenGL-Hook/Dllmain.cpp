#include "GuiWindow.h"
#include "MinHook/include/MinHook.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(WINAPI* wglSwapBuffers)(HDC hdc);
HRESULT WINAPI Hook_wglSwapBuffers(HDC hdc);
LRESULT WINAPI Hook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

wglSwapBuffers Original_wglSwapBuffers;
WNDPROC Original_WndProc;
HMODULE g_hInstance;
HANDLE g_hEndEvent;
GuiWindow* g_GuiWindow;

void InitHook()
{
    MH_Initialize();

    // wglSwapLayerBuffers
    LPVOID lpTarget = ::GetProcAddress(::GetModuleHandle("opengl32.dll"), "wglSwapBuffers");
    MH_CreateHook(lpTarget, &Hook_wglSwapBuffers, (void**)&Original_wglSwapBuffers);
    MH_EnableHook(lpTarget);
}

void ReleaseHook()
{
    ::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Original_WndProc);
    MH_DisableHook(MH_ALL_HOOKS);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ::SetEvent(g_hEndEvent);
}

LRESULT WINAPI Hook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_INSERT)
            g_GuiWindow->bShowMenu = !g_GuiWindow->bShowMenu;
        break;

    case WM_DESTROY:
        ReleaseHook();
        break;
    }

    if (g_GuiWindow->bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return ::CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);
}

inline static void InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->AddFontFromFileTTF(g_GuiWindow->FontPath, 20.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ButtonTextAlign.y = 0.46f;
    style.WindowBorderSize = 0.0f;
    style.WindowRounding = 0.0f;
    style.WindowPadding.x = 0.0f;
    style.WindowPadding.y = 0.0f;
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding.x = 0.0f;
    style.FramePadding.y = 0.0f;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.GrabRounding = 0.0f;
    style.GrabMinSize = 8.0f;
    style.PopupBorderSize = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.TabRounding = 0.0f;
    style.DisplaySafeAreaPadding.x = 0.0f;
    style.DisplaySafeAreaPadding.y = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImColor(0, 74, 122, 100).Value;
    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(0, 74, 122, 175).Value;
    style.Colors[ImGuiCol_FrameBgActive] = ImColor(0, 74, 122, 255).Value;
    style.Colors[ImGuiCol_TitleBg] = ImColor(0, 74, 122, 255).Value;
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(0, 74, 122, 255).Value;

    ImGui_ImplWin32_Init(g_GuiWindow->hWnd);
    ImGui_ImplOpenGL3_Init();
    Original_WndProc = (WNDPROC)::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Hook_WndProc);
}

HRESULT WINAPI Hook_wglSwapBuffers(HDC hdc)
{
    static bool bImGuiInit = false;

    if (!bImGuiInit)
    {
        InitImGui();
        bImGuiInit = true;
    }
    else if (g_GuiWindow->UIStatus & GuiWindow::Detach)
    {
        ReleaseHook();
        return Original_wglSwapBuffers(hdc);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    //g_GuiWindow->Update();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return Original_wglSwapBuffers(hdc);
}

DWORD WINAPI Start(LPVOID lpParameter)
{
    g_hInstance = (HMODULE)lpParameter;
    g_hEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    g_GuiWindow = new GuiWindow();
    g_GuiWindow->Init();

    InitHook();

    if (g_hEndEvent)
        ::WaitForSingleObject(g_hEndEvent, INFINITE);
    ::FreeLibraryAndExitThread(g_hInstance, EXIT_SUCCESS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (!::GetModuleHandle("opengl32.dll"))
            return false;

        ::DisableThreadLibraryCalls(hModule);
        ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Start, hModule, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        Sleep(100);
        MH_Uninitialize();
        break;
    }

    return true;
}