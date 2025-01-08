#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
import player;

using namespace std;
using money = uint32_t;
using playersPool = deque<shared_ptr<Player>>;
using position = uint32_t;
enum class actions { fold, check, call, raise, allIn, bet };

struct Action {
  actions action;
  money bet;
  size_t roundCounter;
};
using playersHistory = std::pmr::unordered_map<int, Action>;

class Game {
private:
public:
};

struct gameSettings {
  size_t minAmountPlayers = 2;
  size_t maxAmountPlayers = 6;
  money minBet = 10;
  money startingChips = 100;
  size_t maximumRounds = 3;
};

struct positions {
  position dealerPosition = 0;
  position posBB = 0;
  position posSB = 0;
};

class Manager {
private:
  Game game;              //  The game containing all the core logic.
  gameSettings settings;  // a struct containing all default settings
  vector<string> names;   // Default names for six players.
  playersPool players;    // a deque with shared pointers to each player.
  playersHistory History; // The history of each player.
  size_t currentRound;
  positions specialPositions; // struct containing position of BB SB and Dealer.
  bool gameActive;

public:
  void log(const string &message) const { cout << message << endl; }
};
