#include "../include/game.hpp"
// ------------------- Round/State Handlers -------------------
// GameFlow
void Game::handlePreFlop() {
  aBetHasBeenPlaced = true;
  letPlayerstakeAction();
  return;
}

// gameFlow
void Game::handleFlop() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

// gameFlow
void Game::handleTurn() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

// gameFlow
void Game::handleRiver() {
  aBetHasBeenPlaced = false;
  dealCommunityCards();
  letPlayerstakeAction();
  return;
}

// gameFlow
void Game::handleShowDown() {
  calculateBesthand();
  return;
}

// gameFlow
void Game::letPlayerstakeAction() {
  std::shared_ptr<Player> player;
  currentRound++;
  firstIterationOfRound = true;

  // Print current betting status:
  printGameState(gameState);

  while ((player = getNextPlayerInSequence()) != nullptr &&
         getNotFoldedPlayers() > 1) {
    int playerIndex = indexOfPlayer(players, player);
    currentPlays.LastTurnPlayer = playerIndex;

    // Show full table and current player's info:
    showTurnInfo(player);
    Action action = getActionPlayer(player);

    performAction(player, action);
    if (firstIterationOfRound) {
      firstIterationOfRound = false;
    }
    decidePlayersGameCycle(players);
  }
}

std::shared_ptr<Player> Game::subRoundHandler() {
  while (getNotFoldedPlayers() > 1) {
    auto handler = stateToFunction.at(gameState);
    handler(this);

    currentPlays.LastTurnPlayer =
        indexOfPlayer(players, players[gamePositions.dealerPosition]);
    currentPlays.ActionTaker = -1;

    printGameState(gameState);

    if (gameState != gameStates::showDown) {
      gameState = static_cast<gameStates>(static_cast<int>(gameState) + 1);
    }
  }
  return decideWinner();
}
