#pragma once
#include "scenes/Scene.hpp"
#include "raylib.h"
#include <string>

class MainMenuScene : public Scene {
public:
    MainMenuScene();
    ~MainMenuScene() override = default;

    void Update(float dt) override;
    void Draw() override;

private:
    int selectedButtonIndex = 0;
    bool showOptionsPopup = false;

    Rectangle newGameBtnRect;
    Rectangle optionsBtnRect;
    Rectangle exitBtnRect;
};
