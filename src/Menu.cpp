#include "Menu.h"
#include "Constants.h"

using namespace Constants;

Menu::Menu() {
    m_labels = {
        "Start Training (new population)",
        "Watch Best Saved Brain",
        "Quit"
    };
}

Menu::Option Menu::handleEvent(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed)
        return Option::None;

    switch (event.key.code) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            m_selectedIndex = (m_selectedIndex + static_cast<int>(m_labels.size()) - 1) % static_cast<int>(m_labels.size());
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_labels.size());
            break;
        case sf::Keyboard::Enter:
        case sf::Keyboard::Space:
            return static_cast<Option>(m_selectedIndex);
        case sf::Keyboard::Num1:
            return Option::StartTraining;
        case sf::Keyboard::Num2:
            return Option::WatchBest;
        case sf::Keyboard::Num3:
        case sf::Keyboard::Escape:
            return Option::Quit;
        default:
            break;
    }
    return Option::None;
}

void Menu::render(sf::RenderWindow& window, const sf::Font& font, bool bestModelAvailable) const {
    sf::RectangleShape backdrop({ static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT) });
    backdrop.setFillColor(sf::Color(35, 42, 60));
    window.draw(backdrop);

    sf::Text title;
    title.setFont(font);
    title.setString("ZaiGame - Flappy AI");
    title.setCharacterSize(42);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 196, 30));
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.width / 2.f, 0.f);
    title.setPosition(WINDOW_WIDTH / 2.f, 90.f);
    window.draw(title);

    sf::Text subtitle;
    subtitle.setFont(font);
    subtitle.setString("A population of birds learns to fly through pipes via a genetic algorithm.");
    subtitle.setCharacterSize(15);
    subtitle.setFillColor(sf::Color(200, 200, 215));
    sf::FloatRect subBounds = subtitle.getLocalBounds();
    subtitle.setOrigin(subBounds.width / 2.f, 0.f);
    subtitle.setPosition(WINDOW_WIDTH / 2.f, 150.f);
    window.draw(subtitle);

    float startY = 240.f;
    for (size_t i = 0; i < m_labels.size(); ++i) {
        bool selected = (static_cast<int>(i) == m_selectedIndex);
        bool disabled = (static_cast<Option>(i) == Option::WatchBest && !bestModelAvailable);

        sf::Text item;
        item.setFont(font);
        std::string prefix = selected ? "> " : "  ";
        std::string label = prefix + std::to_string(i + 1) + ". " + m_labels[i];
        if (disabled) label += "  (no saved model yet)";
        item.setString(label);
        item.setCharacterSize(22);
        item.setFillColor(disabled ? sf::Color(120, 120, 130)
                                    : (selected ? sf::Color(255, 226, 120) : sf::Color::White));
        sf::FloatRect bounds = item.getLocalBounds();
        item.setOrigin(bounds.width / 2.f, 0.f);
        item.setPosition(WINDOW_WIDTH / 2.f, startY + i * 46.f);
        window.draw(item);
    }

    sf::Text hint;
    hint.setFont(font);
    hint.setString("Use Up/Down + Enter, or press 1 / 2 / 3");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(160, 160, 175));
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.width / 2.f, 0.f);
    hint.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 60.f);
    window.draw(hint);
}
