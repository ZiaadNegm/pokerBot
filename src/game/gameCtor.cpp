#include "../../include/game.hpp"

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

void Game::simulateHand() {
  standardStartRoundOperations();
  checkHoleCards();
  std::shared_ptr<Player> winner = subRoundHandler();
  if (winner) {
    winner->addChips(pot);
  }
  resetHand();
}
