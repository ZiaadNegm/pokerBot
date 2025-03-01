#include "../../../include/IGameIO.hpp"
#include "player.hpp"
#include <iomanip>

class consoleGame : public IGameIO {
  void log(textData data) {
    std::cout << data;
    return;
  }

  void checkHoleCards(textData playerName,
                      const std::vector<Card> &playerHand) override {
    std::cout << "Player: " << playerName << " Has the following cards\n";
    for (Card card : playerHand) {
      log(" ");
      log(card.cardToString());
    }
    return;
  }

  void printGameState(textData gameState) override {
    log("Game state: ");
    log(gameState);
    log("\n");
  }

  void showTurnInfo(const std::shared_ptr<Player> &currentPlayer,
                    const std::shared_ptr<Player> &nextPlayer, gameStates state,
                    money highestBet, money pot) override {

    // Print header with separator
    std::cout << "\n=============================================\n"
              << std::endl;
    std::cout << "It's " << currentPlayer->getName() << "'s turn!\n"
              << std::endl;
    std::cout << currentPlayer->getName() << " has "
              << currentPlayer->getChips() << " chips.\n"
              << std::endl;

    // Show game state
    std::string stateStr;
    switch (state) {
    case gameStates::preFlop:
      stateStr = "preFlop";
      break;
    case gameStates::flop:
      stateStr = "flop";
      break;
    case gameStates::turn:
      stateStr = "turn";
      break;
    case gameStates::river:
      stateStr = "river";
      break;
    case gameStates::showDown:
      stateStr = "showDown";
      break;
    default:
      stateStr = "unknown";
      break;
    }
    printGameState(stateStr);

    // Show hole cards
    std::cout << "Hole Cards:\n";
    for (const auto &card : currentPlayer->getHand()) {
      std::cout << "  " << card.cardToString() << "\n" << std::endl;
    }

    // Show next player info
    if (nextPlayer && nextPlayer != currentPlayer) {
      std::cout << "Next to act: " << nextPlayer->getName() << "\n"
                << std::endl;
    } else {
      std::cout << "No other active players.\n" << std::endl;
    }

    // Show bet information
    std::cout << "Highest bet so far: " << highestBet << "\n" << std::endl;
    std::cout << "Pot: " << pot << "\n" << std::endl;
    std::cout << "=============================================\n\n"
              << std::endl;
  }

  void printPlayersTable(const playersPool &players) override {
    std::cout << "#   Name        Chips     Blind       Status\n" << std::endl;
    std::cout << "------------------------------------------\n" << std::endl;

    int playerNumber = 1;
    for (const auto &player : players) {
      // Format blind status
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

      // Format player status
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

  std::vector<actions> offerOptions(actionMap validMoves) override {
    std::vector<actions> offeredOptions;
    offeredOptions.reserve(validMoves.size());

    std::cout << "Available actions:\n" << std::endl;
    int optionCounter = 1;

    for (const auto &[act, pairVal] : validMoves) {
      if (pairVal.first) {
        auto it = actionmessages.find(act);
        std::string actionText =
            (it != actionmessages.end()) ? it->second : "Unknown action";

        std::cout << optionCounter << ") " << actionText;
        if (act == actions::raise || act == actions::bet ||
            act == actions::allIn) {
          std::cout << " [Amount: " << pairVal.second << "]";
        }
        std::cout << "\n" << std::endl;

        offeredOptions.push_back(act);
        optionCounter++;
      }
    }

    return offeredOptions;
  }

  money promptForActionAmount(actions act, money minAmount) override {
    std::string actionName = (act == actions::bet) ? "bet" : "raise";
    std::cout << "You can " << actionName << " from " << minAmount
              << " chips.\n"
              << std::endl;
    std::cout << "How much would you like to " << actionName << "? "
              << std::endl;

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

  Action getInputPlayer(std::vector<actions> offeredOptions,
                        actionMap validMoves) override {
    log("Enter the number of your choice: \n");
    int choice = 0;
    std::cin >> choice;

    if (choice < 1 || choice > static_cast<int>(offeredOptions.size())) {
      std::cout << "Invalid choice, defaulting to fold.\n" << std::endl;
      return Action{actions::fold, 0, 0};
    }

    actions selected = offeredOptions[choice - 1];
    money chosenAmount = validMoves[selected].second;

    if (selected == actions::bet || selected == actions::raise) {
      chosenAmount = promptForActionAmount(selected, chosenAmount);
    }

    return Action{selected, chosenAmount, 0};
  }

  void logActions(std::shared_ptr<Player> player, Action action) override {
    std::string playerName = player->getName();

    std::cout << "[" << playerName << "] ";

    switch (action.action) {
    case actions::fold:
      std::cout << "folds.";
      break;
    case actions::check:
      std::cout << "checks.";
      break;
    case actions::call:
      std::cout << "calls with " << action.bet << " chips.";
      break;
    case actions::raise:
      std::cout << "raises by " << action.bet << " chips.";
      break;
    case actions::allIn:
      std::cout << "goes all-in with " << action.bet << " chips.";
      break;
    case actions::bet:
      std::cout << "places a bet of " << action.bet << " chips.";
      break;
    default:
      std::cout << "performs an unknown action.";
      break;
    }

    std::cout << "\n" << std::endl;
  }
};