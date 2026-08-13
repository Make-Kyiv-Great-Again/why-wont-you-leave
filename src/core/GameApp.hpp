#pragma once
#include <raylib-cpp.hpp>

class GameApp {
public:
    GameApp(int width, int height, const char* title);
    ~GameApp() = default;

    void Run();
    void UpdateDrawFrame();

private:
    raylib::Window window;
};
