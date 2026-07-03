#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "NeuralNetwork.h"

// Runs a population of birds through a genetic algorithm on a Flappy-Bird
// style obstacle course, and can also "replay" a single saved brain in
// watch mode. Owns no window - it draws into whatever RenderWindow /
// Font it's given, so it can share a window with a main menu.
class FlappyAI {
public:
    struct Bird {
        float y = 0.f;
        float velocity = 0.f;
        bool  alive = true;

        int   score = 0;      // pipes passed
        float timeAlive = 0.f;
        float fitness = 0.f;

        NeuralNetwork brain;

        Bird();
    };

    struct Pipe {
        float x = 0.f;
        float gapY = 0.f; // vertical center of the gap
        bool  scored = false;
    };

    enum class Mode { Training, Watching };

    FlappyAI();

    // (Re)starts a fresh training run with a brand-new random population.
    void startTraining();

    // Loads a saved brain and watches it play solo, respawning forever.
    // Returns false if the file couldn't be read.
    bool startWatching(const std::string& path);

    void update(float dt, sf::RenderWindow& window);
    void render(sf::RenderWindow& window, const sf::Font& font);

    // Manually saves whichever brain is currently the best-performing.
    bool saveBestBrain(const std::string& path) const;

    int   generation() const { return m_generation; }
    int   aliveCount() const { return m_aliveCount; }
    int   bestScoreEver() const { return m_bestScoreEver; }
    float simSpeed() const { return m_simSpeed; }
    void  setSimSpeed(float speed);

private:
    Mode m_mode = Mode::Training;

    std::vector<Bird> m_population;
    std::vector<Pipe> m_pipes;

    int   m_generation = 1;
    int   m_aliveCount = 0;
    int   m_bestScoreEver = 0;
    float m_pipeSpawnTimer = 0.f;
    float m_simSpeed = 1.f; // fast-forward multiplier for training

    NeuralNetwork m_bestBrainEver; // best genome found so far, kept for saving
    float m_bestFitnessEver = 0.f;
    int   m_leaderIndex = -1; // index of current best-performing bird, for rendering

    void resetPipes();
    void spawnPipe();
    void stepSimulation(float fixedDt);
    void updateBird(Bird& bird, float dt);
    bool collides(const Bird& bird, const Pipe& pipe) const;
    std::vector<float> gatherInputs(const Bird& bird) const;
    const Pipe* nextPipeFor(const Bird& bird) const;

    void naturalSelection();
    void drawBackground(sf::RenderWindow& window) const;
    void drawPipe(sf::RenderWindow& window, const Pipe& pipe) const;
    void drawBird(sf::RenderWindow& window, const Bird& bird, bool highlight) const;
    void drawHud(sf::RenderWindow& window, const sf::Font& font) const;
};
