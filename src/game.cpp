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
  size_t SBIndex;
  size_t BBIndex;

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

  // Sets and Removes the BB and SB for a round.
  void resetBlindsandSetBlinds() {
    size_t posPreviousSmallBlind = (dealerPosition) % PlayersTurn.size();
    size_t posPreviousBigBlind = (dealerPosition + 1) % PlayersTurn.size();
    size_t nextBigBlind = (posPreviousBigBlind + 1) % PlayersTurn.size();

    PlayersTurn[posPreviousBigBlind].setBlind(Blind::smallBlind);
    PlayersTurn[posPreviousSmallBlind].setBlind(Blind::notBlind);
    PlayersTurn[nextBigBlind].setBlind(Blind::bigBlind);

    SBIndex = posPreviousBigBlind;
    BBIndex = nextBigBlind;
    return;
  }

  // Calculates and collects the bets of the blinds.
  void collectBlindBets() {

    Player SB = PlayersTurn[SBIndex];
    Player BB = PlayersTurn[BBIndex];

    // BB and SB bet the required amount.
    SB.bet(minimumBet / 2);
    BB.bet(minimumBet);

    pot += minimumBet * 1.5;
    return;
  }

  void dealHoleCards() {
    for (int round = 0; round < 2; ++round) {
      size_t currentIndex = dealerPosition;
      do {
        Card dealtCard = deck.dealCard();

        std::vector<Card> currentHand = PlayersTurn[currentIndex].getHand();
        currentHand.push_back(dealtCard);

        PlayersTurn[currentIndex].receiveCards(currentHand);

        currentIndex = (currentIndex + 1) % PlayersTurn.size();

      } while (currentIndex != dealerPosition);
    }
  }

  // Deals quantityCards amount of communityCards to.
  void dealCommunityCards(int quantityCards) {
    deck.burnCard();
    for (int i = 0; i < quantityCards; i++) {
      communityCards.emplace_back(deck.dealCard());
    }
    return;
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

  void simulateRound() {
    resetBlindsandSetBlinds();
    collectBlindBets();
    while (!(PlayersTurn.size() == 1)) {
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