#include "../../../include/IGameIO.hpp"

class consoleGame : public IGameIO {
  void log(textData data) {
    std::cout << data;
    return;
  }

  void checkHoleCards(textData prefix, textData data, textData postfix) {
    log(prefix);
    log(data);
    log(postfix);
  }

  void printGameState(textData prefix, textData data, textData postfix) {
    log(prefix);
    log(data);
    log(postfix);
  }
};