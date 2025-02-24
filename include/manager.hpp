#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "cards.hpp"  // If needed
#include "game.hpp"   // So we have Game, playersPool, positions, etc.
#include "player.hpp" // For Player definition
#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct ManagerTest;

class Manager {
private:
  // Private data members
  Game game;                        // The game containing core logic
  gameSettings settings;            // struct with default settings
  std::vector<std::string> names;   // Default names for six players
  notActivePlayers inActivePlayers; // Players with 0 chips (cannot play)
  playersHistory History;           // History of each player
  size_t currentRound;
  positions specialPositions; // Positions for BB, SB, Dealer

  // Private helper methods
  int activePlayers();
  void handleTooFewPlayers();
  position findNextValidPos(position start);

  // Let ManagerTest access private members for testing
  friend struct ::ManagerTest;

public:
  // Public data member (could also be private, with a getter, if you prefer)
  playersPool players; // A deque with shared pointers to each player

  // Constructor
  Manager();

  // Public methods
  void log(const std::string &message) const;
  void initalizeSpecialPositions();
  void initalizePLayers();
  std::shared_ptr<const positions> getSpecialPositions();
  size_t getCurrentRound();
  void gameStatistics();
  void startGame();
  void endGame();
  void arrangePlayersPosition();
  void decidePlayersLifeCycle();
};

#endif // MANAGER_HPP
