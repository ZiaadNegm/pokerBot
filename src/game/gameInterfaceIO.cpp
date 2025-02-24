#include "../include/game.hpp"
// ------------------- Private Helpers -------------------
// gameInterfaceIO
void Game::checkHoleCards() {
  for (auto player : players) {
    std::cout << "Player " << player->getName() << " Has the following cards\n";
    for (Card card : player->getHand()) {
      std::cout << " " << card.cardToString() << "\n";
    }
    std::cout << "\n";
  }
}
// gameInterfaceIO
void Game::printGameState(gameStates state) {
  std::cout << "Game state: ";
  switch (state) {
  case gameStates::preFlop:
    std::cout << "Pre-Flop";
    break;
  case gameStates::flop:
    std::cout << "Flop";
    break;
  case gameStates::turn:
    std::cout << "Turn";
    break;
  case gameStates::river:
    std::cout << "River";
    break;
  case gameStates::showDown:
    std::cout << "Show Down";
    break;
  default:
    std::cout << "Unknown";
    break;
  }
  std::cout << std::endl;
}

// gameInterfaceIO
void Game::showTurnInfo(const std::shared_ptr<Player> &currentPlayer) {
  // Print full table first
  std::cout << "\n\n";
  printPlayersTable();

  // Show current player's turn block
  std::cout << "\n=============================================\n" << std::endl;
  std::cout << "It's " << currentPlayer->getName() << "'s turn!\n" << std::endl;
  std::cout << currentPlayer->getName() << " has " << currentPlayer->getChips()
            << " chips.\n"
            << std::endl;

  printGameState(gameState);

  // Show hole cards
  std::cout << "Hole Cards:\n";
  for (const auto &card : currentPlayer->getHand()) {
    std::cout << "  " << card.cardToString() << "\n" << std::endl;
  }

  // Who is next?
  auto next = getNextActiveAfter(currentPlayer);
  if (next && next != currentPlayer) {
    std::cout << "Next to act: " << next->getName() << "\n" << std::endl;
  } else {
    std::cout << "No other active players.\n" << std::endl;
  }

  std::cout << "Highest bet so far: " << highestBet << "\n" << std::endl;
  std::cout << "Pot: " << pot << "\n" << std::endl;
  std::cout << "=============================================\n\n" << std::endl;
}

// gameInterfaceIO
void Game::printPlayersTable() {
  std::cout << "#   Name        Chips     Blind       Status\n" << std::endl;
  std::cout << "------------------------------------------\n" << std::endl;

  int playerNumber = 1;
  for (const auto &player : players) {
    // Determine blind status
    std::string blindStatus;
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
      blindStatus = "None";
      break;
    }

    // Determine active/folded
    std::string status;
    if (!player->getIsActive()) {
      status = (player->getChips() == 0 ? "Out of Chips" : "Inactive");
    } else {
      status = (player->hasPlayerFolded() ? "Folded" : "Active");
    }

    std::cout << std::left << std::setw(4) << playerNumber << std::setw(12)
              << player->getName() << std::setw(10) << player->getChips()
              << std::setw(12) << blindStatus << std::setw(10) << status
              << std::endl;
    playerNumber++;
  }
  std::cout << "------------------------------------------\n" << std::endl;
}

// GameFlow / gameInterfaceIO. Can we split this up into logic and interface
// type?
Action Game::offerOptions(actionMap validMoves) {
  std::vector<actions> offeredOptions;
  offeredOptions.reserve(validMoves.size());

  std::cout << "Available actions:\n" << std::endl;
  int optionCounter = 1;
  for (const auto &[act, pairVal] : validMoves) {
    if (pairVal.first) {
      auto it = actionmessages.find(act);
      std::string actionText =
          (it != actionmessages.end()) ? it->second : "Unknown action";
      std::cout << optionCounter << ") " << actionText
                << " [Amount: " << pairVal.second << "]\n"
                << std::endl;
      offeredOptions.push_back(act);
      optionCounter++;
    }
  }
  return getInputPlayer(offeredOptions, validMoves);
}

// GameInterfaceIO
money Game::promptForActionAmount(actions act, money minAmount) {
  std::string actionName = (act == actions::bet) ? "bet" : "raise";
  std::cout << "You can " << actionName << " from " << minAmount << " chips.\n"
            << std::endl;
  std::cout << "How much would you like to " << actionName << "? " << std::endl;

  money inputAmount;
  std::cin >> inputAmount;

  while (std::cin.fail() || inputAmount < minAmount) {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid amount. Please enter an amount >= " << minAmount
              << ": " << std::endl;
    std::cin >> inputAmount;
  }
  return inputAmount;
}

// GameInterfaceIO
Action Game::getInputPlayer(std::vector<actions> offeredOptions,
                            actionMap validMoves) {
  std::cout << "Enter the number of your choice: " << std::endl;
  int choice = 0;
  std::cin >> choice;

  if (choice < 1 || choice > static_cast<int>(offeredOptions.size())) {
    std::cout << "Invalid choice, defaulting to fold.\n" << std::endl;
    return Action{actions::fold, 0, 0};
  }

  actions selected = offeredOptions[choice - 1];
  money chosenAmount = validMoves[selected].second;

  // For bet and raise, allow custom amount (>= the proposed minimum)
  if (selected == actions::bet || selected == actions::raise) {
    chosenAmount = promptForActionAmount(selected, chosenAmount);
  }
  return Action{selected, chosenAmount, currentRound};
}

// gameInterfaceIO
void Game::logActions(std::shared_ptr<Player> player, Action action) {
  std::string playerName = player->getName();
  switch (action.action) {
  case actions::fold:
    std::cout << "[" << playerName << "] folds.\n" << std::endl;
    break;
  case actions::check:
    std::cout << "[" << playerName << "] checks.\n" << std::endl;
    break;
  case actions::call:
    std::cout << "[" << playerName << "] calls with " << action.bet
              << " chips.\n"
              << std::endl;
    break;
  case actions::raise:
    std::cout << "[" << playerName << "] raises by " << action.bet
              << " chips.\n"
              << std::endl;
    break;
  case actions::allIn:
    std::cout << "[" << playerName << "] goes all-in with " << action.bet
              << " chips.\n"
              << std::endl;
    break;
  case actions::bet:
    std::cout << "[" << playerName << "] places a bet of " << action.bet
              << " chips.\n"
              << std::endl;
    break;
  default:
    std::cout << "[" << playerName << "] performs an unknown action."
              << std::endl;
    break;
  }
}
// ------------------- Main Round Flow -------------------