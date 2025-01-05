#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
import cards;
import player;
#include <deque>
#include <map>
#include <string>
std::string playersName[] = {"Phill", "Doyle",  "Daniel",
                             "Chris", "Johnny", "You"};

using money = std::uint32_t;
using playersPool = std::deque<Player>;

namespace poker {
constexpr size_t MIN_PLAYERS = 2;
constexpr size_t MAX_PLAYERS = 6;
constexpr size_t CARDS_PER_PLAYER = 2;
constexpr money MINIMUM_BET = 1;
constexpr money STARTING_CHIPS = 100;

} // namespace poker

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
  size_t dealerPosition;
  money minimumBet;
  size_t playerSize;

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

  void resetBlindsandSetBlinds() {
    size_t posPreviousSmallBlind = (dealerPosition) % PlayersTurn.size();
    size_t posPreviousBigBlind = (dealerPosition + 1) % PlayersTurn.size();
    size_t nextBigBlind = (posPreviousBigBlind + 1) % PlayersTurn.size();

    Player prevSmallBlind = PlayersTurn[posPreviousBigBlind];
    Player prevBigBlind = PlayersTurn[posPreviousBigBlind];
    Player nextBigBlind = PlayersTurn[nextBigBlind];

    prevSmallBlind.setBlind(Blind::notBlind);
    prevBigBlind.setBlind(Blind::smallBlind);
    nextBigBlind.setBlind(Blind::bigBlind);
  }

  // Calculates and collects the bets of the blinds.
  void collectBlindBets() {
    // Calculates the position based on a queue of the Blinds.
    size_t smallBlindPos = (dealerPosition + 1) % PlayersTurn.size();
    size_t bigBlindPos = (dealerPosition + 2) % PlayersTurn.size();

    // Retrieves the position of the Blinds.
    Player smallBlind = PlayersTurn[smallBlindPos];
    Player bigBlind = PlayersTurn[bigBlindPos];

    // Sets players to their certain Blinds to recongize them later on.
    smallBlind.setBlind(Blind::smallBlind);
    bigBlind.setBlind(Blind::bigBlind);

    // BB and SB bet the required amount.
    smallBlind.bet(minimumBet / 2);
    bigBlind.bet(minimumBet);

    pot += minimumBet * 1.5;
    return;
  }

  void simulateRound() {
    while (!(PlayersTurn.size() == 1)) {
      collectBlindBets();
      adjustBlindpos();
      dealCards();
      letPlayerstakeAction();
    }
    givePotToLastStanding();
    resetCards();
    addEveryoneBack();
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

  // Starts the game.
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
    std::cout << "Pot: " << pot << std::endl
              << "Player size: " << playerSize << std::endl
              << "Min bet: " << minimumBet << std::endl;
  }
};

int main() {
  Game game;
  game.startGame();

  return 0;
}