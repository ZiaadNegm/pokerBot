#ifndef IGAMEIO_HPP
#define IGAMEIO_HPP
#include "cards.hpp"
#include "game.hpp"
#include "player.hpp"
#include <memory>
#include <string>
#include <vector>

class IGameIO {
protected:
  using textData = const std::string &;

public:
  virtual ~IGameIO() = default;

  virtual void checkHoleCards(textData playerName,
                              const std::vector<Card> &playerCards) = 0;
  virtual void printGameState(textData gameState) = 0;
  virtual void showTurnInfo(const std::shared_ptr<Player> &currentPlayer,
                            const std::shared_ptr<Player> &nextPlayer,
                            gameStates state, money highestBet, money pot) = 0;
  virtual void printPlayersTable(const playersPool &players) = 0;
  virtual std::vector<actions> offerOptions(actionMap validMoves) = 0;
  virtual money promptForActionAmount(actions act, money minAmount) = 0;
  virtual Action getInputPlayer(std::vector<actions> offeredOptions,
                                actionMap validMoves) = 0;
  virtual void logActions(std::shared_ptr<Player> player, Action action) = 0;
};
#endif