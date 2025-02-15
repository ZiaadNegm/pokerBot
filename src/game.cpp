#include <algorithm>
#include <assert.h>
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
import player;
import cards;
import bestHand;
#define INVALID_POS INT_MIN
#define AMOUNT_OF_CARDS 2

using money = std::uint32_t;
using playersPool = std::deque<std::shared_ptr<Player>>;
using notActivePlayers = std::vector<std::shared_ptr<Player>>;
using position = std::int32_t;
enum class actions { fold, check, call, raise, allIn, bet };

static const std::map<actions, std::string> actionmessages = {
    {actions::fold, "Fold your hand"},
    {actions::check, "Check and pass"},
    {actions::call, "Call the current bet"},
    {actions::raise, "Raise the stakes with atleast: "},
    {actions::allIn, "Go all in with atleast: "},
    {actions::bet, "Place a bet with atleast: "}};

using actionMap = std::map<actions, std::pair<bool, money>>;

std::vector<std::string> names = {"Ziaad", "Daaiz"};

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
  money raiseAmount = settings.minBet;
  positions gamePositions;
  bool aBetHasBeenPlaced;
  size_t currentRound = 0;

  struct whoPlays {
    position ActionTaker;
    position LastTurnPlayer;
  };

  // Define an alias for functions that handle actions.
  using actionHandler =
      std::function<void(Game *, std::shared_ptr<Player> &, Action)>;

  std::unordered_map<actions, actionHandler> actionToFunction = {
      {actions::fold,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->fold(player, action); })},
      {actions::check,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->check(player, action); })},
      {actions::call,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->call(player, action); })},
      {actions::raise,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->raise(player, action); })},
      {actions::allIn,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->allIn(player, action); })},
      {actions::bet,
       static_cast<actionHandler>(
           [this](Game *game, std::shared_ptr<Player> &player,
                  Action action) -> void { this->bet(player, action); })}};

  std::unordered_map<gameStates, stateHandler> stateToFunction = {
      {gameStates::preFlop, [](Game *g) { g->handlePreFlop(); }},
      {gameStates::flop, [](Game *g) { g->handleFlop(); }},
      {gameStates::turn, [](Game *g) { g->handleTurn(); }},
      {gameStates::river, [](Game *g) { g->handleRiver(); }},
      {gameStates::showDown, [](Game *g) { g->handleShowDown(); }}};

  void checkHoleCards() {
    for (auto player : players) {
      std::cout << "Player " << player->getName() << " Has the following cards"
                << "\n";
      for (Card card : player->getHand()) {
        std::cout << " " << card.cardToString() << "\n";
      }
      std::cout << "\n";
    }
  }

  int getActivePlayers() {
    int activePlayers = 0;
    for (auto &player : players) {
      if (player->getIsActive()) {
        activePlayers++;
      }
    }
    return activePlayers;
  }

  int getNotFoldedPlayers() {
    int AmountOfFoldedPlayers = 0;
    for (auto &player : players) {
      if (player->getIsActive() && player->hasPlayerFolded()) {
        AmountOfFoldedPlayers++;
      }
    }
    return getActivePlayers() - AmountOfFoldedPlayers;
  }

  /* Returns the player who is active and hasn't folded.
   */
  std::shared_ptr<Player> getNextActivePlayer() {
    size_t pos = (currentPlays.LastTurnPlayer + 1) % players.size();
    std::shared_ptr<Player> player = players[pos];
    while (!player->getIsActive() || player->hasPlayerFolded()) {
      pos = (pos + 1) % players.size();
    }
    return players[pos];
  }

  static int indexOfPlayer(const playersPool &pool,
                           const std::shared_ptr<Player> &p) {
    for (int i = 0; i < static_cast<int>(pool.size()); i++) {
      if (pool[i] == p) {
        return i;
      }
    }
    return -1;
  }

  actionMap initializeValidActionMap(std::shared_ptr<Player> &player) {
    actionMap validActionMap;
    for (int i = 0; i <= static_cast<int>(actions::bet); i++) {
      actions currentAction = static_cast<actions>(i);
      validActionMap[currentAction] = std::make_pair(false, 0);
    }
    // Set Fold always valid with 0 money
    validActionMap[actions::fold] = {true, 0};
    // Set Allin valid with money equal to player's chips
    validActionMap[actions::allIn] = {true, player->getChips()};
    return validActionMap;
  }

  // 1. Helper to find the next active player after 'current'
  std::shared_ptr<Player>
  getNextActiveAfter(const std::shared_ptr<Player> &current) {
    int idx = indexOfPlayer(players, current);
    if (idx == -1)
      return nullptr;

    size_t nextPos = (idx + 1) % players.size();
    auto candidate = players[nextPos];

    // loop until we find an active, non-folded player or come full circle
    while (!candidate->getIsActive() || candidate->hasPlayerFolded()) {
      nextPos = (nextPos + 1) % players.size();
      candidate = players[nextPos];
      if (candidate == current) {
        return nullptr;
      }
    }

    if (candidate == current) {
      return nullptr;
    }

    return candidate;
  }

  void showTurnInfo(const std::shared_ptr<Player> &currentPlayer) {
    // Print the entire table first (so the user sees all players’ states)
    std::cout << "\n\n";
    printPlayersTable();

    // Now show the current player's turn block
    std::cout << "\n=============================================\n";
    std::cout << "It's " << currentPlayer->getName() << "'s turn!\n";
    std::cout << currentPlayer->getName() << " has "
              << currentPlayer->getChips() << " chips.\n";

    // Show hole cards
    std::cout << "Hole Cards:\n";
    for (const auto &card : currentPlayer->getHand()) {
      std::cout << "  " << card.cardToString() << "\n";
    }

    // Who is next?
    auto next = getNextActiveAfter(currentPlayer);
    if (next && next != currentPlayer) {
      std::cout << "Next to act: " << next->getName() << "\n";
    } else {
      std::cout << "No other active players.\n";
    }

    std::cout << "Highest bet so far: " << highestBet << "\n";
    std::cout << "Pot: " << pot << "\n";

    std::cout << "=============================================\n\n";
  }

  void printPlayersTable() {
    // Print the header
    std::cout << "#   Name        Chips     Blind       Status\n";
    std::cout << "------------------------------------------\n";

    int playerNumber = 1;
    for (const auto &player : players) {
      // Determine blind status as a string
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

      // Determine status as a string
      std::string status;
      if (!player->getIsActive()) {
        status = (player->getChips() == 0 ? "Out of Chips" : "Inactive");
      } else {
        status = (player->hasPlayerFolded() ? "Folded" : "Active");
      }

      // Print row
      std::cout << std::left << std::setw(4) << playerNumber << std::setw(12)
                << player->getName() << std::setw(10) << player->getChips()
                << std::setw(12) << blindStatus << std::setw(10) << status
                << std::endl;
      playerNumber++;
    }
    std::cout << "------------------------------------------\n";
  }

public:
  whoPlays currentPlays;
  Game()
      : pot(0), players(), deck(), communityCards(), highestBet(0),
        gameState(gameStates::preFlop), settings(), gamePositions() {}

  Game(playersPool &players, gameSettings &settings, const positions &pos)
      : players(players), settings(settings), gamePositions(pos), pot(0),
        highestBet(0), deck(), communityCards({}),
        gameState(gameStates::preFlop),
        currentPlays(gamePositions.posSB, gamePositions.posSB + 1) {}

  /* The players folded property is set to true.
   * The effects of this will be executed in other functions while the game is
   * running.
   */
  void fold(std::shared_ptr<Player> &player, Action folded) {
    player->setHasFolded(true);
    logActions(player, folded);
    return;
  }

  /* The player checks. We assume that the player can validly check, to ensure
   * this, this must be checked in different parts of game.
   */
  void check(std::shared_ptr<Player> &player, Action checked) {
    player->check();
    logActions(player, checked);
    return;
  }

  /* The player matches his bet with the current highest bet.
   * We assume here that the player has enough to call. This is left to other
   * function that check this.
   */
  void call(std::shared_ptr<Player> &player, Action called) {
    player->call(called.bet);

    pot += called.bet;

    logActions(player, called);
    return;
  }

  /* The player bets a amount which is greater than the current highest bet.
   * We assume that this player can actually raise, we leave this to other
   * functions.
   */
  void raise(std::shared_ptr<Player> &player, Action raised) {
    player->raise(raised.bet);

    highestBet = raised.bet;

    pot += raised.bet;

    logActions(player, raised);
    return;
  }

  /* All remaining chips go to the pot
   */
  void allIn(std::shared_ptr<Player> &player, Action allIn) {
    player->bet(allIn.bet);

    pot += allIn.bet;

    if (allIn.bet > highestBet) {
      highestBet = allIn.bet;
    }

    logActions(player, allIn);
    return;
  }

  /* This player bets x.
   * We leave it to other functions to ensure that a player can only bet when no
   * one has placed a bet in that round except the SB and BB. This is most
   * likely the player after the SB.
   */
  void bet(std::shared_ptr<Player> &player, Action action) {
    player->bet(action.bet);

    pot += action.bet;
    aBetHasBeenPlaced = true;

    logActions(player, action);
  }

  /* Decides which player plays next.
   * This should decide who the actionTaker is and check if we are already at
   * that actiontaker.
   * Maybe return a pointer to the player who should play?
   * If the returned pointer is NULL. There should be no next player.
   * Note that this function relies on the caller to reset the actionTaker
   * when we did encounter a NULL return.
   */
  std::shared_ptr<Player> getNextPlayerInSequence() {

    std::shared_ptr<Player> nextPlayer = getNextActivePlayer();
    if (nextPlayer == (players[currentPlays.ActionTaker])) {
      return nullptr;
    }
    return nextPlayer;
  };

  Action offerOptions(actionMap validMoves) {
    std::vector<actions> offeredOptions;
    offeredOptions.reserve(validMoves.size());

    std::cout << "Available actions:\n";
    int optionCounter = 1;
    for (const auto &[act, pairVal] : validMoves) {
      if (pairVal.first) {
        auto it = actionmessages.find(act);
        std::string actionText =
            (it != actionmessages.end()) ? it->second : "Unknown action";
        std::cout << optionCounter << ") " << actionText
                  << " [Amount: " << pairVal.second << "]\n";
        offeredOptions.push_back(act);
        optionCounter++;
      }
    }

    return getInputPlayer(offeredOptions, validMoves);
  }

  // Add helper function to prompt user for an amount for bet/raise actions.
  money promptForActionAmount(actions act, money minAmount) {
    std::string actionName = (act == actions::bet) ? "bet" : "raise";
    std::cout << "You can " << actionName << " from " << minAmount
              << " chips.\n";
    std::cout << "How much would you like to " << actionName << "? ";

    money inputAmount;
    std::cin >> inputAmount;

    while (std::cin.fail() || inputAmount < minAmount) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout
          << "Invalid amount. Please enter an amount greater than or equal to "
          << minAmount << ": ";
      std::cin >> inputAmount;
    }

    return inputAmount;
  }

  Action getInputPlayer(std::vector<actions> offeredOptions,
                        actionMap validMoves) {
    std::cout << "Enter the number of your choice: ";
    int choice = 0;
    std::cin >> choice;

    if (choice < 1 || choice > static_cast<int>(offeredOptions.size())) {
      std::cout << "Invalid choice, defaulting to fold.\n";
      return Action{actions::fold, 0, 0};
    }

    actions selected = offeredOptions[choice - 1];
    money chosenAmount = validMoves[selected].second;

    // For bet and raise, allow custom amount input starting from the minimum.
    if (selected == actions::bet || selected == actions::raise) {
      chosenAmount = promptForActionAmount(selected, chosenAmount);
    }

    return Action{selected, chosenAmount, currentRound};
  }

  void logActions(std::shared_ptr<Player> player, Action action) {
    std::string playerName = player->getName();
    switch (action.action) {
    case actions::fold:
      std::cout << "[" << playerName << "] folds.\n";
      break;
    case actions::check:
      std::cout << "[" << playerName << "] checks.\n";
      break;
    case actions::call:
      std::cout << "[" << playerName << "] calls with " << action.bet
                << " chips.\n";
      break;
    case actions::raise:
      std::cout << "[" << playerName << "] raises by " << action.bet
                << " chips.\n";
      break;
    case actions::allIn:
      std::cout << "[" << playerName << "] goes all-in with " << action.bet
                << " chips.\n";
      break;
    case actions::bet:
      std::cout << "[" << playerName << "] places a bet of " << action.bet
                << " chips.\n";
      break;
    default:
      std::cout << "[" << playerName << "] performs an unknown action.\n";
      break;
    }
  }

  /* This function handles the preflop round.
   * We iterate over the players untill getNextPlayerInSequence determines that
   * we can stop. For each player we get all valid actions from a helper
   * function in the form of a map from a action to a bool and a amount.
   * We then offer and execute the action.
   */

  // Note: We seem to stop at player = 0 while condition =  1;
  void handlePreFlop() {
    aBetHasBeenPlaced = true;
    letPlayerstakeAction();
    return;
  }

  void handleFlop() {
    aBetHasBeenPlaced = false;
    dealCommunityCards();
    letPlayerstakeAction();
    return;
  }
  void handleTurn() {
    aBetHasBeenPlaced = false;
    dealCommunityCards();
    letPlayerstakeAction();
    return;
  }

  void handleRiver() {
    aBetHasBeenPlaced = false;
    dealCommunityCards();
    letPlayerstakeAction();
    return;
  }

  void handleShowDown() {
    calculateBesthand();
    return;
  }

  /* Returns what the player can play, what is valid. This is done in the form
   * of a map:
   *  [action] -> {valid?, Amount}.
   */
  actionMap allValidAction(std::shared_ptr<Player> &player) {
    auto validActionMap = initializeValidActionMap(player);

    // can only check if a bet has been placed. in PR, always true.
    if (!aBetHasBeenPlaced) {
      validActionMap[actions::check] = {true, 0};
    }

    if ((aBetHasBeenPlaced) && ((player->getChips()) >= highestBet)) {
      validActionMap[actions::call] = {true, highestBet};
    }

    if ((aBetHasBeenPlaced) &&
        ((player->getChips()) >= raiseAmount + highestBet)) {
      validActionMap[actions::raise] = {true, raiseAmount + highestBet};
    }

    if ((aBetHasBeenPlaced == false) &&
        (player->getChips() >= settings.minBet)) {
      validActionMap[actions::bet] = {true, settings.minBet};
    }

    // std::cout << "\n";
    // for (const auto &[act, pr] : validActionMap) {
    //   std::cout << "Action: " << static_cast<int>(act)
    //             << " Valid: " << std::boolalpha << pr.first
    //             << " Amount: " << pr.second << "\n";
    // }
    return validActionMap;
  }

  /* Actually calls the action for the player which is validated through the
   * given input.
   */
  void performAction(std::shared_ptr<Player> player, Action actionToExecute) {
    auto actionHandler = actionToFunction[actionToExecute.action];
    actionHandler(this, player, actionToExecute);
  }

  /* offers certain options to the player. The player gives his option as input.
   * We return this input as a action type.
   */
  Action getActionPlayer(std::shared_ptr<Player> player) {
    actionMap validMoves = allValidAction(player);
    return offerOptions(validMoves);
  }

  /* Gets, validates and performs action
   * Possible input player?
   * Calls getNextPlayerInSequence()
   * Calculate all valid actions for player.
   * Offer these options
   * Gets the action from getActionPlayer().
   * performs these actions.
   * Maybe return a pointer to the winner?
   *
   */
  void letPlayerstakeAction() {
    std::shared_ptr<Player> player;
    currentRound++;
    while ((player = getNextPlayerInSequence()) != nullptr) {
      currentPlays.LastTurnPlayer = indexOfPlayer(players, player);
      showTurnInfo(player);
      Action action = getActionPlayer(player);
      performAction(player, action);
    }
  }

  /* Will walk through all gameStates and update these states.
   * The stop condition can be:
   *  either we reached the end of showDown and we calculate a winner
   * ||
   * Everyone except one person folded.
   *
   * Calls getNextPlayerInSequence()
   * Calculate all valid actions for player.
   * Offer these options
   * Gets the action from getActionPlayer().
   * performs these actions.
   * Maybe return a pointer to the winner?
   */
  std::shared_ptr<Player> subRoundHandler() {
    while (getNotFoldedPlayers() > 1) {
      auto handler = stateToFunction[gameState];
      handler(this);
      if (gameState != gameStates::showDown) {
        gameState = static_cast<gameStates>(static_cast<int>(gameState) + 1);
      }
    }
    return decideWinner();
  }

  /* Calls the players method resetHand to give a clean sheet for the next
   * round.
   */
  void resetHand() {
    for (auto &player : players) {
      player->resetCards();
      player->setHasFolded(false);
    }
    return;
  }

  /* Will function as the entre point for a round.
   * Calls standardStartRoundOperations.
   * Calls subRoundHandler.
   * Give the pot to the winner.
   * Reset function? s.a cards cleaned up, folded status reset.
   */
  void simulateHand() {
    standardStartRoundOperations();
    checkHoleCards();
    std::cout << "Finished checking HoleCards"
              << "\n";
    subRoundHandler();
    resetHand();
  }

  /* Decides whether a player should be excluded from this round.
   * For example player is out of chips or player has folded.
   * Maybe return a bool or change the valid property of the player?
   */
  void decidePlayersLifeCycle() {}

  /* Deals 2 cards to every player who is active in this round
   */
  void dealHoleCards() {
    for (int i = 0; i < AMOUNT_OF_CARDS; i++) {
      for (auto &player : players) {
        if (player->getIsActive()) {
          Card toBeDealt = deck.dealCard();
          player->receiveCards(toBeDealt);
        }
      }
    }
    return;
  }

  /* Depending on the game state, adds cards to communityCards.
   */
  void dealCommunityCards() {
    int cardsTobeDealt = 1;
    if (gameState == gameStates::flop) {
      cardsTobeDealt = 3;
    } else if (gameState == gameStates::preFlop) {
      cardsTobeDealt = 0;
    }

    deck.burnCard();
    for (int i = 0; i < cardsTobeDealt; i++) {
      Card communityCard = deck.dealCard();
      communityCards.push_back(communityCard);
    }
    return;
  }

  /* Does the basic one time operations needed in a round.
   * Make the Blinds pay.
   * Deal hole cards to players in the game.
   */
  void standardStartRoundOperations() {
    auto &smallBlindPlayer = players[gamePositions.posSB];
    auto &bigBlindPlayer = players[gamePositions.posBB];

    if (smallBlindPlayer->getChips() <
        static_cast<money>(0.5 * settings.minBet)) {
      Action allInAction = {
          actions::allIn, static_cast<money>(smallBlindPlayer->getChips()), 0};
      allIn(smallBlindPlayer, allInAction);
    } else {
      Action betAction = {actions::bet,
                          static_cast<money>(settings.minBet * 0.5), 0};
      bet(smallBlindPlayer, betAction);
    }
    if (smallBlindPlayer->getChips() < settings.minBet) {
      Action allInAction = {actions::allIn, bigBlindPlayer->getChips(), 0};
      highestBet = bigBlindPlayer->getChips();
      allIn(bigBlindPlayer, allInAction);
    } else {
      Action betAction = {actions::bet, settings.minBet, 0};
      highestBet = settings.minBet;
      bet(bigBlindPlayer, betAction);
    }

    dealHoleCards();

    return;
  }

  /* Calculates which player has the best hand. Sets all other playes
   * besides this player to inactive.
   * Does this with the help of the module bestHand.cppm
   */
  void calculateBesthand() {
    std::vector<std::shared_ptr<Player>> playerVector(players.begin(),
                                                      players.end());

    // 2. Create the determineBestHand object
    determineBestHand bestHandCalculator(playerVector, communityCards);

    // 3. Determine the winner
    std::shared_ptr<Player> winner =
        bestHandCalculator.determineWinnerByHighestCard();
    std::cout << "The best hand winner is: " << winner->getName() << "\n";

    // 4. Fold every other active player
    for (auto &p : players) {
      if (p != winner && p->getIsActive() && !p->hasPlayerFolded()) {
        p->setHasFolded(true);
      }
    }
  }

  /* Returns the player who won the game.
   * This is thus the only player who hasn't been marked as folded.
   */
  std::shared_ptr<Player> &decideWinner() {
    static std::shared_ptr<Player> nullPlayer = nullptr;

    for (auto &player : players) {
      if (player->getIsActive() && !player->hasPlayerFolded()) {
        return player;
      }
    }
    return nullPlayer;
  }
};

class ManagerTest;

class Manager {
private:
  gameSettings settings;            // a struct containing all default settings
  std::vector<std::string> names;   // Default names for six players.
  notActivePlayers inActivePlayers; // All players who have 0 chips and cannot
                                    // play anymore
  playersHistory History;           // The history of each player.
  size_t currentRound;
  positions specialPositions; // struct containing position of BB SB and Dealer.
  bool gameActive;

  friend class ManagerTest;

  int activePlayers() {
    int activeCount = 0;
    for (auto &player : players) {
      if (player->getIsActive()) {
        activeCount++;
      }
    }
    return activeCount;
  }

public:
  Game game;           //  The game containing all the core logic.
  playersPool players; // a deque with shared pointers to each player.
  Manager()
      : game(), names({"Phill", "Doyle", "Daniel", "Chris", "Johnny", "You"}),
        currentRound(0) {
    initalizePLayers();
  }

  void log(const std::string &message) const {
    std::cout << message << std::endl;
  }

  /* Error catching function. (Later. For now names is a static defined
   * array.) When too few names provided, ask input to fill in missingAmount
   * of names.
   */
  void handleTooFewNamesProvided(int missingAmount) {}

  /* Initalizes the SB BB and D positions. This can't be reduced to a default
   * value init as in the begin of the game we might already have inactive
   * players.
   */
  void initalizeSpecialPositions() {
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

  /* Initalizes all players with their names and starting chips.
   * These players get allocated on the heap with shared pointers and pushed
   * onto the playersPool
   */
  void initalizePLayers() {

    if (names.size() < settings.minAmountPlayers) {
      handleTooFewNamesProvided(names.size() - settings.minAmountPlayers);
    }

    for (int i = 0; i < names.size() && i < settings.maxAmountPlayers; i++) {
      std::shared_ptr<Player> p =
          make_shared<Player>(names[i], settings.startingChips);
      players.emplace_back(p);
    }
    initalizeSpecialPositions();
  }

  /* Return the special position by pointer.
   */
  std::shared_ptr<const positions> getSpecialPositions() {
    return std::make_shared<positions>(specialPositions);
  }

  size_t getCurrentRound() { return currentRound; }

  /* Will print out the current round, name, chips, and activity status in a
   * nice format.
   */
  void gameStatistics() {
    std::cout << "==========================================" << std::endl;
    std::cout << "           Poker Game Statistics          " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Current Round: " << currentRound << std::endl << std::endl;

    // Print the header with fixed column widths
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

      // Print player information with proper alignment
      std::cout << std::left << std::setw(4) << playerNumber << std::setw(12)
                << player->getName() << std::setw(10) << player->getChips()
                << std::setw(12) << blindStatus << std::setw(10) << status
                << std::endl;
      playerNumber++;
    }

    std::cout << "------------------------------------------" << std::endl;
  }

  /* Will simulate k amount of hands by letting the Game class run the logic
   * behind that. Maybe a for loop for k amounts of reps, call
   * game.simulateHand()? At the end set the gameActive variable to false?
   */
  void startGame() {
    for (int i = 0; i < settings.maximumRounds; i++) {
      Game game(players, settings, specialPositions);
      game.simulateHand();
      decidePlayersLifeCycle();
      arrangePlayersPosition();
      gameStatistics();
    }

    endGame();
  }

  /* Will print out all players with their chips etc.
   * Indicates the end of the program.
   */
  void endGame() {
    std::cout << "==========================================" << std::endl;
    std::cout << "                Game Over                  " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Final Standings:" << std::endl;

    std::cout << std::left << std::setw(4) << "#" << std::setw(15) << "Name"
              << std::setw(10) << "Chips" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Convert deque to vector using vector's range constructor
    std::vector<std::shared_ptr<Player>> sortedPlayers(players.begin(),
                                                       players.end());

    std::sort(sortedPlayers.begin(), sortedPlayers.end(),
              [](const std::shared_ptr<Player> &a,
                 const std::shared_ptr<Player> &b) -> bool {
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

    // Identify the winner(s)
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

      // Announce the winner(s)
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

  /* Error catching function. We can't assign the same player to be a SB and a
   * BB. The game has to terminate as something clearly went wrong.
   */
  void handleTooFewPlayers() {
    throw std::runtime_error("Too few players to continue the game.");
  }

  position findNextValidPos(position start) {
    // Count active players
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

  /* For a betRound, move the dealer button. This will indirectly move the SB
   * and BB
   */
  void arrangePlayersPosition() {
    if (activePlayers() < 3) {
      return handleTooFewPlayers();
    }

    // Clear all blinds first
    for (auto &player : players) {
      player->setBlind(Blind::notBlind);
    }

    // Set new positions with validation
    position newDealerIndex = findNextValidPos(
        (specialPositions.dealerPosition + 1) % players.size());
    if (newDealerIndex == INVALID_POS) {
      handleTooFewPlayers();
      return;
    }

    position newSBIndex =
        findNextValidPos((newDealerIndex + 1) % players.size());
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

    // Set new positions and blinds
    specialPositions.dealerPosition = newDealerIndex;
    specialPositions.posSB = newSBIndex;
    specialPositions.posBB = newBBIndex;

    players[newDealerIndex]->setBlind(Blind::dealer);
    players[newSBIndex]->setBlind(Blind::smallBlind);
    players[newBBIndex]->setBlind(Blind::bigBlind);
  }

  /* If a player has no chips more, remove them from the game as they can't do
   * anyhting. This will ensure that no funny things happen when moving around
   * the dealersButton.
   * We set the player to inActive.
   */
  void decidePlayersLifeCycle() {
    for (auto &player : players) {
      if (player->getChips() == 0) {
        player->setIsActive(false);
        log(player->getName() + "is out of chips and inactive. ");
      }
    }
  }
};

struct ManagerTest {
  static void printBlindValues(const Manager &manager) {
    std::cout << "Current blind values:\n";
    for (size_t i = 0; i < manager.players.size(); i++) {
      std::cout << "Player " << i << " (" << manager.names[i] << ") blind: "
                << static_cast<int>(manager.players[i]->getBlind())
                << " Active status " << manager.players[i]->getIsActive()
                << "\n";
    }
    std::cout << std::endl;
  }

  /* Helper function to set player activity */
  static void setPlayerActive(Manager &manager, size_t playerIndex,
                              bool isActive) {
    if (playerIndex < manager.players.size()) {
      manager.players[playerIndex]->setIsActive(isActive);
    }
  }

  /* Test Case 1: All Players Active */
  static void testAllPlayersActive() {
    std::cout << "Test Case 1: All Players Active\n";
    Manager manager;

    // Ensure all players are active
    for (size_t i = 0; i < manager.players.size(); i++) {
      manager.players[i]->setIsActive(true);
    }

    // Print blind values
    std::cout << "After initialization:\n";
    printBlindValues(manager);

    // Verify initial positions
    auto initialPos = manager.getSpecialPositions();
    assert(manager.players[initialPos->dealerPosition]->getBlind() ==
           Blind::dealer);
    assert(manager.players[initialPos->posSB]->getBlind() == Blind::smallBlind);
    assert(manager.players[initialPos->posBB]->getBlind() == Blind::bigBlind);

    // Test rotation
    manager.arrangePlayersPosition();

    // Print blind values after rotation
    std::cout << "After rotation:\n";
    printBlindValues(manager);

    // Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 1); // Should move to position 1
    assert(newPos->posSB == 2);          // Should move to position 2
    assert(newPos->posBB == 3);          // Should move to position 3

    std::cout << "Test Case 1 Passed!\n\n";
  }

  /* Test Case 2: Some Players Inactive */
  static void testSomePlayersInactive() {
    std::cout << "Test Case 2: Some Players Inactive\n";
    Manager manager;

    // Initial blind assignment is already done in the Manager constructor via
    // initalizeSpecialPositions()

    // Print initial blind values
    std::cout << "Initial blind assignment:\n";
    printBlindValues(manager);

    // 1. First Rotation: Rotate blinds once
    manager.arrangePlayersPosition();
    std::cout << "After first rotation:\n";
    printBlindValues(manager);

    // 2. Set players 1 and 3 as inactive
    setPlayerActive(manager, 1, false); // Assuming player index 1 is "Doyle"
    setPlayerActive(manager, 3, false); // Assuming player index 3 is "Chris"
    std::cout << "After setting players 1 and 3 as inactive:\n";
    printBlindValues(manager);

    // 3. Second Rotation: Rotate blinds again
    manager.arrangePlayersPosition();
    std::cout << "After second rotation with some players inactive:\n";
    printBlindValues(manager);

    // 4. Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 2); // Should move to position 2 ("Daniel")
    assert(newPos->posSB == 4);          // Should move to position 4 ("Johnny")
    assert(newPos->posBB == 5);          // Should move to position 5 ("You")

    std::cout << "Test Case 2 Passed!\n\n";
  }

  /* Test Case 3: Minimum Active Players (3 Players) */
  static void testMinimumActivePlayers() {
    std::cout << "Test Case 3: Minimum Active Players (2 Players)\n";
    Manager manager;

    // Set players 3, 4, 5 as inactive
    for (size_t i = 3; i < manager.players.size(); i++) {
      setPlayerActive(manager, i, false);
    }

    // Assign blinds based on current active players
    manager.arrangePlayersPosition();

    // Print blind values after blind assignment
    std::cout << "After assigning blinds with minimum active players:\n";
    printBlindValues(manager);

    // Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 1); // Should move to position 1
    assert(newPos->posSB == 2);          // Should move to position 0
    assert(newPos->posBB == 0); // Should move to position 1 (Dealer also as BB)

    std::cout << "Test Case 3 Passed!\n\n";
  }

  /* Test Case 4: Multiple Rotations */
  static void testMultipleRotations() {
    std::cout << "Test Case 4: Multiple Rotations\n";
    Manager manager;

    // Ensure all players are active initially
    for (size_t i = 0; i < manager.players.size(); i++) {
      manager.players[i]->setIsActive(true);
    }

    std::cout << "Initial setup:\n";
    printBlindValues(manager);

    // Perform multiple rotations and verify positions
    for (int rotation = 1; rotation <= 3; rotation++) {
      std::cout << "\nRotation " << rotation << ":\n";

      manager.arrangePlayersPosition();
      auto pos = manager.getSpecialPositions();

      printBlindValues(manager);

      // Verify blinds are assigned correctly
      assert(manager.players[pos->dealerPosition]->getBlind() == Blind::dealer);
      assert(manager.players[pos->posSB]->getBlind() == Blind::smallBlind);
      assert(manager.players[pos->posBB]->getBlind() == Blind::bigBlind);

      // Verify positions are different
      assert(pos->dealerPosition != pos->posSB);
      assert(pos->dealerPosition != pos->posBB);
      assert(pos->posSB != pos->posBB);

      // After second rotation, deactivate some players
      if (rotation == 2) {
        setPlayerActive(manager, 1, false);
        setPlayerActive(manager, 3, false);
        std::cout << "Deactivated players 1 and 3\n";
      }
    }

    std::cout << "Test Case 4 Passed!\n\n";
  }

  // Update runAllTests
  static void runAllTests() {
    testAllPlayersActive();
    testSomePlayersInactive();
    testMinimumActivePlayers();
    testMultipleRotations();
    std::cout << "All ManagerTest cases passed successfully!\n";
  }
};

// Update main function
int main() {
  ManagerTest::runAllTests();
  Manager mng;
  mng.startGame();
  return 0;
}
