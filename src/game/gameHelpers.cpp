#include "../include/game.hpp"
// gameHelpers
int Game::getActivePlayers() {
  int activePlayers = 0;
  for (auto &player : players) {
    if (player->getIsActive()) {
      activePlayers++;
    }
  }
  return activePlayers;
}

// gameHelpers
int Game::getNotFoldedPlayers() {
  int amountFolded = 0;
  for (auto &player : players) {
    if (player->getIsActive() && player->hasPlayerFolded()) {
      amountFolded++;
    }
  }
  return getActivePlayers() - amountFolded;
}

// gameHelpers
std::shared_ptr<Player> Game::getNextActivePlayer() {
  size_t pos = (currentPlays.LastTurnPlayer + 1) % players.size();
  std::shared_ptr<Player> player = players[pos];
  while (!player->getIsActive() || player->hasPlayerFolded()) {
    pos = (pos + 1) % players.size();
    player = players[pos];
  }
  return players[pos];
}

// gameHelper
int Game::indexOfPlayer(const playersPool &pool,
                        const std::shared_ptr<Player> &p) {
  for (int i = 0; i < static_cast<int>(pool.size()); i++) {
    if (pool[i] == p) {
      return i;
    }
  }
  return -1;
}
// gameHelper
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

// gameHelpers
std::shared_ptr<Player> Game::getNextPlayerInSequence() {
  if (killSwitch) {
    killSwitch = false;
    return nullptr;
  }

  std::shared_ptr<Player> nextPlayer = getNextActivePlayer();

  // Recompute leftOfDealer
  std::shared_ptr<Player> leftOfDealerCandidate =
      getNextActiveAfter(players[gamePositions.dealerPosition]);
  leftPlayerToDealer = indexOfPlayer(players, leftOfDealerCandidate);

  // If nextPlayer == leftOfDealer in a non-first iteration, normally we stop
  // the betting round, *unless* freePassForLeftOfDealer is true
  if ((currentPlays.ActionTaker == -1) &&
      (nextPlayer == leftOfDealerCandidate) && !firstIterationOfRound) {
    if (freePassForLeftOfDealer) {
      freePassForLeftOfDealer = false;
    } else {
      return nullptr;
    }
  }

  if ((currentPlays.ActionTaker != -1) &&
      (nextPlayer == players[currentPlays.ActionTaker])) {
    if (currentPlays.ActionTaker == gamePositions.posBB && firstTime) {
      firstTime = false;
      killSwitch = true;
      return nextPlayer;
    }
    return nullptr;
  }
  return nextPlayer;
}

// gameHelpers
void Game::resetHand() {
  for (auto &player : players) {
    player->resetCards();
    player->setHasFolded(false);
  }
  return;
}

// gameHelpers
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

// gameHelpers
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

// gameHelpers
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

// gameHelpers
void Game::calculateBesthand() {
  std::vector<std::shared_ptr<Player>> playerVector(players.begin(),
                                                    players.end());
  determineBestHand bestHandCalculator(playerVector, communityCards);

  std::shared_ptr<Player> winner =
      bestHandCalculator.determineWinnerByHighestCard();

  // Everyone else folds
  for (auto &p : players) {
    if (p != winner && p->getIsActive() && !p->hasPlayerFolded()) {
      p->setHasFolded(true);
    }
  }
}

// gameHelper
std::shared_ptr<Player> &Game::decideWinner() {
  static std::shared_ptr<Player> nullPlayer = nullptr;
  for (auto &player : players) {
    if (player->getIsActive() && !player->hasPlayerFolded()) {
      return player;
    }
  }
  return nullPlayer;
}

// gameHelpers
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

// gameInterfaceIO