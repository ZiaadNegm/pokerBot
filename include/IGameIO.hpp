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

  virtual void checkHoleCards(textData prefix, textData data,
                              textData postfix) = 0;
  virtual void printGameState(textData prefix, textData data, textData postfix) = 0;
  virtual void showTurnInfo(const std::shared_ptr<Player> &currentPlayer) = 0;
  virtual void printPlayersTable() = 0; // Added missing declaration
  virtual Action
  offerOptions(actionMap validMoves) = 0; // Added missing declaration
  virtual money promptForActionAmount(actions act,
                                      money minAmount) = 0; // Added
  virtual Action getInputPlayer(std::vector<actions> offeredOptions,
                                actionMap validMoves) = 0; // Added
  virtual void logActions(std::shared_ptr<Player> player,
                          Action action) = 0; // Added
};