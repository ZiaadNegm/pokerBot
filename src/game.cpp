#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <system_error>
#include <unordered_map>
#include <vector>
import cards;
import player;

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
  int getUserOption(Player &player) {
    int option = 0;
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
    return option;
  }

  int getBotOption(Player &player, position pos) {
    int option = 1 + (rand() % 5);
    std::cout << "[" << player.getName() << " (BOT)] at Position " << pos
              << " with cards " << player.getHand()[0].cardToString() << " and "
              << player.getHand()[1].cardToString() << std::endl
              << " chooses option " << option << "\n";
    return option;
  }

  // Helper function to execute Call action
  actions executeCall(Player &player, money toCall, money &pot,
                      money &highestBettedInRound) {
    pot += player.bet(toCall);
    std::cout << "[" << player.getName() << "] calls " << toCall << " chips.\n";
    if (toCall > highestBettedInRound) {
      highestBettedInRound = toCall;
    }
    return actions::call;
  }

  // Function to handle human player's decision to Fold or All-In when calling
  actions handleHumanFoldOrAllInCall(Player &player, money toCall, money &pot,
                                     money &highestBettedInRound) {
    while (true) {
      std::cout << "You only have " << player.getChips() << " chips, but need "
                << toCall << " to call.\nPick action:\n"
                << "1. Fold\n"
                << "2. Go All-In\n> ";
      int choice;
      std::cin >> choice;

      if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input.\n";
        continue;
      }

      if (choice == 1) {
        player.fold();
        std::cout << "[" << player.getName() << "] folds.\n";
        return actions::fold;
      } else if (choice == 2) {
        return executeAllIn(player, pot, highestBettedInRound);
      } else {
        std::cout << "Invalid choice. Please try again.\n";
      }
    }
  }

  // Function to handle bot player's decision to Fold or All-In when calling
  actions handleBotFoldOrAllInCall(Player &player, money &pot,
                                   money &highestBettedInRound) {
    bool doAllIn = (rand() % 2 == 0); // 50-50 chance
    if (doAllIn) {
      return executeAllIn(player, pot, highestBettedInRound);
    } else {
      player.fold();
      std::cout << "[" << player.getName() << "] folds.\n";
      return actions::fold;
    }
  }

  // Refactored handleCall function
  actions handleCall(Player &player, bool isYou, money &pot,
                     money &highestBettedInRound) {
    money toCall = highestBettedInRound - player.getCurrentBet();

    // 1. If there's no extra to call:
    if (toCall <= 0) {
      std::cout << "[" << player.getName() << "] checks (no extra to call).\n";
      player.check();
      return actions::check;
    }

    // 2. If toCall > player's chips, ask fold/all-in (if human) or do so
    // automatically (if bot)
    if (toCall > player.getChips()) {
      if (isYou) {
        // Handle human player's decision
        return handleHumanFoldOrAllInCall(player, toCall, pot,
                                          highestBettedInRound);
      } else {
        // Handle bot player's decision
        return handleBotFoldOrAllInCall(player, pot, highestBettedInRound);
      }
    }

    // 3. Otherwise, the player can afford to call
    return executeCall(player, toCall, pot, highestBettedInRound);
  }

  // Ensures that the player can't partake in any more rounds untill a new game
  // is started.
  actions handleFold(Player &player) {
    player.fold();
    std::cout << "[" << player.getName() << "] folds.\n";
    return actions::fold;
  }

  actions handleCheck(Player &player, position pos, bool isYou) {
    // Player can get away with betting nothing as no bets are currently placed
    // into the round.
    if (highestBettedInRound == 0) {
      player.check();
      std::cout << "[" << player.getName() << "] checks.\n";
      return actions::check;
    } else { // This player must check.
      std::cout << "A bet has already been made. This player cannot check.\n";
      // Recursively call action again
      return action(player, pos, isYou, pot, highestBettedInRound);
    }
  }

  money promptRaiseAmount(Player &player, bool isYou) {
    money raiseAmount = 0;
    if (!isYou) {
      // Bot picks random raise
      raiseAmount = 10 + (rand() % 20);
    } else {
      std::cout << "[" << player.getName() << "] Raise amount?\n> ";
      std::cin >> raiseAmount;

      if (std::cin.fail() || raiseAmount < 1) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid raise. Defaulting to 1.\n";
        raiseAmount = 1;
      }
    }
    return raiseAmount;
  }

  // Helper function to execute All-In action
  actions executeAllIn(Player &player, money &pot,
                       money &highestBettedInRound) {
    money allin = player.getChips();
    pot += player.bet(allin);
    std::cout << "[" << player.getName() << "] goes ALL-IN with " << allin
              << " chips.\n";
    if (allin > highestBettedInRound) {
      highestBettedInRound = allin;
    }
    return actions::allIn;
  }

  // Function to handle human player's decision to Fold or All-In
  actions handleHumanFoldOrAllIn(Player &player, money totalRaise, money &pot,
                                 money &highestBettedInRound) {
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
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input.\n";
        continue;
      }

      if (choice == 1) {
        player.fold();
        std::cout << "[" << player.getName() << "] folds.\n";
        return actions::fold;
      } else if (choice == 2) {
        return executeAllIn(player, pot, highestBettedInRound);
      } else {
        std::cout << "Invalid choice. Please try again.\n";
      }
    }
  }

  // Function to handle bot player's decision to Fold or All-In
  actions handleBotFoldOrAllIn(Player &player, money &pot,
                               money &highestBettedInRound) {
    bool doAllIn = (rand() % 2 == 0); // 50-50 chance
    if (doAllIn) {
      return executeAllIn(player, pot, highestBettedInRound);
    } else {
      player.fold();
      std::cout << "[" << player.getName() << "] folds.\n";
      return actions::fold;
    }
  }

  // Refactored handleRaiseAllInOrFold function
  actions handleRaiseAllInOrFold(Player &player, bool isYou, money totalRaise,
                                 money &pot, money &highestBettedInRound) {
    if (isYou) {
      // Handle human player's decision
      return handleHumanFoldOrAllIn(player, totalRaise, pot,
                                    highestBettedInRound);
    } else {
      // Handle bot player's decision
      return handleBotFoldOrAllIn(player, pot, highestBettedInRound);
    }
  }

  actions handleRaise(Player &player, bool isYou, money &pot,
                      money &highestBettedInRound) {
    money raiseAmount = promptRaiseAmount(player, isYou);
    money totalRaise =
        (highestBettedInRound - player.getCurrentBet()) + raiseAmount;

    // If totalRaise is more than player’s chips => fold or all-in
    if (totalRaise > player.getChips()) {
      return handleRaiseAllInOrFold(player, isYou, totalRaise, pot,
                                    highestBettedInRound);
    }

    // Otherwise, proceed with the normal raise flow
    pot += player.bet(totalRaise);
    std::cout << "[" << player.getName() << "] raises " << raiseAmount << ".\n";
    if (totalRaise > highestBettedInRound) {
      highestBettedInRound = totalRaise;
    }
    return actions::raise;
  }

  actions handleAllIn(Player &player, money &pot, money &highestBettedInRound) {
    money allinAmount = player.getChips();

    // Player bets all they have into the pot
    pot += player.bet(allinAmount);

    std::cout << "[" << player.getName() << "] goes ALL-IN with " << allinAmount
              << " chips.\n";

    if (allinAmount > highestBettedInRound) {
      highestBettedInRound = allinAmount;
    }

    return actions::allIn;
  }

  actions action(Player &player, position pos, bool isYou, money &pot,
                 money &highestBettedInRound) {
    printCommunityCards();

    // Distinguish user vs. bot logic to get the option
    int option = isYou ? getUserOption(player) : getBotOption(player, pos);

    switch (option) {
    case 1: // Fold
      return handleFold(player);

    case 2: // Check
      return handleCheck(player, pos, isYou);

    case 3: // Call
      // handleCall version you already fixed to match the switch logic
      return handleCall(player, isYou, pot, highestBettedInRound);

    case 4: // Raise
      return handleRaise(player, isYou, pot, highestBettedInRound);

    case 5: // All-In
      return handleAllIn(player, pot, highestBettedInRound);

    default:
      // Default to fold if something weird happens
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
      act = action(currentPlayer, pos, isYou, pot, highestBettedInRound);

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

  vector<Card> getCommunityCards() { return communityCards; }

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

  void debugAll() {
    std::cout << "Active players: " << countActivePlayers() << "\n";
    printGameState(state);
    std::cout << "Pot: " << pot << "\n"
              << "Player size: " << playerSize << "\n"
              << "Min bet: " << minimumBet << "\n";
  }

  bool playStage() {
    debugAll();
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
