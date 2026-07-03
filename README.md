# ZaiGame — Flappy AI

A genetic algorithm teaches a population of birds to play Flappy Bird. No ML libraries — the neural network is written by hand, math and all.

## About

I built this in my first year as a Computer Engineering student, back in 2023. I wanted to actually understand neural networks instead of just importing one from a library, so I wrote the feed-forward math and the genetic algorithm myself and watched it learn.

A batch of birds spawns and competes to survive the longest and score the most points. Every generation, the best performers pass their "brains" to the next one, with a bit of random mutation thrown in, so each generation tends to get a little better than the last.

I named it Zai. It's a simple project, but it's the one that made neural networks click for me.

## Features

- Feed-forward neural network written from scratch
- Genetic algorithm with elitism, fitness-proportionate selection, and mutation
- 150 birds evolving in parallel across generations
- Save and load the best-performing brain
- A basic main menu — train a new population or watch a saved brain play
- Fast-forward training up to 8x speed

## Folder Structure

```
ZaiGame/
├── ZaiGame.sln
├── ZaiGame.vcxproj
├── ZaiGame.vcxproj.filters
├── include/                # headers
│   ├── Constants.h          - all tunable numbers
│   ├── NeuralNetwork.h
│   ├── FlappyAI.h            - population + physics + rendering
│   ├── Menu.h
│   └── Game.h                - owns the window, state machine
├── src/                    # implementation
│   ├── NeuralNetwork.cpp
│   ├── FlappyAI.cpp
│   ├── Menu.cpp
│   ├── Game.cpp
│   └── main.cpp
├── assets/fonts/           # put a .ttf here (see setup below)
├── saves/                  # best_model.txt is written here at runtime
└── External/SFML/          # your SFML install: include/, lib/, bin/
```

## Setup

Two things need to be in place before it builds.

**SFML**
Put your SFML install at `External/SFML/` so it looks like this:
```
External/SFML/include/
External/SFML/lib/
External/SFML/bin/     (the .dll files)
```
This links against SFML 2.x (`sfml-graphics-2.lib` / `sfml-graphics-d-2.lib`, etc.). If your SFML build uses different library names, update `AdditionalDependencies` in the project's linker settings.

**Font**
Drop any `.ttf` file at `assets/fonts/arial.ttf`, or rename it, or point `Constants::FONT_PATH` somewhere else. Any open font like DejaVu Sans works fine. Without one the game still runs, it just won't draw text.

**Build**
Build Debug or Release, x64. A post-build step copies the SFML DLLs and the `assets/` folder next to the .exe, and creates an empty `saves/` folder there too.

## Controls

Main menu:
- Up/Down + Enter to navigate and select
- 1 / 2 / 3 to jump straight to an option

Training / Watching:
- `+` speeds up the simulation, up to 8x
- `-` slows it back down
- `S` saves the current best brain to `saves/best_model.txt`
- `Esc` returns to the main menu

## How It Works

### The brain — NeuralNetwork

A plain feed-forward network: `weights[layer][from][to]` plus a bias per neuron, tanh activation on the hidden layers and sigmoid on the output so it lands in (0, 1). `mutate()` nudges random weights and biases with gaussian noise. `clone()` deep-copies a genome for the next generation. `saveToFile()` / `loadFromFile()` serialize topology, weights, and biases as plain numbers, so a trained brain survives between runs.

### The simulation — FlappyAI

Holds the population (150 birds by default) and the pipes. Every frame, each living bird:

1. Looks at five normalized inputs — its own height, its own velocity, the horizontal distance to the next pipe, and the top and bottom edges of that pipe's gap.
2. Feeds those into its brain. If the output neuron fires above 0.5, the bird flaps.
3. Gravity and velocity get integrated each frame, then it's checked against the ground, ceiling, and pipe rectangles.

All birds share the same x position — only the pipes scroll. That's the standard trick for this kind of simulation: every bird sees the exact same obstacle at the exact same moment, so comparing their fitness is fair.

Fitness is time survived plus 70 times pipes passed, so a bird that actually finds the gap is worth a lot more than one that just glides around for a while.

When every bird is dead, `naturalSelection()` runs:

1. Sort the population by fitness.
2. Keep the top 4 unmutated — elitism, so a good genome doesn't get lost to bad luck.
3. Fill the rest of the new population through fitness-proportionate (roulette) selection of a parent, cloned and mutated.
4. Remember the best genome ever seen, independent of any single generation's population, so `S` always has something worth saving.

### The shell — Game and Menu

`Game` owns the SFML window and switches between three states: MainMenu, Training, and Watching (a single saved brain replaying forever). `Menu` just draws the options and reports a selection back to `Game`.
