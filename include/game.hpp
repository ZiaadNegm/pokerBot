#pragma once
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations or imports for your modules
import player;
import cards;
import bestHand;

constexpr int INVALID_POS = INT_MIN;
constexpr int AMOUNT_OF_CARDS = 2;

using money = std::uint32_t;
using playersPool = std::deque<std::shared_ptr<Player>>;
using notActivePlayers = std::vector<std::shared_ptr<Player>>;
using position = std::int32_t;

enum class actions { fold, check, call, raise, allIn, bet };
extern const std::map<actions, std::string> actionmessages;

using actionMap = std::map<actions, std::pair<bool, money>>;

enum class gameStates { preFlop, flop, turn, river, showDown };

struct Action {
  actions action;
  money bet;
  size_t roundCounter;
};
using playersHistory = std::pmr::unordered_map<int, Action>;

struct gameSettings {
  size_t minAmountPlayers = 2;
  size_t maxAmountPlayers = 6;
  money minBet = 10;
  money startingChips = 100;
  size_t maximumRounds = 10;
};

struct positions {
  position dealerPosition = 0;
  position posBB = 1;
  position posSB = 2;
};

struct Pot {
  money total;
  std::vector<std::shared_ptr<Player>> contributors;
};

struct whoPlays {
  position ActionTaker;
  position LastTurnPlayer;
};

class Game {
private:
  using stateHandler = std::function<void(Game *)>;

  money pot;
  playersPool players;
  Deck deck;
  std::vector<Card> communityCards;
  money highestBet;
  gameStates gameState;
  gameSettings settings;
  money raiseAmount;
  positions gamePositions;
  bool aBetHasBeenPlaced;
  size_t currentRound;

  using actionHandler =
      std::function<void(Game *, std::shared_ptr<Player> &, Action)>;
  std::unordered_map<actions, actionHandler> actionToFunction;
  std::unordered_map<gameStates, stateHandler> stateToFunction;

  // Private helper methods (e.g., checkHoleCards, dealCommunityCards, etc.)...
  // ...

public:
  whoPlays currentPlays;

  Game();
  Game(playersPool &players, gameSettings &settings, const positions &pos);

  void fold(std::shared_ptr<Player> &player, Action folded);
  void check(std::shared_ptr<Player> &player, Action checked);
  void call(std::shared_ptr<Player> &player, Action called);
  void raise(std::shared_ptr<Player> &player, Action raised);
  void allIn(std::shared_ptr<Player> &player, Action allIn);
  void bet(std::shared_ptr<Player> &player, Action action);

  std::shared_ptr<Player> getNextPlayerInSequence();
  Action offerOptions(actionMap validMoves);
  money promptForActionAmount(actions act, money minAmount);
  Action getInputPlayer(std::vector<actions> offeredOptions,
                        actionMap validMoves);

  void handlePreFlop();
  void handleFlop();
  void handleTurn();
  void handleRiver();
  void handleShowDown();

  void letPlayerstakeAction();
  void logActions(std::shared_ptr<Player> player, Action action);
  actionMap allValidAction(std::shared_ptr<Player> &player);
  void performAction(std::shared_ptr<Player> player, Action actionToExecute);

  void resetHand();
  void standardStartRoundOperations();
  void dealHoleCards();
  void dealCommunityCards();
  void calculateBesthand();
  std::shared_ptr<Player> decideWinner();
  void simulateHand();
  void decidePlayersLifeCycle();
};
