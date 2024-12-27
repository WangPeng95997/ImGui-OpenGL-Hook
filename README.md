# ImGui-OpenGL-Hook

## Getting Started
[Dllmain.cpp](https://github.com/WangPeng95997/ImGui-OpenGL-Hook/blob/master/ImGui-OpenGL-Hook/Dllmain.cpp#L162-L163)
```C++
ImGui::ShowDemoWindow();
//g_GuiWindow->Update();
```
[GuiWindow.cpp](https://github.com/WangPeng95997/ImGui-OpenGL-Hook/blob/master/ImGui-OpenGL-Hook/GuiWindow.cpp#L116-L119)
```C++
// Insert your custom ImGui code here
ImGui::Text("Hello World");
ImGui::Checkbox("Hello World##0", (bool*)this->lpBuffer);
ImGui::Button("Hello World##1", ImVec2(100.0f, 20.0f));
```

## Screenshot
![Image](https://github.com/WangPeng95997/ImGui-OpenGL-Hook/blob/master/Screenshot/Image.png)

## Credits
* ImGui   https://github.com/ocornut/imgui
* kiero https://github.com/Rebzzel/kiero
* MinHook https://github.com/TsudaKageyu/minhook
* UniversalHookX https://github.com/bruhmoment21/UniversalHookX