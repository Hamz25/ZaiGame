#pragma once

// All tunable numbers live here so you can tweak the simulation
// without hunting through game logic code.
namespace Constants {

    // ---- Window ----
    constexpr unsigned int WINDOW_WIDTH  = 900;
    constexpr unsigned int WINDOW_HEIGHT = 600;
    constexpr float GROUND_HEIGHT        = 60.f;

    // ---- Population / Genetic Algorithm ----
    constexpr int   POPULATION_SIZE   = 150;
    constexpr int   ELITE_COUNT       = 4;     // top N carried over unmutated
    constexpr float MUTATION_RATE     = 0.12f; // chance per weight/bias to mutate
    constexpr float MUTATION_STRENGTH = 0.35f; // std-dev of gaussian nudge

    // ---- Bird physics ----
    constexpr float BIRD_X            = 150.f;
    constexpr float BIRD_RADIUS       = 12.f;
    constexpr float GRAVITY           = 1400.f;  // px/s^2
    constexpr float FLAP_VELOCITY     = -420.f;  // px/s (negative = up)
    constexpr float MAX_FALL_SPEED    = 700.f;

    // ---- Pipes ----
    constexpr float PIPE_WIDTH    = 70.f;
    constexpr float PIPE_GAP      = 170.f;
    constexpr float PIPE_SPACING  = 300.f; // horizontal distance between pipes
    constexpr float PIPE_MIN_Y    = 90.f;  // min gap-center distance from top
    constexpr float SCROLL_SPEED  = 220.f; // px/s world scroll speed

    // ---- Neural network topology ----
    // Inputs: birdY(norm), birdVel(norm), dxToPipe(norm), gapTop(norm), gapBottom(norm)
    constexpr int NN_INPUTS  = 5;
    constexpr int NN_HIDDEN  = 8;
    constexpr int NN_OUTPUTS = 1;

    // ---- Files ----
    inline const char* BEST_MODEL_PATH = "saves/best_model.txt";
    inline const char* FONT_PATH       = "assets/fonts/RobotikaPixelGreek-nAWJR.otf";
}
