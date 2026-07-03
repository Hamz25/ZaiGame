#include "Game.h"
#include "Constants.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace Constants;

Game::Game()
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ZaiGame - Flappy AI",
            sf::Style::Titlebar | sf::Style::Close)
{
    m_window.setFramerateLimit(60);

    // Make sure the folder we save trained brains into actually exists -
    // std::ofstream will silently fail to write if it doesn't.
    std::filesystem::create_directories("saves");

    m_fontLoaded = m_font.loadFromFile(FONT_PATH);
    if (!m_fontLoaded) {
        std::cerr << "[ZaiGame] Warning: could not load font at \"" << FONT_PATH
                << "\". Text will not render. Place a .ttf there (e.g. a copy of\n"
                << "DejaVuSans.ttf or arial.ttf) to see the HUD and menu text.\n";
    }
}

bool Game::bestModelExists() const {
    std::ifstream f(BEST_MODEL_PATH);
    return f.good();
}

void Game::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 1.f / 20.f); // avoid huge jumps if the window was dragged/paused

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event{};
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_window.close();
            continue;
        }

        if (m_state == State::MainMenu) {
            Menu::Option choice = m_menu.handleEvent(event);
            if (choice != Menu::Option::None)
                handleMenuSelection(choice);
            continue;
        }

        // Training / Watching states
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Escape:
                    m_state = State::MainMenu;
                    break;
                case sf::Keyboard::Add:
                case sf::Keyboard::Equal:
                    m_sim.setSimSpeed(m_sim.simSpeed() * 2.f);
                    break;
                case sf::Keyboard::Subtract:
                case sf::Keyboard::Hyphen:
                    m_sim.setSimSpeed(m_sim.simSpeed() / 2.f);
                    break;
                case sf::Keyboard::S: {
                    bool ok = m_sim.saveBestBrain(BEST_MODEL_PATH);
                    std::cout << (ok ? "[ZaiGame] Best brain saved to " : "[ZaiGame] Save failed: ")
                            << BEST_MODEL_PATH << "\n";
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void Game::handleMenuSelection(Menu::Option option) {
    switch (option) {
        case Menu::Option::StartTraining:
            m_sim.startTraining();
            m_state = State::Training;
            break;
        case Menu::Option::WatchBest:
            if (m_sim.startWatching(BEST_MODEL_PATH))
                m_state = State::Watching;
            // if it fails (no saved model), just stay on the menu
            break;
        case Menu::Option::Quit:
            m_window.close();
            break;
        default:
            break;
    }
}

void Game::update(float dt) {
    if (m_state == State::Training || m_state == State::Watching)
        m_sim.update(dt, m_window);
}

void Game::render() {
    m_window.clear();

    if (m_state == State::MainMenu) {
        m_menu.render(m_window, m_font, bestModelExists());
    } else {
        m_sim.render(m_window, m_font);
    }

    m_window.display();
}
