#include "FlappyAI.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <random>

using namespace Constants;

namespace {
    std::mt19937& localRng() {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    float randRange(float lo, float hi) {
        std::uniform_real_distribution<float> dist(lo, hi);
        return dist(localRng());
    }

    // Closest-point circle vs. axis-aligned-rect overlap test.
    bool circleRectOverlap(float cx, float cy, float r,
                            float rx, float ry, float rw, float rh) {
        float closestX = std::clamp(cx, rx, rx + rw);
        float closestY = std::clamp(cy, ry, ry + rh);
        float dx = cx - closestX;
        float dy = cy - closestY;
        return (dx * dx + dy * dy) < (r * r);
    }
}

FlappyAI::Bird::Bird() : brain({ NN_INPUTS, NN_HIDDEN, NN_OUTPUTS }) {}

FlappyAI::FlappyAI() {
    startTraining();
}

// ---------------------------------------------------------------- lifecycle

void FlappyAI::startTraining() {
    m_mode = Mode::Training;
    m_population.assign(POPULATION_SIZE, Bird{});
    for (auto& bird : m_population)
        bird.y = WINDOW_HEIGHT / 2.f;

    m_generation = 1;
    m_aliveCount = static_cast<int>(m_population.size());
    m_leaderIndex = 0;
    resetPipes();
}

bool FlappyAI::startWatching(const std::string& path) {
    NeuralNetwork loaded;
    if (!loaded.loadFromFile(path))
        return false;

    m_mode = Mode::Watching;
    m_population.assign(1, Bird{});
    m_population[0].brain = std::move(loaded);
    m_population[0].y = WINDOW_HEIGHT / 2.f;

    m_aliveCount = 1;
    m_leaderIndex = 0;
    resetPipes();
    return true;
}

void FlappyAI::setSimSpeed(float speed) {
    m_simSpeed = std::clamp(speed, 1.f, 8.f);
}

// ------------------------------------------------------------------ update

void FlappyAI::update(float dt, sf::RenderWindow& /*window*/) {
    int steps = std::max(1, static_cast<int>(std::round(m_simSpeed)));
    for (int i = 0; i < steps; ++i) {
        stepSimulation(dt);

        if (m_mode == Mode::Training && m_aliveCount == 0) {
            naturalSelection();
        } else if (m_mode == Mode::Watching && m_aliveCount == 0) {
            // Respawn the single bird endlessly so you can watch it play.
            m_population[0].y = WINDOW_HEIGHT / 2.f;
            m_population[0].velocity = 0.f;
            m_population[0].alive = true;
            m_population[0].score = 0;
            m_population[0].timeAlive = 0.f;
            m_population[0].fitness = 0.f;
            m_aliveCount = 1;
            resetPipes();
        }
    }
}

void FlappyAI::resetPipes() {
    m_pipes.clear();
    m_pipeSpawnTimer = 0.f;
}

void FlappyAI::spawnPipe() {
    Pipe pipe;
    pipe.x = static_cast<float>(WINDOW_WIDTH) + PIPE_WIDTH;
    float gapHalf = PIPE_GAP / 2.f;
    float minCenter = PIPE_MIN_Y + gapHalf;
    float maxCenter = (WINDOW_HEIGHT - GROUND_HEIGHT) - PIPE_MIN_Y - gapHalf;
    pipe.gapY = randRange(minCenter, maxCenter);
    m_pipes.push_back(pipe);
}

void FlappyAI::stepSimulation(float dt) {
    // --- spawn / scroll pipes ---
    m_pipeSpawnTimer -= dt;
    if (m_pipeSpawnTimer <= 0.f) {
        spawnPipe();
        m_pipeSpawnTimer += PIPE_SPACING / SCROLL_SPEED;
    }

    for (auto& pipe : m_pipes)
        pipe.x -= SCROLL_SPEED * dt;

    m_pipes.erase(
        std::remove_if(m_pipes.begin(), m_pipes.end(),
            [](const Pipe& p) { return p.x + PIPE_WIDTH < -10.f; }),
        m_pipes.end());

    // --- score keeping: every alive bird shares the same x position, so a
    //     pipe is "passed" for everyone in the same frame ---
    for (auto& pipe : m_pipes) {
        if (!pipe.scored && pipe.x + PIPE_WIDTH < BIRD_X - BIRD_RADIUS) {
            pipe.scored = true;
            for (auto& bird : m_population)
                if (bird.alive)
                    bird.score++;
        }
    }

    // --- birds think & move ---
    for (auto& bird : m_population)
        if (bird.alive)
            updateBird(bird, dt);

    // --- collisions ---
    for (auto& bird : m_population) {
        if (!bird.alive) continue;

        if (bird.y - BIRD_RADIUS <= 0.f || bird.y + BIRD_RADIUS >= WINDOW_HEIGHT - GROUND_HEIGHT) {
            bird.alive = false;
            continue;
        }
        for (const auto& pipe : m_pipes) {
            if (collides(bird, pipe)) {
                bird.alive = false;
                break;
            }
        }
    }

    // --- bookkeeping ---
    m_aliveCount = 0;
    int leader = -1;
    float bestFitness = -1.f;
    for (int i = 0; i < static_cast<int>(m_population.size()); ++i) {
        auto& bird = m_population[i];
        if (bird.alive) {
            m_aliveCount++;
            if (bird.fitness > bestFitness) {
                bestFitness = bird.fitness;
                leader = i;
            }
        }
        m_bestScoreEver = std::max(m_bestScoreEver, bird.score);
    }
    if (leader != -1)
        m_leaderIndex = leader;
}

void FlappyAI::updateBird(Bird& bird, float dt) {
    std::vector<float> inputs = gatherInputs(bird);
    std::vector<float> out = bird.brain.feedForward(inputs);
    if (!out.empty() && out[0] > 0.5f)
        bird.velocity = FLAP_VELOCITY;

    bird.velocity += GRAVITY * dt;
    bird.velocity = std::min(bird.velocity, MAX_FALL_SPEED);
    bird.y += bird.velocity * dt;

    bird.timeAlive += dt;
    // Reward survival time a little, and getting through pipes a lot -
    // otherwise a bird that flies straight into the first gap looks the
    // same as one that dies instantly, which gives the GA nothing to select on.
    bird.fitness = bird.timeAlive + static_cast<float>(bird.score) * 70.f;
}

bool FlappyAI::collides(const Bird& bird, const Pipe& pipe) const {
    float gapHalf = PIPE_GAP / 2.f;
    float topRectH = pipe.gapY - gapHalf;
    float bottomRectY = pipe.gapY + gapHalf;
    float bottomRectH = (WINDOW_HEIGHT - GROUND_HEIGHT) - bottomRectY;

    if (topRectH > 0.f &&
        circleRectOverlap(BIRD_X, bird.y, BIRD_RADIUS, pipe.x, 0.f, PIPE_WIDTH, topRectH))
        return true;

    if (bottomRectH > 0.f &&
        circleRectOverlap(BIRD_X, bird.y, BIRD_RADIUS, pipe.x, bottomRectY, PIPE_WIDTH, bottomRectH))
        return true;

    return false;
}

const FlappyAI::Pipe* FlappyAI::nextPipeFor(const Bird& /*bird*/) const {
    for (const auto& pipe : m_pipes)
        if (pipe.x + PIPE_WIDTH > BIRD_X - BIRD_RADIUS)
            return &pipe;
    return nullptr;
}

std::vector<float> FlappyAI::gatherInputs(const Bird& bird) const {
    const Pipe* pipe = nextPipeFor(bird);
    if (!pipe)
        return std::vector<float>(NN_INPUTS, 0.f);

    float birdY   = bird.y / WINDOW_HEIGHT;
    float birdVel = std::clamp(bird.velocity / MAX_FALL_SPEED, -1.f, 1.f);
    float dx      = std::clamp((pipe->x - BIRD_X) / WINDOW_WIDTH, 0.f, 1.f);
    float gapTop  = (pipe->gapY - PIPE_GAP / 2.f) / WINDOW_HEIGHT;
    float gapBot  = (pipe->gapY + PIPE_GAP / 2.f) / WINDOW_HEIGHT;

    return { birdY, birdVel, dx, gapTop, gapBot };
}

// -------------------------------------------------------------- evolution

void FlappyAI::naturalSelection() {
    std::sort(m_population.begin(), m_population.end(),
        [](const Bird& a, const Bird& b) { return a.fitness > b.fitness; });

    if (!m_population.empty() && m_population.front().fitness > m_bestFitnessEver) {
        m_bestFitnessEver = m_population.front().fitness;
        m_bestBrainEver = m_population.front().brain.clone();
    }

    float totalFitness = 0.f;
    for (const auto& bird : m_population)
        totalFitness += bird.fitness + 1.f; // +1 so zero-fitness birds still have a shot

    auto pickParentBrain = [&]() -> const NeuralNetwork& {
        float pick = randRange(0.f, totalFitness);
        float running = 0.f;
        for (const auto& bird : m_population) {
            running += bird.fitness + 1.f;
            if (running >= pick)
                return bird.brain;
        }
        return m_population.back().brain;
    };

    std::vector<Bird> nextGen;
    nextGen.reserve(m_population.size());

    int eliteCount = std::min<int>(ELITE_COUNT, static_cast<int>(m_population.size()));
    for (int i = 0; i < eliteCount; ++i) {
        Bird elite;
        elite.brain = m_population[i].brain.clone();
        elite.y = WINDOW_HEIGHT / 2.f;
        nextGen.push_back(std::move(elite));
    }

    while (nextGen.size() < m_population.size()) {
        Bird child;
        child.brain = pickParentBrain().clone();
        child.brain.mutate(MUTATION_RATE, MUTATION_STRENGTH);
        child.y = WINDOW_HEIGHT / 2.f;
        nextGen.push_back(std::move(child));
    }

    m_population = std::move(nextGen);
    m_generation++;
    m_aliveCount = static_cast<int>(m_population.size());
    m_leaderIndex = 0;
    resetPipes();
}

bool FlappyAI::saveBestBrain(const std::string& path) const {
    if (m_bestBrainEver.isValid())
        return m_bestBrainEver.saveToFile(path);

    // Nothing has beaten the initial fitness yet (or we're in watch mode) -
    // fall back to whichever bird is currently alive and doing best.
    if (m_leaderIndex >= 0 && m_leaderIndex < static_cast<int>(m_population.size()))
        return m_population[m_leaderIndex].brain.saveToFile(path);

    return false;
}

// -------------------------------------------------------------- rendering

void FlappyAI::render(sf::RenderWindow& window, const sf::Font& font) {
    drawBackground(window);
    for (const auto& pipe : m_pipes)
        drawPipe(window, pipe);

    for (int i = 0; i < static_cast<int>(m_population.size()); ++i) {
        const auto& bird = m_population[i];
        if (bird.alive)
            drawBird(window, bird, i == m_leaderIndex);
    }

    drawHud(window, font);
}

void FlappyAI::drawBackground(sf::RenderWindow& window) const {
    sf::VertexArray sky(sf::Quads, 4);
    sf::Color top(105, 190, 230);
    sf::Color bottom(190, 230, 245);
    sky[0] = sf::Vertex({ 0.f, 0.f }, top);
    sky[1] = sf::Vertex({ static_cast<float>(WINDOW_WIDTH), 0.f }, top);
    sky[2] = sf::Vertex({ static_cast<float>(WINDOW_WIDTH), WINDOW_HEIGHT - GROUND_HEIGHT }, bottom);
    sky[3] = sf::Vertex({ 0.f, WINDOW_HEIGHT - GROUND_HEIGHT }, bottom);
    window.draw(sky);

    sf::RectangleShape ground({ static_cast<float>(WINDOW_WIDTH), GROUND_HEIGHT });
    ground.setPosition(0.f, WINDOW_HEIGHT - GROUND_HEIGHT);
    ground.setFillColor(sf::Color(222, 197, 118));
    window.draw(ground);

    sf::RectangleShape groundLine({ static_cast<float>(WINDOW_WIDTH), 4.f });
    groundLine.setPosition(0.f, WINDOW_HEIGHT - GROUND_HEIGHT);
    groundLine.setFillColor(sf::Color(94, 168, 74));
    window.draw(groundLine);
}

void FlappyAI::drawPipe(sf::RenderWindow& window, const Pipe& pipe) const {
    sf::Color body(94, 168, 74);
    sf::Color outline(60, 120, 50);
    float gapHalf = PIPE_GAP / 2.f;

    float topH = pipe.gapY - gapHalf;
    if (topH > 0.f) {
        sf::RectangleShape top({ PIPE_WIDTH, topH });
        top.setPosition(pipe.x, 0.f);
        top.setFillColor(body);
        top.setOutlineColor(outline);
        top.setOutlineThickness(-3.f);
        window.draw(top);

        sf::RectangleShape cap({ PIPE_WIDTH + 10.f, 22.f });
        cap.setPosition(pipe.x - 5.f, topH - 22.f);
        cap.setFillColor(body);
        cap.setOutlineColor(outline);
        cap.setOutlineThickness(-3.f);
        window.draw(cap);
    }

    float bottomY = pipe.gapY + gapHalf;
    float bottomH = (WINDOW_HEIGHT - GROUND_HEIGHT) - bottomY;
    if (bottomH > 0.f) {
        sf::RectangleShape bot({ PIPE_WIDTH, bottomH });
        bot.setPosition(pipe.x, bottomY);
        bot.setFillColor(body);
        bot.setOutlineColor(outline);
        bot.setOutlineThickness(-3.f);
        window.draw(bot);

        sf::RectangleShape cap({ PIPE_WIDTH + 10.f, 22.f });
        cap.setPosition(pipe.x - 5.f, bottomY);
        cap.setFillColor(body);
        cap.setOutlineColor(outline);
        cap.setOutlineThickness(-3.f);
        window.draw(cap);
    }
}

void FlappyAI::drawBird(sf::RenderWindow& window, const Bird& bird, bool highlight) const {
    float angle = std::clamp(bird.velocity / MAX_FALL_SPEED, -1.f, 1.f) * 40.f;

    sf::Transform t;
    t.translate(BIRD_X, bird.y);
    t.rotate(angle);

    sf::RenderStates states;
    states.transform = t;

    sf::CircleShape body(BIRD_RADIUS);
    body.setOrigin(BIRD_RADIUS, BIRD_RADIUS);
    body.setFillColor(highlight ? sf::Color(255, 196, 30) : sf::Color(255, 226, 120, 210));
    body.setOutlineColor(sf::Color(150, 100, 10));
    body.setOutlineThickness(highlight ? 2.5f : 1.f);
    window.draw(body, states);

    sf::ConvexShape beak(3);
    beak.setPoint(0, { BIRD_RADIUS - 2.f, -3.f });
    beak.setPoint(1, { BIRD_RADIUS + 8.f, 0.f });
    beak.setPoint(2, { BIRD_RADIUS - 2.f, 5.f });
    beak.setFillColor(sf::Color(230, 120, 30));
    window.draw(beak, states);

    sf::CircleShape eye(2.2f);
    eye.setOrigin(2.2f, 2.2f);
    eye.setPosition(4.f, -4.f);
    eye.setFillColor(sf::Color::Black);
    window.draw(eye, states);
}

void FlappyAI::drawHud(sf::RenderWindow& window, const sf::Font& font) const {
    sf::RectangleShape panel({ 250.f, 118.f });
    panel.setPosition(10.f, 10.f);
    panel.setFillColor(sf::Color(20, 20, 30, 150));
    window.draw(panel);

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);

    std::string modeStr = (m_mode == Mode::Training) ? "Training" : "Watching";
    std::string body =
        "Mode: " + modeStr + "\n" +
        "Generation: " + std::to_string(m_generation) + "\n" +
        "Alive: " + std::to_string(m_aliveCount) + " / " + std::to_string(m_population.size()) + "\n" +
        "Best score ever: " + std::to_string(m_bestScoreEver) + "\n" +
        "Sim speed: " + std::to_string(static_cast<int>(m_simSpeed)) + "x";

    text.setString(body);
    text.setPosition(20.f, 18.f);
    window.draw(text);

    sf::Text controls;
    controls.setFont(font);
    controls.setCharacterSize(14);
    controls.setFillColor(sf::Color(255, 255, 255, 200));
    controls.setString("[+/-] speed   [S] save best brain   [ESC] menu");
    controls.setPosition(10.f, WINDOW_HEIGHT - GROUND_HEIGHT + 20.f);
    window.draw(controls);
}
