module; // <--- global module fragment
#include <cstdint>
#include <string>
#include <vector>

export module player; // <--- now declare the actual module interface
import cards;

class Player {
private:
  std::string name;
  int id;
  std::vector<Card> hand;
  uint32_t chips;
  uint32_t currentBet;
  bool canCheck;
  bool Bblind, Sblind, Nblind;
  bool hasFolded;
};