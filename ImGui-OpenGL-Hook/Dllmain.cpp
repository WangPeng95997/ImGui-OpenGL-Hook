#include "GuiWindow.h"
#include "MinHook/include/MinHook.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(WINAPI* wglSwapBuffers)(HDC hdc);
HRESULT WINAPI Hook_wglSwapBuffers(HDC hdc, UINT unnamedParam);
LRESULT WINAPI Hook_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

wglSwapBuffers  Original_wglSwapBuffers;
WNDPROC         Original_WndProc;
HMODULE         g_hInstance;
HANDLE          g_hEndEvent;
GuiWindow*      g_GuiWindow;

void InitHook()
{
    MH_Initialize();

    // wglSwapBuffers
    LPVOID lpTarget = ::GetProcAddress(::GetModuleHandle("opengl32.dll"), "wglSwapBuffers");
    MH_CreateHook(lpTarget, &Hook_wglSwapBuffers, (LPVOID*)&Original_wglSwapBuffers);
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
            g_GuiWindow->showMenu = !g_GuiWindow->showMenu;
        break;

    case WM_DESTROY:
        ReleaseHook();
        break;
    }

    if (g_GuiWindow->showMenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return ::CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);
}

inline void InitImGui()
{
    ImGui::CreateContext();

    ImFontConfig fontConfig{};
    fontConfig.GlyphOffset.y = -1.75f;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(g_GuiWindow->fontPath.c_str(), FONT_SIZE, &fontConfig);
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.WindowRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 4.0f;
    style.WindowPadding = ImVec2(10.0f, 5.0f);
    style.FramePadding = ImVec2(0.0f, 0.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 0.0f;
    style.GrabMinSize = 10.0f;
    style.ButtonTextAlign = ImVec2(0.5f, 0.50f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.56f, 1.00f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.56f, 1.00f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.56f, 1.00f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    ImGui_ImplWin32_Init(g_GuiWindow->hWnd);
    ImGui_ImplOpenGL3_Init();
    Original_WndProc = (WNDPROC)::SetWindowLongPtr(g_GuiWindow->hWnd, GWLP_WNDPROC, (LONG_PTR)Hook_WndProc);
}

HRESULT WINAPI Hook_wglSwapBuffers(HDC hdc, UINT unnamedParam)
{
    static bool imGuiInitialized = false;

    if (!imGuiInitialized)
    {
        InitImGui();
        imGuiInitialized = true;
    }
    else if (g_GuiWindow->uiStatus & static_cast<DWORD>(GuiWindow::GuiState::GuiState_Detach))
    {
        ReleaseHook();
        return Original_wglSwapBuffers(hdc);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (g_GuiWindow->showMenu) {
        ImGui::ShowDemoWindow();
        //g_GuiWindow->Update();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return Original_wglSwapBuffers(hdc);
}

DWORD WINAPI ThreadEntry(LPVOID lpParameter)
{
    g_hInstance = (HMODULE)lpParameter;
    g_hEndEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    g_GuiWindow = new GuiWindow();
    g_GuiWindow->Initialize();

    InitHook();

    if (g_hEndEvent)
        ::WaitForSingleObject(g_hEndEvent, INFINITE);
    delete g_GuiWindow;
    ::FreeLibraryAndExitThread(g_hInstance, EXIT_SUCCESS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (::GetModuleHandle("opengl32.dll") == NULL)
            return false;

        ::DisableThreadLibraryCalls(hModule);
        ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadEntry, hModule, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        ::Sleep(100);
        MH_Uninitialize();
        break;
    }

    return true;
}