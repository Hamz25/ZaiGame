#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// A small keyboard-driven main menu. Not a separate application state
// machine of its own - Game owns the state, Menu just draws options and
// reports which one is selected.
class Menu {
public:
    enum class Option {
        StartTraining,
        WatchBest,
        Quit,
        None
    };

    Menu();

    // Call once per polled event; returns an Option if the user just
    // confirmed a choice (Enter/Space), otherwise Option::None.
    Option handleEvent(const sf::Event& event);

    void render(sf::RenderWindow& window, const sf::Font& font, bool bestModelAvailable) const;

private:
    std::vector<std::string> m_labels;
    int m_selectedIndex = 0;
};
