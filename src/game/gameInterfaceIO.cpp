#include "../include/IGameIO.hpp"
#include "../include/game.hpp"
void Game::checkHoleCards() {
  for (auto player : players) {
    IO->checkHoleCards(player->getName(), player->getHand());
  }
  return;
}
void Game::printGameState(gameStates state) {
  switch (state) {
  case gameStates::preFlop:
    IO->printGameState("preFlop");
    break;
  case gameStates::flop:
    IO->printGameState("flop");
    break;
  case gameStates::turn:
    IO->printGameState("turn");
    break;
  case gameStates::river:
    IO->printGameState("river");
    break;
  case gameStates::showDown:
    IO->printGameState("showDown");
    break;
  default:
    IO->printGameState("unknown State");
    break;
  }
}

void Game::showTurnInfo(const std::shared_ptr<Player> &currentPlayer) {
  auto nextPlayer = getNextActiveAfter(currentPlayer);

  IO->printPlayersTable(players);

  IO->showTurnInfo(currentPlayer, nextPlayer, gameState, highestBet, pot);
}

Action Game::offerOptions(actionMap validMoves) {

  std::vector<actions> offeredOptions = IO->offerOptions(validMoves);

  Action action = getInputPlayer(offeredOptions, validMoves);

  action.roundCounter = currentRound;

  return action;
}

money Game::promptForActionAmount(actions act, money minAmount) {
  return IO->promptForActionAmount(act, minAmount);
}

Action Game::getInputPlayer(std::vector<actions> offeredOptions,
                            actionMap validMoves) {
  Action action = IO->getInputPlayer(offeredOptions, validMoves);

  action.roundCounter = currentRound;

  return action;
}

// gameInterfaceIO
void Game::logActions(std::shared_ptr<Player> player, Action action) {
  IO->logActions(player, action);
}