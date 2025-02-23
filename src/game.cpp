#include "../include/game.hpp"

const std::unordered_map<actions, Game::actionHandler> Game::actionToFunction =
    {{actions::fold,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->fold(p, a); }},
     {actions::check,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->check(p, a); }},
     {actions::call,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->call(p, a); }},
     {actions::raise,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->raise(p, a); }},
     {actions::allIn,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->allIn(p, a); }},
     {actions::bet,
      [](Game *g, std::shared_ptr<Player> &p, Action a) { g->bet(p, a); }}};

const std::unordered_map<gameStates, Game::stateHandler> Game::stateToFunction =
    {{gameStates::preFlop, [](Game *g) { g->handlePreFlop(); }},
     {gameStates::flop, [](Game *g) { g->handleFlop(); }},
     {gameStates::turn, [](Game *g) { g->handleTurn(); }},
     {gameStates::river, [](Game *g) { g->handleRiver(); }},
     {gameStates::showDown, [](Game *g) { g->handleShowDown(); }}};

Game::Game()
    : pot(0), players(), deck(), communityCards(), highestBet(0),
      gameState(gameStates::preFlop), settings(), raiseAmount(settings.minBet),
      gamePositions(), aBetHasBeenPlaced(false), currentRound(0),
      firstIterationOfRound(true), firstTime(true), killSwitch(false),
      freePassForLeftOfDealer(false), leftPlayerToDealer(0) {}

Game::Game(playersPool &players, gameSettings &settings, const positions &pos)
    : pot(0), players(players), deck(), communityCards({}), highestBet(0),
      gameState(gameStates::preFlop), settings(settings),
      raiseAmount(settings.minBet), gamePositions(pos),
      aBetHasBeenPlaced(false), currentRound(0), firstIterationOfRound(true),
      firstTime(true), killSwitch(false), freePassForLeftOfDealer(false),
      leftPlayerToDealer(0) {
  currentPlays.ActionTaker = gamePositions.posBB + 1;
  currentPlays.LastTurnPlayer = gamePositions.posSB + 1;
}

// ------------------- Private Helpers -------------------
void Game::checkHoleCards() {
  for (auto player : players) {
    std::cout << "Player " << player->getName() << " Has the following cards\n";
    for (Card card : player->getHand()) {
      std::cout << " " << card.cardToString() << "\n";
    }
    std::cout << "\n";
  }
}

int Game::getActivePlayers() {
  int activePlayers = 0;
  for (auto &player : players) {
    if (player->getIsActive()) {
      activePlayers++;
    }
  }
  return activePlayers;
}

int Game::getNotFoldedPlayers() {
  int amountFolded = 0;
  for (auto &player : players) {
    if (player->getIsActive() && player->hasPlayerFolded()) {
      amountFolded++;
    }
  }
  return getActivePlayers() - amountFolded;
}

std::shared_ptr<Player> Game::getNextActivePlayer() {
  size_t pos = (currentPlays.LastTurnPlayer + 1) % players.size();
  std::shared_ptr<Player> player = players[pos];
  while (!player->getIsActive() || player->hasPlayerFolded()) {
    pos = (pos + 1) % players.size();
    player = players[pos];
  }
  return players[pos];
}

int Game::indexOfPlayer(const playersPool &pool,
                        const std::shared_ptr<Player> &p) {
  for (int i = 0; i < static_cast<int>(pool.size()); i++) {
    if (pool[i] == p) {
      return i;
    }
  }
  return -1;
}

actionMap Game::initializeValidActionMap(std::shared_ptr<Player> &player) {
  actionMap validActionMap;
  for (int i = 0; i <= static_cast<int>(actions::bet); i++) {
    actions currentAction = static_cast<actions>(i);
    validActionMap[currentAction] = std::make_pair(false, 0);
  }
  // Fold is always valid
  validActionMap[actions::fold] = {true, 0};
  // All-in is always valid with player's full stack
  validActionMap[actions::allIn] = {true, player->getChips()};
  return validActionMap;
}

std::shared_ptr<Player>
Game::getNextActiveAfter(const std::shared_ptr<Player> &current) {
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

std::shared_ptr<Player> Game::getNextPlayerInSequence() {
  if (killSwitch) {
    std::cout << "\nKILLSWITCH ACTIVATED\n" << std::endl;
    killSwitch = false;
    return nullptr;
  }

  std::shared_ptr<Player> nextPlayer = getNextActivePlayer();

  // Recompute leftOfDealer
  std::shared_ptr<Player> leftOfDealerCandidate =
      getNextActiveAfter(players[gamePositions.dealerPosition]);
  leftPlayerToDealer = indexOfPlayer(players, leftOfDealerCandidate);

  std::cout << "[DEBUG] Left Of Dealer: " << leftPlayerToDealer << std::endl;
  std::cout << "[DEBUG] Next player: " << indexOfPlayer(players, nextPlayer)
            << std::endl;

  // If nextPlayer == leftOfDealer in a non-first iteration, normally we stop
  // the betting round, *unless* freePassForLeftOfDealer is true
  if ((currentPlays.ActionTaker == -1) &&
      (nextPlayer == leftOfDealerCandidate) && !firstIterationOfRound) {
    if (freePassForLeftOfDealer) {
      std::cout
          << "[DEBUG] Free pass active for LeftOfDealer. Allowing action.\n";
      freePassForLeftOfDealer = false;
    } else {
      std::cout << "[DEBUG] ActionTaker -1 - Next = left - Not first round\n";
      return nullptr;
    }
  }

  if ((currentPlays.ActionTaker != -1) &&
      (nextPlayer == players[currentPlays.ActionTaker])) {
    std::cout << "[DEBUG] ActionTaker NOT -1, next = actiontaker\n";
    if (currentPlays.ActionTaker == gamePositions.posBB && firstTime) {
      firstTime = false;
      killSwitch = true;
      return nextPlayer;
    }
    return nullptr;
  }
  return nextPlayer;
}

// ------------------- Betting Flow Logic -------------------
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

// ------------------- Round/State Handlers -------------------
void Game::handlePreFlop() {
  aBetHasBeenPlaced = true;
  letPlayerstakeAction();
  return;
}

void Game::handleFlop() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

void Game::handleTurn() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

void Game::handleRiver() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

void Game::handleShowDown() {
  calculateBesthand();
  return;
}

// ------------------- Action Validations & Execution -------------------
actionMap Game::allValidAction(std::shared_ptr<Player> &player) {
  auto validActionMap = initializeValidActionMap(player);

  // If no bet has been placed, check is valid
  if (!aBetHasBeenPlaced) {
    validActionMap[actions::check] = {true, 0};
  }
  // If there is a highestBet, the player can call if they have enough chips
  if (aBetHasBeenPlaced && (player->getChips() >= highestBet)) {
    validActionMap[actions::call] = {true, highestBet};
  }
  // If there is a bet, the player can raise if they have enough chips
  if (aBetHasBeenPlaced && (player->getChips() >= (raiseAmount + highestBet))) {
    validActionMap[actions::raise] = {true, raiseAmount + highestBet};
  }
  // If no one has bet yet, the player can bet
  if (!aBetHasBeenPlaced && (player->getChips() >= settings.minBet)) {
    validActionMap[actions::bet] = {true, settings.minBet};
  }
  return validActionMap;
}

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

void Game::fold(std::shared_ptr<Player> &player, Action folded) {
  int foldedIndex = indexOfPlayer(players, player);
  player->setHasFolded(true);

  // If folding player is the leftPlayerToDealer, update
  if (foldedIndex == leftPlayerToDealer) {
    std::shared_ptr<Player> newLeft =
        getNextActiveAfter(players[gamePositions.dealerPosition]);
    while (newLeft && (newLeft->hasPlayerFolded() || !newLeft->getIsActive())) {
      newLeft = getNextActiveAfter(newLeft);
      if (!newLeft)
        break;
    }
    leftPlayerToDealer =
        (newLeft ? indexOfPlayer(players, newLeft) : leftPlayerToDealer);
    freePassForLeftOfDealer = true;
  }
  logActions(player, folded);
  return;
}

void Game::check(std::shared_ptr<Player> &player, Action checked) {
  player->check();
  logActions(player, checked);
  return;
}

void Game::call(std::shared_ptr<Player> &player, Action called) {
  player->call(called.bet);
  pot += called.bet;
  logActions(player, called);
  return;
}

void Game::raise(std::shared_ptr<Player> &player, Action raised) {
  player->raise(raised.bet);
  highestBet = raised.bet;
  pot += raised.bet;
  currentPlays.ActionTaker = indexOfPlayer(players, player);
  logActions(player, raised);
  return;
}

void Game::allIn(std::shared_ptr<Player> &player, Action allIn) {
  player->bet(allIn.bet);
  pot += allIn.bet;

  if (allIn.bet > highestBet) {
    highestBet = allIn.bet;
    currentPlays.ActionTaker = indexOfPlayer(players, player);
  }
  logActions(player, allIn);
  return;
}

void Game::bet(std::shared_ptr<Player> &player, Action action) {
  player->bet(action.bet);
  pot += action.bet;
  aBetHasBeenPlaced = true;
  currentPlays.ActionTaker = indexOfPlayer(players, player);
  logActions(player, action);
}

void Game::performAction(std::shared_ptr<Player> player,
                         Action actionToExecute) {
  auto handler = actionToFunction.at(actionToExecute.action);
  handler(this, player, actionToExecute);
}

Action Game::getActionPlayer(std::shared_ptr<Player> player) {
  actionMap validMoves = allValidAction(player);
  return offerOptions(validMoves);
}

// ------------------- Main Round Flow -------------------
void Game::letPlayerstakeAction() {
  std::shared_ptr<Player> player;
  currentRound++;
  firstIterationOfRound = true;

  std::cout << "[DEBUG] Starting letPlayerstakeAction, currentRound: "
            << currentRound << std::endl;
  // Print current betting status:
  std::cout << "[DEBUG] currentPlays.LastTurnPlayer: "
            << currentPlays.LastTurnPlayer
            << ", currentPlays.ActionTaker: " << currentPlays.ActionTaker
            << std::endl;

  std::cout << "[DEBUG] Game State: ";
  printGameState(gameState);

  while ((player = getNextPlayerInSequence()) != nullptr &&
         getNotFoldedPlayers() > 1) {
    int playerIndex = indexOfPlayer(players, player);
    std::cout << "[DEBUG] Next player in sequence is seat " << playerIndex
              << " (" << player->getName() << ")" << std::endl;
    currentPlays.LastTurnPlayer = playerIndex;

    // Show full table and current player's info:
    showTurnInfo(player);
    Action action = getActionPlayer(player);
    std::cout << "[DEBUG] Player " << player->getName()
              << " chose action: " << actionmessages.at(action.action)
              << " with bet: " << action.bet << std::endl;

    performAction(player, action);
    if (firstIterationOfRound) {
      firstIterationOfRound = false;
    }
    // Print updated pot/highestBet:
    std::cout << "[DEBUG] Updated pot: " << pot
              << ", highestBet: " << highestBet << std::endl;
  }
  std::cout << "[DEBUG] Exiting letPlayerstakeAction loop." << std::endl;
}

std::shared_ptr<Player> Game::subRoundHandler() {
  while (getNotFoldedPlayers() > 1) {
    std::cout << "\n\n Not-Folded-Players: " << getNotFoldedPlayers()
              << std::endl;
    auto handler = stateToFunction.at(gameState);
    handler(this);

    currentPlays.LastTurnPlayer =
        indexOfPlayer(players, players[gamePositions.dealerPosition]);
    currentPlays.ActionTaker = -1;

    std::cout << "Printing gamestate here.!!!!" << std::endl;
    printGameState(gameState);

    if (gameState != gameStates::showDown) {
      gameState = static_cast<gameStates>(static_cast<int>(gameState) + 1);
    }
  }
  return decideWinner();
}

// ------------------- Helper Methods -------------------
void Game::resetHand() {
  for (auto &player : players) {
    player->resetCards();
    player->setHasFolded(false);
  }
  return;
}

void Game::dealHoleCards() {
  for (int i = 0; i < AMOUNT_OF_CARDS; i++) {
    for (auto &player : players) {
      if (player->getIsActive()) {
        Card toBeDealt = deck.dealCard();
        player->receiveCards(toBeDealt);
      }
    }
  }
}

void Game::dealCommunityCards() {
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
}

void Game::standardStartRoundOperations() {
  auto &smallBlindPlayer = players[gamePositions.posSB];
  auto &bigBlindPlayer = players[gamePositions.posBB];

  // Small blind
  if (smallBlindPlayer->getChips() < (money)(0.5 * settings.minBet)) {
    Action allInAction = {actions::allIn, (money)smallBlindPlayer->getChips(),
                          0};
    allIn(smallBlindPlayer, allInAction);
  } else {
    Action betAction = {actions::bet, (money)(settings.minBet * 0.5), 0};
    bet(smallBlindPlayer, betAction);
  }

  // Big blind
  if (bigBlindPlayer->getChips() < settings.minBet) {
    Action allInAction = {actions::allIn, bigBlindPlayer->getChips(), 0};
    highestBet = bigBlindPlayer->getChips();
    allIn(bigBlindPlayer, allInAction);
  } else {
    Action betAction = {actions::bet, settings.minBet, 0};
    highestBet = settings.minBet;
    bet(bigBlindPlayer, betAction);
  }

  dealHoleCards();
}

void Game::calculateBesthand() {
  std::vector<std::shared_ptr<Player>> playerVector(players.begin(),
                                                    players.end());
  determineBestHand bestHandCalculator(playerVector, communityCards);

  std::shared_ptr<Player> winner =
      bestHandCalculator.determineWinnerByHighestCard();
  std::cout << "The best hand winner is: " << winner->getName() << "\n";

  // Everyone else folds
  for (auto &p : players) {
    if (p != winner && p->getIsActive() && !p->hasPlayerFolded()) {
      p->setHasFolded(true);
    }
  }
}

std::shared_ptr<Player> &Game::decideWinner() {
  static std::shared_ptr<Player> nullPlayer = nullptr;
  for (auto &player : players) {
    if (player->getIsActive() && !player->hasPlayerFolded()) {
      return player;
    }
  }
  return nullPlayer;
}

// ------------------- Public Interface -------------------
void Game::simulateHand() {
  standardStartRoundOperations();
  checkHoleCards();
  std::cout << "Finished checking HoleCards\n";
  std::shared_ptr<Player> winner = subRoundHandler();
  if (winner) {
    winner->addChips(pot);
  }
  resetHand();
}
