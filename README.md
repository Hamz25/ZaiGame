## ZaiGame — Flappy AI

This is a flappy bird game that I created to learn about reinforcment learning.
It will spawn a number of birds that will compete to get the best score.
Each generation will be better than the previous generation.
I created this project in my first year as a computer engineering student in 2023. 
It is a simple project but a greate one to make me understand how neural networks work and how to deal with it and it's math. 
I named the game Zai.

## Folder structure

```
ZaiGame/
├── ZaiGame.sln
├── ZaiGame.vcxproj
├── ZaiGame.vcxproj.filters
├── include/              # headers
│   ├── Constants.h        (all tunable numbers)
│   ├── NeuralNetwork.h
│   ├── FlappyAI.h          (population + physics + rendering)
│   ├── Menu.h
│   └── Game.h              (owns the window, state machine)
├── src/                   # implementation
│   ├── NeuralNetwork.cpp
│   ├── FlappyAI.cpp
│   ├── Menu.cpp
│   ├── Game.cpp
│   └── main.cpp
├── assets/fonts/          # put a .ttf here (see below)
├── saves/                 # best_model.txt is written here at runtime
└── External/SFML/         # put your SFML install here: include/, lib/, bin/
```

## One-time setup before it builds

1. **SFML**: this project expects `External/SFML/include`, `External/SFML/lib`,
   and `External/SFML/bin` (for the DLLs) next to the `.sln`, matching your
   original layout. It's linked against SFML 2.x (`sfml-graphics-2.lib` /
   `sfml-graphics-d-2.lib`, etc.) — if your SFML build uses different lib
   names, update `AdditionalDependencies` in the project's Linker settings.
2. **Font**: drop any `.ttf` file at `assets/fonts/arial.ttf` (rename it, or
   change `Constants::FONT_PATH`). Any open font like DejaVu Sans works fine.
   Without a font file the game still runs, it just won't draw text.
3. Build (Debug or Release, x64). A post-build step copies the SFML DLLs and
   the `assets/` folder next to the .exe, and creates an empty `saves/`
   folder there too.

## Controls

- **Main menu**: Up/Down + Enter, or press `1` / `2` / `3`
- **In training / watching**:
  - `+` / `-` — speed up / slow down the simulation (up to 8x, useful for
    blasting through generations quickly)
  - `S` — save the current best-performing brain to `saves/best_model.txt`
  - `Esc` — back to the main menu

## How it works

**`NeuralNetwork`** is a plain feed-forward net: `weights[layer][from][to]`
plus a bias per neuron, `tanh` on hidden layers and `sigmoid` on the output
so it lands in (0,1). `mutate()` nudges random weights/biases by gaussian
noise. `clone()` deep-copies a genome. `saveToFile`/`loadFromFile` serialize
topology + weights + biases as plain numbers so a trained brain survives
between runs.

**`FlappyAI`** holds the population (150 birds by default) and the pipes.
Every frame, each living bird:
1. Looks at 5 normalized inputs: its own height, its own velocity, the
   horizontal distance to the next pipe, and the top/bottom edges of that
   pipe's gap.
2. Feeds them through its brain. If the single output neuron fires above
   0.5, the bird flaps.
3. Gravity and velocity get integrated, and it's checked against the
   ground/ceiling and the pipe rectangles (circle-vs-rectangle test).

All birds share the same x position (only the pipes scroll), which is the
standard trick for this kind of demo — it means every bird sees the exact
same obstacle at the exact same time, so comparing their fitness is fair.

Fitness = time survived + 70 × pipes passed, so a bird that finds the gap is
worth far more than one that just glides for a while. When every bird is
dead, `naturalSelection()` runs:
- sorts by fitness,
- keeps the top 4 unmutated (elitism, so a good genome can't be lost to bad
  luck in the next generation),
- fills the rest of the new population via fitness-proportionate (roulette)
  selection of a parent, cloned and mutated,
- and remembers the best genome ever seen (independent of any one
  generation's population) so `S` always has something meaningful to save.

**`Game`** owns the SFML window and switches between `MainMenu`,
`Training`, and `Watching` (a single saved brain replaying forever).
**`Menu`** just draws options and reports a selection back to `Game`.
