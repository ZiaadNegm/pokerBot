#include "manager.hpp"

#include <algorithm> // for std::sort
#include <iomanip>
#include <iostream>
#include <stdexcept>

// -------------------- Private Helpers --------------------

int Manager::activePlayers() {
  int activeCount = 0;
  for (auto &player : players) {
    if (player->getIsActive()) {
      activeCount++;
    }
  }
  return activeCount;
}

void Manager::handleTooFewPlayers() {
  throw std::runtime_error("Too few players to continue the game.");
}

position Manager::findNextValidPos(position start) {
  // If fewer than 3 active players, we can't proceed
  if (activePlayers() < 3) {
    return INVALID_POS;
  }

  start = start % players.size();
  position initial = start;

  do {
    if (players[start] && players[start]->getIsActive()) {
      return start;
    }
    start = (start + 1) % players.size();
  } while (start != initial);

  return INVALID_POS;
}

// -------------------- Public Methods --------------------

Manager::Manager()
    : game(), // default-initialized Game
      names({"Phill", "Doyle", "Daniel", "Chris", "Johnny", "You"}),
      currentRound(0) {
  initalizePLayers();
}

void Manager::log(const std::string &message) const {
  std::cout << message << std::endl;
}

void Manager::initalizeSpecialPositions() {
  position newDealerIndex = findNextValidPos(0);
  if (newDealerIndex == INVALID_POS) {
    handleTooFewPlayers();
    return;
  }

  players[newDealerIndex]->setBlind(Blind::dealer);
  specialPositions.dealerPosition = newDealerIndex;

  position initialSBIndex = findNextValidPos(newDealerIndex + 1);
  if (initialSBIndex == INVALID_POS) {
    handleTooFewPlayers();
    return;
  }

  players[initialSBIndex]->setBlind(Blind::smallBlind);
  specialPositions.posSB = initialSBIndex;

  position initialBBIndex = findNextValidPos(initialSBIndex + 1);
  if (initialBBIndex == INVALID_POS) {
    handleTooFewPlayers();
    return;
  }

  players[initialBBIndex]->setBlind(Blind::bigBlind);
  specialPositions.posBB = initialBBIndex;
}

void Manager::initalizePLayers() {
  // If fewer than min players in names, you could handle it here
  // e.g. prompt or fill with default placeholders

  for (size_t i = 0; i < names.size() && i < settings.maxAmountPlayers; i++) {
    std::shared_ptr<Player> p =
        std::make_shared<Player>(names[i], settings.startingChips);
    players.emplace_back(p);
  }
  initalizeSpecialPositions();
}

std::shared_ptr<const positions> Manager::getSpecialPositions() {
  return std::make_shared<positions>(specialPositions);
}

size_t Manager::getCurrentRound() { return currentRound; }

void Manager::gameStatistics() {
  std::cout << "==========================================" << std::endl;
  std::cout << "           Poker Game Statistics          " << std::endl;
  std::cout << "==========================================" << std::endl;
  std::cout << "Current Round: " << currentRound << std::endl << std::endl;

  // Print header
  std::cout << std::left << std::setw(4) << "#" << std::setw(12) << "Name"
            << std::setw(10) << "Chips" << std::setw(12) << "Blind"
            << std::setw(10) << "Status" << std::endl;

  std::cout << "------------------------------------------" << std::endl;

  int playerNumber = 1;
  for (const auto &player : players) {
    std::string blindStatus = "None";
    switch (player->getBlind()) {
    case Blind::dealer:
      blindStatus = "Dealer";
      break;
    case Blind::smallBlind:
      blindStatus = "SmallBlind";
      break;
    case Blind::bigBlind:
      blindStatus = "BigBlind";
      break;
    default:
      break;
    }

    std::string status =
        player->getIsActive()
            ? "Active"
            : (player->getChips() == 0 ? "Out of Chips" : "Inactive");

    std::cout << std::left << std::setw(4) << playerNumber << std::setw(12)
              << player->getName() << std::setw(10) << player->getChips()
              << std::setw(12) << blindStatus << std::setw(10) << status
              << std::endl;
    playerNumber++;
  }

  std::cout << "------------------------------------------" << std::endl;
}

void Manager::startGame() {
  for (size_t i = 0; i < settings.maximumRounds; i++) {
    // Create a Game object each round with the current players, settings,
    // positions
    Game game(players, settings, specialPositions);
    game.simulateHand();

    decidePlayersLifeCycle();
    arrangePlayersPosition();

    currentRound++;
    gameStatistics();
  }

  endGame();
}

void Manager::endGame() {
  std::cout << "==========================================" << std::endl;
  std::cout << "                Game Over                 " << std::endl;
  std::cout << "==========================================" << std::endl;
  std::cout << "Final Standings:" << std::endl;

  std::cout << std::left << std::setw(4) << "#" << std::setw(15) << "Name"
            << std::setw(10) << "Chips" << std::endl;

  std::cout << "------------------------------------------" << std::endl;

  // Copy players to a vector so we can sort them
  std::vector<std::shared_ptr<Player>> sortedPlayers(players.begin(),
                                                     players.end());
  std::sort(
      sortedPlayers.begin(), sortedPlayers.end(),
      [](const std::shared_ptr<Player> &a, const std::shared_ptr<Player> &b) {
        return a->getChips() > b->getChips();
      });

  // Display each player's final chips
  int rank = 1;
  for (const auto &player : sortedPlayers) {
    std::cout << std::left << std::setw(4) << rank << std::setw(15)
              << player->getName() << std::setw(10) << player->getChips()
              << std::endl;
    rank++;
  }

  std::cout << "------------------------------------------" << std::endl;

  // Identify winner(s)
  if (!sortedPlayers.empty()) {
    money topChips = sortedPlayers.front()->getChips();
    std::vector<std::string> winners;

    for (const auto &player : sortedPlayers) {
      if (player->getChips() == topChips && topChips > 0) {
        winners.push_back(player->getName());
      } else {
        break;
      }
    }

    // Announce winner(s)
    if (winners.size() == 1) {
      std::cout << "Winner: " << winners.front() << " with " << topChips
                << " chips!" << std::endl;
    } else if (winners.size() > 1) {
      std::cout << "Winners: ";
      for (size_t i = 0; i < winners.size(); ++i) {
        std::cout << winners[i];
        if (i < winners.size() - 1) {
          std::cout << ", ";
        }
      }
      std::cout << " each with " << topChips << " chips!" << std::endl;
    } else {
      std::cout << "No winners. All players are out of chips." << std::endl;
    }
  } else {
    std::cout << "No players participated in the game." << std::endl;
  }

  std::cout << "==========================================" << std::endl;
}

void Manager::arrangePlayersPosition() {
  if (activePlayers() < 3) {
    return handleTooFewPlayers();
  }

  // Clear all blinds first
  for (auto &player : players) {
    player->setBlind(Blind::notBlind);
  }

  // Set new positions with validation
  position newDealerIndex =
      findNextValidPos((specialPositions.dealerPosition + 1) % players.size());
  if (newDealerIndex == INVALID_POS) {
    handleTooFewPlayers();
    return;
  }

  position newSBIndex = findNextValidPos((newDealerIndex + 1) % players.size());
  if (newSBIndex == INVALID_POS || newSBIndex == newDealerIndex) {
    handleTooFewPlayers();
    return;
  }

  position newBBIndex = findNextValidPos((newSBIndex + 1) % players.size());
  if (newBBIndex == INVALID_POS || newBBIndex == newDealerIndex ||
      newBBIndex == newSBIndex) {
    handleTooFewPlayers();
    return;
  }

  // Assign new blinds
  specialPositions.dealerPosition = newDealerIndex;
  specialPositions.posSB = newSBIndex;
  specialPositions.posBB = newBBIndex;

  players[newDealerIndex]->setBlind(Blind::dealer);
  players[newSBIndex]->setBlind(Blind::smallBlind);
  players[newBBIndex]->setBlind(Blind::bigBlind);
}

void Manager::decidePlayersLifeCycle() {
  for (auto &player : players) {
    if (player->getChips() == 0) {
      player->setIsActive(false);
      log(player->getName() + " is out of chips and inactive.");
    }
  }
}
