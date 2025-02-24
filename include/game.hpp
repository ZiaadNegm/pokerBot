#ifndef GAME_HPP
#define GAME_HPP

#include "bestHand.hpp"
#include "cards.hpp"
#include "player.hpp"

#include <algorithm>
#include <assert.h>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define INVALID_POS INT_MIN
#define AMOUNT_OF_CARDS 2

using money = std::uint32_t;
using playersPool = std::deque<std::shared_ptr<Player>>;
using notActivePlayers = std::vector<std::shared_ptr<Player>>;
using position = std::int32_t;

enum class actions { fold, check, call, raise, allIn, bet };
enum class gameStates { preFlop, flop, turn, river, showDown };

static const std::map<actions, std::string> actionmessages = {
    {actions::fold, "Fold your hand"},
    {actions::check, "Check and pass"},
    {actions::call, "Call the current bet"},
    {actions::raise, "Raise the stakes with at least: "},
    {actions::allIn, "Go all in with at least: "},
    {actions::bet, "Place a bet with at least: "}};

using actionMap = std::map<actions, std::pair<bool, money>>;

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
  // Type aliases
  using stateHandler = std::function<void(Game *)>;
  using actionHandler =
      std::function<void(class Game *, std::shared_ptr<Player> &, Action)>;
  static const std::unordered_map<gameStates, stateHandler> stateToFunction;
  static const std::unordered_map<actions, actionHandler> actionToFunction;
  // Members
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
  bool firstIterationOfRound;
  bool firstTime;
  bool killSwitch;
  bool freePassForLeftOfDealer;
  position leftPlayerToDealer;

  // Private methods
  void checkHoleCards();
  int getActivePlayers();
  int getNotFoldedPlayers();
  std::shared_ptr<Player> getNextActivePlayer();
  static int indexOfPlayer(const playersPool &pool,
                           const std::shared_ptr<Player> &p);
  actionMap initializeValidActionMap(std::shared_ptr<Player> &player);
  std::shared_ptr<Player>
  getNextActiveAfter(const std::shared_ptr<Player> &current);
  void printGameState(gameStates state);
  void showTurnInfo(const std::shared_ptr<Player> &currentPlayer);
  void printPlayersTable();
  std::shared_ptr<Player> getNextPlayerInSequence();
  Action offerOptions(actionMap validMoves);
  money promptForActionAmount(actions act, money minAmount);
  Action getInputPlayer(std::vector<actions> offeredOptions,
                        actionMap validMoves);

  // Round/state handling
  void handlePreFlop();
  void handleFlop();
  void handleTurn();
  void handleRiver();
  void handleShowDown();

  // Action validations and executions
  actionMap allValidAction(std::shared_ptr<Player> &player);
  void logActions(std::shared_ptr<Player> player, Action action);
  void fold(std::shared_ptr<Player> &player, Action folded);
  void check(std::shared_ptr<Player> &player, Action checked);
  void call(std::shared_ptr<Player> &player, Action called);
  void raise(std::shared_ptr<Player> &player, Action raised);
  void allIn(std::shared_ptr<Player> &player, Action allIn);
  void bet(std::shared_ptr<Player> &player, Action action);
  void performAction(std::shared_ptr<Player> player, Action actionToExecute);
  Action getActionPlayer(std::shared_ptr<Player> player);

  // Core loop
  void letPlayerstakeAction();
  std::shared_ptr<Player> subRoundHandler();

  // Helper methods
  void resetHand();
  void dealHoleCards();
  void dealCommunityCards();
  void standardStartRoundOperations();
  void calculateBesthand();
  std::shared_ptr<Player> &decideWinner();

public:
  // Public for convenience, but can also be accessed via getters/setters
  whoPlays currentPlays;

  // Constructors
  Game();
  Game(playersPool &players, gameSettings &settings, const positions &pos);

  // Public interface
  void simulateHand();
};

#endif // GAME_HPP
