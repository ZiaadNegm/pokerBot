#include <cstdint>
#include <sys/types.h>
#include <vector>
import cards;
import player;
#include <deque>
#include <string>
std::string playersName[] = {"Phill", "Doyle",  "Daniel",
                             "Chris", "Johnny", "You"};

using money = std::uint32_t;
using playersPool = std::deque<Player>;

enum class actions { fold, check, call, raise, allIn };
enum class gameState { preFlop, flop, turn, river, showDown };

class Game {
private:
  Deck deck;
  money pot;
  money highestBettedInRound;
  money currentBet;
  playersPool PlayersTurn;
  std::vector<Card> communityCards;
  int roundCounter;
};

using money = std::uint32_t;
int main() { return 0; }