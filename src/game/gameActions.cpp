#include "../../include/game.hpp"
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