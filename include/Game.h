#pragma once
#include <SFML/Graphics.hpp>
#include "FlappyAI.h"
#include "Menu.h"

class Game {
public:
    Game();
    void run();

private:
    enum class State { MainMenu, Training, Watching };

    sf::RenderWindow m_window;
    sf::Font m_font;
    bool m_fontLoaded = false;

    State m_state = State::MainMenu;
    Menu m_menu;
    FlappyAI m_sim;

    void processEvents();
    void update(float dt);
    void render();

    void handleMenuSelection(Menu::Option option);
    bool bestModelExists() const;
};
