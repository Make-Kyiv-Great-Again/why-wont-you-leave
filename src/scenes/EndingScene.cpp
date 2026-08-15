#include "scenes/EndingScene.hpp"
#include "core/SceneManager.hpp"
#include "core/ResourceManager.hpp"
#include "core/ActManager.hpp"
#include "scenes/MainMenuScene.hpp"
#include <sstream>
#include <cmath>

EndingScene::EndingScene(bool hasAllTrueArtifacts, bool isLeaveChoice)
    : hasAllTrue(hasAllTrueArtifacts), isLeave(isLeaveChoice) {

    Color graceColor = Color{ 120, 160, 240, 255 }; // Grace soft spirit blue
    Color evanColor  = Color{ 220, 180, 110, 255 }; // Evan warm gold
    Color narrColor  = Color{ 200, 200, 200, 255 }; // Epilogue soft gray

    if (!hasAllTrue) {
        if (!isLeave) {
            // Ending 1: Imperfect + Stay (Coma in Denial)
            lines.push_back({ "Grace", "I'm sorry... I wanted you to live on.", graceColor });
            lines.push_back({ "", "After the devastating accident, Grace passed away, and Evan never woke up from his coma.", narrColor });
        } else {
            // Ending 2: Imperfect + Leave (Haunting Regret)
            lines.push_back({ "Grace", "I'm sorry... I wanted you to live on.", graceColor });
            lines.push_back({ "Evan", "I will never forget you, Grace.", evanColor });
            lines.push_back({ "", "After the devastating accident, Evan lost the love of his life, but survived. Sadly, he could never let go of the pain, living the rest of his days trapped in haunting memories and regret.", narrColor });
        }
    } else {
        if (!isLeave) {
            // Ending 3: Perfect Truth + Stay (Eternal Dream)
            lines.push_back({ "Grace", "I'm sorry... I wanted you to live on.", graceColor });
            lines.push_back({ "Evan", "I don't want to lose you... It feels so peaceful here. I want to stay here with you forever.", evanColor });
            lines.push_back({ "", "Evan never woke up from his coma, but in his eternal dreams, he remained by the side of the one he loved.", narrColor });
        } else {
            // Ending 4: Perfect Truth + Leave (True Acceptance / Peaceful Farewell)
            lines.push_back({ "Grace", "I'm so glad you were able to move on. I'm truly sorry we never got to say goodbye. I love you.", graceColor });
            lines.push_back({ "Evan", "I'm grateful you were always by my side. I will never forget you.", evanColor });
            lines.push_back({ "", "After the devastating accident, Evan lost the love of his life, but recovered and found the strength to live on. And sometimes, Grace still visits him in warm, peaceful dreams.", narrColor });
        }
    }

    whiteAlpha = 0.0f;
    fadeToBlack = 0.0f;
    creditsAlpha = 0.0f;
    inCreditsPhase = false;
    currentLineIndex = 0;
    displayedCharCount = 0.0f;
    isLineComplete = false;

    // Stop gameplay background music
    Music gameMusic = ResourceManager::Get().GetMusic("assets/sounds/game_sound.mp3");
    if (gameMusic.ctxData != nullptr && IsMusicStreamPlaying(gameMusic)) {
        StopMusicStream(gameMusic);
    }
}

void EndingScene::Update(float dt) {
    // 1. Initial White In
    if (whiteAlpha < 1.0f) {
        whiteAlpha += dt * 1.0f;
        if (whiteAlpha > 1.0f) whiteAlpha = 1.0f;
    }

    if (!inCreditsPhase) {
        if (currentLineIndex < lines.size()) {
            const auto& cur = lines[currentLineIndex];
            size_t totalLen = cur.text.length();

            if (displayedCharCount < (float)totalLen) {
                float prevCount = displayedCharCount;
                displayedCharCount += typingSpeed * dt;
                if (displayedCharCount > (float)totalLen) {
                    displayedCharCount = (float)totalLen;
                    isLineComplete = true;
                }

                if ((int)displayedCharCount > (int)prevCount && (int)displayedCharCount % 2 == 0) {
                    Sound sfx = ResourceManager::Get().GetSound("assets/sounds/ui_typing_sound.mp3");
                    if (sfx.frameCount > 0) PlaySound(sfx);
                }
            } else {
                isLineComplete = true;
            }

            bool advance = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || 
                           IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (advance) {
                if (!isLineComplete) {
                    // Instant complete
                    displayedCharCount = (float)totalLen;
                    isLineComplete = true;
                } else {
                    // Move to next line
                    currentLineIndex++;
                    displayedCharCount = 0.0f;
                    isLineComplete = false;

                    Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
                    if (sfx.frameCount > 0) PlaySound(sfx);
                }
            }
        } else {
            // Dialogue completed -> Start fade to black
            fadeToBlack += dt * 0.8f;
            if (fadeToBlack >= 1.0f) {
                fadeToBlack = 1.0f;
                inCreditsPhase = true;
            }
        }
    } else {
        // Credits phase
        if (creditsAlpha < 1.0f) {
            creditsAlpha += dt * 0.8f;
            if (creditsAlpha > 1.0f) creditsAlpha = 1.0f;
        }

        bool returnToMenu = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || 
                            IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (returnToMenu && creditsAlpha >= 0.5f) {
            Sound sfx = ResourceManager::Get().GetSound("assets/sounds/select_sound.mp3");
            if (sfx.frameCount > 0) PlaySound(sfx);

            ActManager::Get().Reset();
            SceneManager::Get().ClearSavedGameplayScene();
            SceneManager::Get().ChangeScene(std::make_unique<MainMenuScene>());
        }
    }
}

void EndingScene::Draw() {
    float screenWidth = 2000.0f;
    float screenHeight = 800.0f;

    if (!inCreditsPhase) {
        // 1. Brilliant White / Ethereal Light Background
        ClearBackground(WHITE);

        // Soft vignette / gradient over white for readability
        DrawRectangleGradientV(0, 0, (int)screenWidth, (int)screenHeight, Fade(WHITE, whiteAlpha), Fade(Color{ 235, 238, 245, 255 }, whiteAlpha));

        // Dialogue Box / Cinematic Text on White
        if (currentLineIndex < lines.size() && whiteAlpha >= 0.3f) {
            const auto& line = lines[currentLineIndex];

            // Elegant dark card for crisp text readability
            float boxW = 1480.0f;
            float boxH = 300.0f;
            float boxX = (screenWidth - boxW) / 2.0f;
            float boxY = (screenHeight - boxH) / 2.0f;

            DrawRectangleRec(Rectangle{ boxX, boxY, boxW, boxH }, Fade(Color{ 18, 18, 24, 255 }, 0.90f));
            DrawRectangleLinesEx(Rectangle{ boxX, boxY, boxW, boxH }, 2.5f, Fade(Color{ 180, 185, 200, 255 }, 0.65f));

            // Speaker Title
            if (!line.speaker.empty()) {
                ResourceManager::DrawGameText(line.speaker.c_str(), (int)boxX + 50, (int)boxY + 32, 30, line.speakerColor);
            }

            // Word-wrapped Typewriter Text
            std::string visibleText = line.text.substr(0, (size_t)displayedCharCount);
            int textY = line.speaker.empty() ? (int)boxY + 50 : (int)boxY + 80;
            int textFontSize = 26;
            int maxLineWidth = (int)boxW - 100;
            int lineSpacing = (int)(textFontSize * 1.45f);

            // Word wrap rendering
            std::istringstream words(visibleText);
            std::string word;
            std::string currentLineStr = "";
            int currentY = textY;

            while (words >> word) {
                std::string testLine = currentLineStr.empty() ? word : (currentLineStr + " " + word);
                int lineWidth = ResourceManager::MeasureGameText(testLine.c_str(), textFontSize);

                if (lineWidth > maxLineWidth && !currentLineStr.empty()) {
                    ResourceManager::DrawGameText(currentLineStr.c_str(), (int)boxX + 50, currentY, textFontSize, WHITE);
                    currentLineStr = word;
                    currentY += lineSpacing;
                } else {
                    currentLineStr = testLine;
                }
            }

            if (!currentLineStr.empty()) {
                ResourceManager::DrawGameText(currentLineStr.c_str(), (int)boxX + 50, currentY, textFontSize, WHITE);
            }

            // Continue prompt
            if (isLineComplete) {
                float time = (float)GetTime();
                float blink = sinf(time * 5.0f) * 0.5f + 0.5f;
                const char* continueHint = "[ Press Space to Continue ▶ ]";
                int chw = ResourceManager::MeasureGameText(continueHint, 18);
                ResourceManager::DrawGameText(continueHint, (int)(boxX + boxW - chw - 40), (int)(boxY + boxH - 34), 18, Fade(GOLD, blink));
            }
        }

        // Fade to Black Transition
        if (fadeToBlack > 0.0f) {
            DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, Fade(BLACK, fadeToBlack));
        }
    } else {
        // 2. Pitch Black Credits Screen
        ClearBackground(BLACK);

        Texture2D authorsTex = ResourceManager::Get().GetTexture("assets/sprites/authors.png");
        if (authorsTex.id == 0) {
            authorsTex = ResourceManager::Get().GetTexture("assets/sprites/Authors.svg");
        }

        if (authorsTex.id != 0) {
            float maxW = 1400.0f;
            float maxH = 600.0f;
            float scale = fminf(maxW / (float)authorsTex.width, maxH / (float)authorsTex.height);
            float dw = (float)authorsTex.width * scale;
            float dh = (float)authorsTex.height * scale;
            Rectangle destRect = {
                (screenWidth - dw) / 2.0f,
                (screenHeight - dh) / 2.0f - 25.0f,
                dw,
                dh
            };
            DrawTexturePro(
                authorsTex,
                Rectangle{ 0, 0, (float)authorsTex.width, (float)authorsTex.height },
                destRect,
                Vector2{ 0, 0 },
                0.0f,
                Fade(WHITE, creditsAlpha)
            );
        }

        // Return to Main Menu Prompt
        if (creditsAlpha >= 0.6f) {
            float time = (float)GetTime();
            float blink = sinf(time * 3.5f) * 0.4f + 0.6f;
            const char* menuPrompt = "Press [Space] to Return to Main Menu";
            int mw = ResourceManager::MeasureGameText(menuPrompt, 24);
            ResourceManager::DrawGameText(menuPrompt, (int)(screenWidth - mw) / 2, (int)screenHeight - 75, 24, Fade(LIGHTGRAY, creditsAlpha * blink));
        }
    }
}
