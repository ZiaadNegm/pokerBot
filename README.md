# Poker Engine

A Texas Hold'em poker simulation built to explore modern C++20 features and software design patterns.

## Motivation

I built this project to get hands-on experience with C++20 in a non-trivial domain. Poker demands complex state management, turn-based logic with positional awareness, and clean separation of concerns, making it an ideal vehicle for learning modern C++ patterns beyond textbook examples.

## C++20 Features Used

- **C++20 Modules** (`.cppm`): Replaced traditional header-only design with proper module boundaries (`import player`, `import cards`, `import bestHand`), enforcing clear dependency graphs at compile time.
- **Strong type aliases**: `using money = std::uint32_t` and `using position = std::int32_t` for domain-level type clarity instead of raw primitives.
- **Polymorphic memory resources** (`std::pmr`): Used `std::pmr::unordered_map` for player action history, exploring allocator-aware containers.
- **Smart pointers**: `std::shared_ptr<Player>` models shared ownership across game, manager, and pot structures.
- **`std::function` and lambdas**: Used extensively for action dispatch and state handler registration.

## Design Patterns

### State Machine

Game states (`preFlop → flop → turn → river → showDown`) are mapped to handler functions via `std::unordered_map<gameStates, stateHandler>`. This gives clean state dispatch without a monolithic switch/case block. Each state handler is a self-contained function.

### Command Pattern

Player actions (fold, check, call, raise, all-in, bet) are dispatched through `std::unordered_map<actions, actionHandler>`. Each action is a first-class callable with a uniform signature, making it straightforward to add or modify actions without touching dispatch logic.

### Manager / Game Separation

`Manager` orchestrates multi-round play: player rotation, blind progression, chip elimination, and game settings. `Game` owns single-hand logic: the deck, community cards, betting rounds, and winner determination. This separation keeps each class focused on a single responsibility.

## Features

- Full betting system: fold, check, call, raise, all-in
- Game state machine progressing through all poker stages
- Dealer and blind rotation across rounds
- Dynamic valid-action computation based on player chip count, current bet, and pot state
- Player lifecycle management (automatic elimination at 0 chips)
- Unit tests with Google Test for card and player classes

## What I Learned

This project went through 40+ commits of iterative development. Here's what stood out:

**Debugging stateful systems is hard.** Position tracking and betting sequences required introducing explicit flags (`killSwitch`, `freeDealerPass`, `firstIteration`) to handle boundary conditions like the next player being the dealer or everyone folding to the big blind. I learned that game state machines need deliberate handling of edge cases that aren't obvious from the rules alone.

**Modularization is a refactoring discipline.** The codebase started as a single `game.cpp`. Over multiple commits, I deliberately split it into C++20 modules and header/source pairs, learning when to draw module boundaries and how C++20 modules change the way you think about compilation units versus traditional `#include`.

**Smart pointer ownership is a design decision.** Using `shared_ptr` for players shared across the game, manager, and pot structures taught me to think about object lifetimes as a design choice, not just a memory management detail.

**Early tests pay off during refactoring.** Building card and player tests early with Google Test gave me confidence to aggressively restructure the game logic without breaking foundational behavior.

## Roadmap

- [ ] Full poker hand ranking (flushes, straights, full houses, etc.)
- [ ] Side pot support for all-in scenarios
- [ ] HTTP REST API via cpp-httplib or Crow to decouple from console I/O
- [ ] Python ML integration for training a poker-playing agent

## Build & Run

**Requirements**: CMake 3.25+, a C++20-capable compiler (Clang recommended), Ninja, Google Test

```bash
# Configure
cmake -B build -G Ninja

# Build
cmake --build build

# Run the game
./build/poker_game

# Run tests
cd build && ctest --output-on-failure
```

## Tech Stack

C++20, CMake, Clang/Ninja, Google Test
