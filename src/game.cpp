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
class Game {
private:
public:
  /* Does the basic one time operations needed in a round.
   * Make the Blinds pay.
   * Deal hole cards to players in the game.
   */
  void standardStartRoundOperations() {}

  /* Validates the given action for the player.
   * Possible input: action and player?
   */
  void allValidAction() {}

  /* Actually calls the action for the player which is validated through the
   * function validateAction.
   */
  void performAction() {}

  /* Gets, validates and performs action
   * Possible input player?
   * Calls getNextPlayerInSequence()
   * Calculate all valid actions for player.
   * Offer these options
   * Gets the action from getActionPlayer().
   * performs these actions.
   * Maybe return a pointer to the winner?
   *
   */
  void letPlayerstakeAction() {}

  /* Will walk through all gameStates and update these states.
   * The stop condition can be:
   *  either we reached the end of showDown and we calculate a winner
   * ||
   * Everyone except one person folded.
   *
   * Calls getNextPlayerInSequence()
   * Calculate all valid actions for player.
   * Offer these options
   * Gets the action from getActionPlayer().
   * performs these actions.
   * Maybe return a pointer to the winner?
   */
  void subRoundHandler() {}

  /* Will function as the entre point for a round.
   * Calls standardStartRoundOperations.
   * Calls subRoundHandler.
   * Give the pot to the winner.
   * Reset function? s.a cards cleaned up, folded status reset.
   */
  void simulateHand() {}

  /* --Possible useless--
   * Keeps track of the Gamestatus
   */
  void keepTrackGameStatus(){};

  /* Decides which player plays next.
   * This should decide who the actionTaker is and check if we are already at
   * that actiontaker.
   * Maybe return a pointer to the player who should play?
   */
  void getNextPlayerInSequence(){};

  /* Decides whether a player should be excluded from this round.
   * For example player is out of chips or player has folded.
   * Maybe return a bool or change the valid property of the player?
   */
  void decidePlayersLifeCycle() {}

  /* Deals 2 cards to every player who is active in this round
   */
  void dealHoleCards() {}

  /* Depending on the game state, adds cards to communityCards.
   */
  void dealCommunityCards() {}

  /* The players folded property is set to true.
   * The effects of this will be executed in other functions while the game is
   * running.
   */
  void fold() {}

  /* The player checks. We assume that the player can validly check, to ensure
   * this, this must be checked in different parts of game.
   */
  void check() {}

  /* The player matches his bet with the current highest bet.
   * We assume here that the player has enough to call. This is left to other
   * function that check this.
   */
  void call() {}

  /* The player bets a amount which is greater than the current highest bet.
   * We assume that this player can actually raise, we leave this to other
   * functions.
   */
  void raise() {}

  /* All remaining chips go to the pot
   */
  void allIn() {}

  /* This player bets x.
   * We leave it to other functions to ensure that a player can only bet when no
   * one has placed a bet in that round except the SB and BB. This is most
   * likely the player after the SB.
   */
  void bet() {}

  /* Interface to manage the pot.
   * Input amount?
   */
  void managePot() {}

  /* Uses the module bestHand to calculate who has the best hand.
   * Returns a pointer to the player?
   */
  void decideWinner() {}
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
  Manager()
      : names({"Phill", "Doyle", "Daniel", "Chris", "Johnny", "You"}),
        currentRound(0) {
    initalizePLayers();
    startGame();
  }

  void log(const string &message) const { cout << message << endl; }

  /* Initalizes all players with their names and starting chips.
   * These players get allocated on the heap with shared pointers and pushed
   * onto the playersPool
   */
  void initalizePLayers() {}

  /* Will simulate k amount of hands by letting the Game class run the logic
   * behind that. Maybe a for loop for k amounts of reps, call
   * game.simulateHand()? At the end set the gameActive variable to false?
   */
  void startGame() {}

  /* Will print out all players with their chips etc.
   * Indicates the end of the program.
   */
  void endGame() {}
  void keepTrackCurrentRound() {}

  /* For a hand, move the dealer button. This will indirectly move the SB and BB
   */
  void arrangePlayersPosition() {}

  /* If a player has no chips more, remove them from the game as they can't do
   * anyhting. This will ensure that no funny things happen when moving around
   * the dealersButton
   */
  void decidePlayersLifeCycle() {}
};
