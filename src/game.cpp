#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <system_error>
#include <unordered_map>
#include <vector>
import cards;
import player;
#include <deque>
#include <map>
#include <string>

// Shorter aliases
using money = std::uint32_t;
using playersPool = std::deque<Player>;
using position = std::size_t;

std::string playersName[] = {"Phill", "Doyle",  "Daniel",
                             "Chris", "Johnny", "You"};

// Poker constants
namespace poker {
constexpr size_t MIN_PLAYERS = 2;
constexpr size_t MAX_PLAYERS = 6;
constexpr size_t CARDS_PER_PLAYER = 2;
constexpr money MINIMUM_BET = 10;
constexpr money STARTING_CHIPS = 100;
} // namespace poker

// Enums
enum class actions { fold, check, call, raise, allIn };
enum class gameState { preFlop, flop, turn, river, showDown };

struct Action {
  actions action;
  money bet;
  int roundCounter;
};

using playersHistory = std::pmr::unordered_map<int, Action>;

class Game {
private:
  Deck deck;
  money pot;
  money highestBettedInRound;
  money currentBet;
  playersPool PlayersTurn;
  std::vector<Card> communityCards;
  int roundCounter;
  playersHistory History;
  gameState state;
  position dealerPosition;
  money minimumBet;
  size_t playerSize;
  position SBIndex;
  position BBIndex;

  // Provide default and custom config
  // Provides default initialization.
  void defaultConfig() {
    std::cout << "Default configuration" << std::endl;
    std::cout << std::endl;
  }

  // Customizes the game to the persons liking.
  void customConfig() {
    std::cout << "Custom configuration: " << std::endl;

    std::cout << "Enter amount of players (2-" << poker::MAX_PLAYERS << "): ";
    std::cin >> playerSize;

    std::cout << "Enter minimum bet: ";
    std::cin >> minimumBet;

    if ((playerSize > poker::MAX_PLAYERS) ||
        (playerSize < poker::MIN_PLAYERS)) {
      throw std::invalid_argument(
          "Invalid amount of players added. Must be between 2 and up to 6");
    }
    if (minimumBet <= 0) {
      throw std::invalid_argument(
          "Invalid minimumBet provided. Must be greater than 0");
    }
  }

  // Count how many players are still active (not folded)
  int countActivePlayers() {
    int count = 0;
    for (auto &p : PlayersTurn) {
      if (!p.hasPlayerFolded()) {
        count++;
      }
    }
    return count;
  }

  void resetBlindsandSetBlinds() {
    position posPreviousSmallBlind = (dealerPosition) % PlayersTurn.size();
    position posPreviousBigBlind = (dealerPosition + 1) % PlayersTurn.size();
    position nextBigBlind = (posPreviousBigBlind + 1) % PlayersTurn.size();

    PlayersTurn[posPreviousBigBlind].setBlind(Blind::smallBlind);
    PlayersTurn[posPreviousSmallBlind].setBlind(Blind::notBlind);
    PlayersTurn[nextBigBlind].setBlind(Blind::bigBlind);

    SBIndex = posPreviousBigBlind;
    BBIndex = nextBigBlind;
  }

  // Collect blinds
  void collectBlindBets() {
    Player &SB = PlayersTurn[SBIndex];
    Player &BB = PlayersTurn[BBIndex];

    SB.bet(minimumBet / 2);
    BB.bet(minimumBet);

    pot += static_cast<money>(1.5 * minimumBet);
  }

  void dealHoleCards() {
    for (int round = 0; round < 2; ++round) {
      position currentIndex = dealerPosition;
      do {
        Card dealtCard = deck.dealCard();
        std::vector<Card> currentHand = PlayersTurn[currentIndex].getHand();
        currentHand.push_back(dealtCard);
        PlayersTurn[currentIndex].receiveCards(currentHand);

        currentIndex = (currentIndex + 1) % PlayersTurn.size();
      } while (currentIndex != dealerPosition);
    }
  }

  void dealCommunityCards(int quantityCards) {
    deck.burnCard();
    for (int i = 0; i < quantityCards; i++) {
      communityCards.emplace_back(deck.dealCard());
    }
  }

  void dealCards() {
    switch (state) {
    case gameState::preFlop:
      dealHoleCards();
      break;
    case gameState::flop:
      dealCommunityCards(3);
      break;
    case gameState::turn:
    case gameState::river:
      dealCommunityCards(1);
      break;
    default:
      break;
    }
  }

  void printCommunityCards() const {
    std::cout << "Community Cards:";
    for (const auto &card : communityCards) {
      std::cout << " " << card.cardToString();
    }
    std::cout << "\n";
  }

  void printGameState(gameState s) {
    switch (s) {
    case gameState::preFlop:
      std::cout << "Game State: preFlop\n";
      break;
    case gameState::flop:
      std::cout << "Game State: flop\n";
      break;
    case gameState::turn:
      std::cout << "Game State: turn\n";
      break;
    case gameState::river:
      std::cout << "Game State: river\n";
      break;
    case gameState::showDown:
      std::cout << "Game State: showDown\n";
      break;
    default:
      std::cout << "Game State: Unknown\n";
      break;
    }
  }

  actions action(Player &player, position pos, bool isYou) {
    int option = 0;
    printCommunityCards();
    if (isYou) {
      // Prompt user for an action
      while (true) {
        std::cout << "\n[" << player.getName() << "] Choose your action:\n"
                  << "1. Fold\n"
                  << "2. Check\n"
                  << "3. Call\n"
                  << "4. Raise\n"
                  << "5. All-In\n"
                  << "> ";
        std::cin >> option;
        if (std::cin.fail() || option < 1 || option > 5) {
          std::cin.clear();
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::cout << "Invalid choice. Enter a number 1..5.\n";
          continue;
        }
        break;
      }
    } else {
      // Simple bot logic
      int randomChoice = 1 + (rand() % 5);
      option = randomChoice;
      std::cout << "[" << player.getName() << " (BOT)] at Position " << pos
                << " with cards " << player.getHand()[0].cardToString()
                << " and " << player.getHand()[1].cardToString() << std::endl
                << " chooses option "
                << " " << option << "\n";
    }

    switch (option) {
    case 1:          // Fold
      player.fold(); // sets hasPlayerFolded(true)
      std::cout << "[" << player.getName() << "] folds.\n";
      return actions::fold;
    case 2: // Check
      if (highestBettedInRound == 0) {
        player.check();
        std::cout << "[" << player.getName() << "] checks.\n";
        return actions::check;
      } else {
        std::cout << "A bet has already been made. This player cannot check.\n"
                  << std::endl;
        // recursively call action() again
        return action(player, pos, isYou);
      }
    case 3: // Call
    {
      money toCall = highestBettedInRound - player.getCurrentBet();

      // If there's no extra bet to match
      if (toCall <= 0) {
        std::cout << "[" << player.getName()
                  << "] checks (no extra to call).\n";
        return actions::check;
      }

      // If toCall is more than the player's chips => fold or all-in
      if (toCall > player.getChips()) {
        // If it's the human player, ask them
        if (isYou) {
          while (true) {
            std::cout << "You only have " << player.getChips()
                      << " chips, but need " << toCall
                      << " to call.\nPick action:\n"
                      << "1. Fold\n"
                      << "2. Go All-In\n> ";
            int choice;
            std::cin >> choice;
            if (std::cin.fail()) {
              std::cin.clear();
              std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                              '\n');
              std::cout << "Invalid input.\n";
              continue;
            }

            if (choice == 1) {
              player.fold();
              std::cout << "[" << player.getName() << "] folds.\n";
              return actions::fold;
            } else if (choice == 2) {
              money allin = player.getChips();
              pot += player.bet(allin);
              std::cout << "[" << player.getName() << "] goes ALL-IN with "
                        << allin << " chips.\n";
              if (allin > highestBettedInRound) {
                highestBettedInRound = allin;
              }
              return actions::allIn;
            } else {
              std::cout << "Invalid choice. Please try again.\n";
            }
          }
        } else {
          // If it's a bot, pick fold or all-in automatically (e.g. 50-50).
          bool doAllIn = (rand() % 2 == 0);
          if (doAllIn) {
            money allin = player.getChips();
            pot += player.bet(allin);
            std::cout << "[" << player.getName() << "] goes ALL-IN with "
                      << allin << " chips.\n";
            if (allin > highestBettedInRound) {
              highestBettedInRound = allin;
            }
            return actions::allIn;
          } else {
            player.fold();
            std::cout << "[" << player.getName() << "] folds.\n";
            return actions::fold;
          }
        }
      }

      // Otherwise, they can afford the call
      pot += player.bet(toCall);
      std::cout << "[" << player.getName() << "] calls " << toCall << ".\n";
      return actions::call;
    }

    case 4: // Raise
    {
      std::cout << "[" << player.getName() << "] Raise amount?\n> ";
      int raiseAmount = 0;
      if (!isYou) {
        // Simple bot logic
        raiseAmount = 10 + (rand() % 20);
        std::cout << raiseAmount << "\n";
      } else {
        std::cin >> raiseAmount;
        if (std::cin.fail() || raiseAmount < 1) {
          std::cin.clear();
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::cout << "Invalid raise. Defaulting to 1.\n";
          raiseAmount = 1;
        }
      }

      // totalRaise = how many more chips beyond player's current bet
      money totalRaise =
          (highestBettedInRound - player.getCurrentBet()) + raiseAmount;

      // If totalRaise is more than player has, fold or all-in
      if (totalRaise > player.getChips()) {
        if (isYou) {
          while (true) {
            std::cout << "You only have " << player.getChips()
                      << " chips, but total raise would be " << totalRaise
                      << ".\nPick action:\n"
                      << "1. Fold\n"
                      << "2. Go All-In\n> ";
            int choice;
            std::cin >> choice;
            if (std::cin.fail()) {
              std::cin.clear();
              std::cin.ignore(std::numeric_limits<std::streamsize>::max(),
                              '\n');
              std::cout << "Invalid input.\n";
              continue;
            }

            if (choice == 1) {
              player.fold();
              std::cout << "[" << player.getName() << "] folds.\n";
              return actions::fold;
            } else if (choice == 2) {
              money allin = player.getChips();
              pot += player.bet(allin);
              std::cout << "[" << player.getName() << "] goes ALL-IN with "
                        << allin << " chips.\n";
              if (allin > highestBettedInRound) {
                highestBettedInRound = allin;
              }
              return actions::allIn;
            } else {
              std::cout << "Invalid choice. Please try again.\n";
            }
          }
        } else {
          // Bot logic: 50/50 fold or all-in
          bool doAllIn = (rand() % 2 == 0);
          if (doAllIn) {
            money allin = player.getChips();
            pot += player.bet(allin);
            std::cout << "[" << player.getName() << "] goes ALL-IN with "
                      << allin << " chips.\n";
            if (allin > highestBettedInRound) {
              highestBettedInRound = allin;
            }
            return actions::allIn;
          } else {
            player.fold();
            std::cout << "[" << player.getName() << "] folds.\n";
            return actions::fold;
          }
        }
      }

      // Otherwise, they can afford the raise
      pot += player.raise(highestBettedInRound, raiseAmount);
      highestBettedInRound += raiseAmount;
      std::cout << "[" << player.getName() << "] raises by " << raiseAmount
                << ". New highest bet: " << highestBettedInRound << "\n";
      return actions::raise;
    }

    case 5: // All-In
    {
      money allinAmount = player.getChips();
      pot += player.bet(allinAmount);
      std::cout << "[" << player.getName() << "] goes ALL-IN with "
                << allinAmount << " chips.\n";
      if (allinAmount > highestBettedInRound) {
        highestBettedInRound = allinAmount;
      }
      return actions::allIn;
    }
    default:
      // Default: fold
      std::cout << "[" << player.getName() << "] folds by default.\n";
      player.fold();
      return actions::fold;
    }
  }

  void nextState() {
    switch (state) {
    case gameState::preFlop:
      state = gameState::flop;
      break;
    case gameState::flop:
      state = gameState::turn;
      break;
    case gameState::turn:
      state = gameState::river;
      break;
    case gameState::river:
      state = gameState::showDown;
      break;
    case gameState::showDown: /* do nothing */
      break;
    }
  }

  void handleEarlyWinner() {
    givePotToLastStanding();
    resetCards();
    dealerPosition = (dealerPosition + 1) % PlayersTurn.size();
  }

  void givePotToLastStandingOrShowdown() {
    if (countActivePlayers() == 1) {
      givePotToLastStanding();
    } else {
      std::cout << "Showdown logic not implemented.\n";
    }
  }

  void letPlayerstakeAction() {
    position actionInitiator = (BBIndex + 1) % PlayersTurn.size();
    position pos = actionInitiator;
    actions act;

    do {
      Player &currentPlayer = PlayersTurn[pos];

      // Skip if folded or out of chips
      if (currentPlayer.hasPlayerFolded() || currentPlayer.getChips() == 0) {
        pos = (pos + 1) % PlayersTurn.size();
        if (pos == actionInitiator)
          break;
        continue;
      }
      std::cout << "Pot: " << pot << std::endl;
      bool isYou = (currentPlayer.getName() == "You");
      act = action(currentPlayer, pos, isYou);

      if (act == actions::raise || act == actions::allIn) {
        actionInitiator = pos;
      }

      pos = (pos + 1) % PlayersTurn.size();
    } while (pos != actionInitiator);
  }

  // Award pot to the last active player
  void givePotToLastStanding() {
    // Find whoever isn't folded
    for (auto &p : PlayersTurn) {
      if (!p.hasPlayerFolded()) {
        // give pot to this player
        p.addChips(pot);
        std::cout << "[" << p.getName() << "] wins the pot of " << pot << "!\n";
        break;
      }
    }
    pot = 0;
  }

  // Un-fold players, clear community cards, etc.
  void resetCards() {
    communityCards.clear();
    highestBettedInRound = 0;
    currentBet = 0;

    for (auto &p : PlayersTurn) {
      // Un-fold them for the next hand
      p.setHasFolded(false);
      p.resetCards();
      p.setCurrentBet(0);
      p.setBlind(Blind::notBlind);
    }
  }

public:
  Game()
      : deck(), pot(0), highestBettedInRound(0), currentBet(0), PlayersTurn(),
        communityCards(), roundCounter(0), History(), state(gameState::preFlop),
        dealerPosition(0), minimumBet(poker::MINIMUM_BET), playerSize(6) {}

  void initalizePLayers() {
    PlayersTurn.clear();
    for (size_t i = 0; i < playerSize; i++) {
      PlayersTurn.emplace_back(playersName[i], std::vector<Card>{},
                               poker::STARTING_CHIPS, Blind::notBlind);
    }
  }

  void startGame() {
    int config;
    std::cout << "Enter configuration for the game. 0: default, 1: custom\n";
    std::cin >> config;
    if (config == 0) {
      defaultConfig();
    } else {
      customConfig();
    }
  }

  void printGameProperties() {
    std::cout << "Pot: " << pot << "\n"
              << "Player size: " << playerSize << "\n"
              << "Min bet: " << minimumBet << "\n";
  }

  bool playStage() {
    std::cout << "Active players " << countActivePlayers() << std::endl;
    printGameState(state);
    dealCards();
    letPlayerstakeAction();
    // If only one player remains, winner is decided
    if (countActivePlayers() <= 1) {
      handleEarlyWinner();
      return true; // signals round ended
    }
    nextState();
    return false; // signals continue
  }

  void simulateRound() {
    resetBlindsandSetBlinds();
    collectBlindBets();

    // Start
    state = gameState::preFlop;
    if (playStage())
      return; // preFlop done
    if (playStage())
      return; // flop done
    if (playStage())
      return; // turn done
    if (playStage())
      return; // river done

    // Showdown
    givePotToLastStandingOrShowdown();
    resetCards();
    dealerPosition = (dealerPosition + 1) % PlayersTurn.size();
  }

  void runMultipleGames(int numberOfRounds) {
    for (int i = 0; i < numberOfRounds; ++i) {
      if (countActivePlayers() < 2) {
        std::cout << "Not enough players left to continue.\n";
        break;
      }
      std::cout << "\n=== Start of Hand #" << (i + 1) << " ===\n";
      simulateRound();

      // Re-shuffle deck
      deck.resetDeck();
      deck.shuffleDeck();
    }
  }
};

int main() {
  Game game;
  game.startGame();
  game.initalizePLayers();

  // Example: run 5 rounds
  game.runMultipleGames(5);

  return 0;
}
