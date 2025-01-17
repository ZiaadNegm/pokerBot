#include <algorithm>
#include <assert.h>
#include <climits>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
import player;
import cards;
#define INVALID_POS INT_MIN
#define AMOUNT_OF_CARDS 2

using money = std::uint32_t;
using playersPool = std::deque<std::shared_ptr<Player>>;
using notActivePlayers = std::vector<std::shared_ptr<Player>>;
using position = std::int32_t;
enum class actions { fold, check, call, raise, allIn, bet };

std::vector<std::string> names = {"Ziaad", "Daaiz"};

enum class gameStates { preFlop, flop, turn, river, showDown };

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
  position posBB = 1;
  position posSB = 2;
};
class Game {
private:
  money pot;
  playersPool players;
  Deck deck;
  std::vector<Card> communityCards;
  money highestBet;
  gameStates gameState;
  gameSettings settings;
  positions gamePositions;

  void checkHoleCards() {
    for (auto player : players) {
      std::cout << "Player " << player->getName() << " Has the following cards"
                << "\n";
      for (Card card : player->getHand()) {
        std::cout << " " << card.cardToString() << "\n";
      }
      std::cout << "\n";
    }
  }

public:
  Game()
      : pot(0), players(), deck(), communityCards(), highestBet(0),
        gameState(gameStates::preFlop), settings(), gamePositions() {}

  Game(playersPool &players, gameSettings &settings, const positions &pos)
      : players(players), settings(settings), gamePositions(pos), pot(0),
        highestBet(0), deck(), communityCards({}),
        gameState(gameStates::preFlop) {}

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
  void allIn(std::shared_ptr<Player> &player) {}

  /* This player bets x.
   * We leave it to other functions to ensure that a player can only bet when no
   * one has placed a bet in that round except the SB and BB. This is most
   * likely the player after the SB.
   */
  void bet() {}

  /* Validates the given action for the player.
   * Possible input: action and player?
   */
  void allValidAction() {}

  /* Actually calls the action for the player which is validated through the
   * function validateAction.
   */
  void performAction() {}

  /* offers certain options to the player. The player gives his option as input.
   * We return this input as a action type.
   */
  void getActionPlayer() {}

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

  /* Calls the players method resetHand to give a clean sheet for the next
   * round.
   */
  void resetHand() {
    for (auto &player : players) {
      player->resetCards();
    }
    return;
  }

  /* Will function as the entre point for a round.
   * Calls standardStartRoundOperations.
   * Calls subRoundHandler.
   * Give the pot to the winner.
   * Reset function? s.a cards cleaned up, folded status reset.
   */
  void simulateHand() {
    standardStartRoundOperations();
    checkHoleCards();
    resetHand();
  }

  /* --Possible useless--
   * Keeps track of the Gamestatus
   */
  void keepTrackGameStatus() {}

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
  void dealHoleCards() {
    for (int i = 0; i < AMOUNT_OF_CARDS; i++) {
      for (auto &player : players) {
        if (player->getIsActive()) {
          Card toBeDealt = deck.dealCard();
          player->receiveCards(toBeDealt);
        }
      }
    }
    return;
  }

  /* Depending on the game state, adds cards to communityCards.
   */
  void dealCommunityCards() {}

  /* Interface to manage the pot.
   * Input amount?
   */
  void managePot() {}

  /* Does the basic one time operations needed in a round.
   * Make the Blinds pay.
   * Deal hole cards to players in the game.
   */
  void standardStartRoundOperations() {
    auto &smallBlindPlayer = players[gamePositions.posSB];
    auto &bigBlindPlayer = players[gamePositions.posBB];

    if (smallBlindPlayer->getChips() < 0.5 * settings.minBet) {
      allIn(smallBlindPlayer);
    }
    if (smallBlindPlayer->getChips() < settings.minBet) {
      allIn(bigBlindPlayer);
    }

    dealHoleCards();

    return;
  }

  /* Uses the module bestHand to calculate who has the best hand.
   * Returns a pointer to the player?
   */
  void decideWinner(){};
};

class ManagerTest;

class Manager {
private:
  Game game;                        //  The game containing all the core logic.
  gameSettings settings;            // a struct containing all default settings
  std::vector<std::string> names;   // Default names for six players.
  notActivePlayers inActivePlayers; // All players who have 0 chips and cannot
                                    // play anymore
  playersHistory History;           // The history of each player.
  size_t currentRound;
  positions specialPositions; // struct containing position of BB SB and Dealer.
  bool gameActive;

  friend class ManagerTest;

  int activePlayers() {
    int activeCount = 0;
    for (auto &player : players) {
      if (player->getIsActive()) {
        activeCount++;
      }
    }
    return activeCount;
  }

public:
  playersPool players; // a deque with shared pointers to each player.
  Manager()
      : game(), names({"Phill", "Doyle", "Daniel", "Chris", "Johnny", "You"}),
        currentRound(0) {
    initalizePLayers();
  }

  void log(const std::string &message) const {
    std::cout << message << std::endl;
  }

  /* Error catching function. (Later. For now names is a static defined
   * array.) When too few names provided, ask input to fill in missingAmount
   * of names.
   */
  void handleTooFewNamesProvided(int missingAmount) {}

  /* Initalizes the SB BB and D positions. This can't be reduced to a default
   * value init as in the begin of the game we might already have inactive
   * players.
   */
  void initalizeSpecialPositions() {
    position newDealerIndex = findNextValidPos(0);
    if (newDealerIndex == INVALID_POS) {
      handleTooFewPlayers();
      return;
    }

    players[newDealerIndex]->setBlind(Blind::dealer);
    specialPositions.dealerPosition = newDealerIndex;

    position initialSBIndex = findNextValidPos(newDealerIndex + 1);
    if (initialSBIndex == INVALID_POS) {
      handleTooFewPlayers();
      return;
    }

    players[initialSBIndex]->setBlind(Blind::smallBlind);
    specialPositions.posSB = initialSBIndex;

    position initialBBIndex = findNextValidPos(initialSBIndex + 1);
    if (initialBBIndex == INVALID_POS) {
      handleTooFewPlayers();
      return;
    }

    players[initialBBIndex]->setBlind(Blind::bigBlind);
    specialPositions.posBB = initialBBIndex;
  }

  /* Initalizes all players with their names and starting chips.
   * These players get allocated on the heap with shared pointers and pushed
   * onto the playersPool
   */
  void initalizePLayers() {

    if (names.size() < settings.minAmountPlayers) {
      handleTooFewNamesProvided(names.size() - settings.minAmountPlayers);
    }

    for (int i = 0; i < names.size() && i < settings.maxAmountPlayers; i++) {
      std::shared_ptr<Player> p =
          make_shared<Player>(names[i], settings.startingChips);
      players.emplace_back(p);
    }
    initalizeSpecialPositions();
  }

  /* Return the special position by pointer.
   */
  std::shared_ptr<const positions> getSpecialPositions() {
    return std::make_shared<positions>(specialPositions);
  }

  size_t getCurrentRound() { return currentRound; }

  /* Will print out the current round, name, chips, and activity status in a
   * nice format.
   */
  void gameStatistics() {
    std::cout << "==========================================" << std::endl;
    std::cout << "           Poker Game Statistics          " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Current Round: " << currentRound << std::endl << std::endl;

    // Print the header with fixed column widths
    std::cout << std::left << std::setw(4) << "#" << std::setw(12) << "Name"
              << std::setw(10) << "Chips" << std::setw(12) << "Blind"
              << std::setw(10) << "Status" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    int playerNumber = 1;
    for (const auto &player : players) {
      std::string blindStatus = "None";
      switch (player->getBlind()) {
      case Blind::dealer:
        blindStatus = "Dealer";
        break;
      case Blind::smallBlind:
        blindStatus = "SmallBlind";
        break;
      case Blind::bigBlind:
        blindStatus = "BigBlind";
        break;
      default:
        break;
      }

      std::string status =
          player->getIsActive()
              ? "Active"
              : (player->getChips() == 0 ? "Out of Chips" : "Inactive");

      // Print player information with proper alignment
      std::cout << std::left << std::setw(4) << playerNumber << std::setw(12)
                << player->getName() << std::setw(10) << player->getChips()
                << std::setw(12) << blindStatus << std::setw(10) << status
                << std::endl;
      playerNumber++;
    }

    std::cout << "------------------------------------------" << std::endl;
  }

  /* Will simulate k amount of hands by letting the Game class run the logic
   * behind that. Maybe a for loop for k amounts of reps, call
   * game.simulateHand()? At the end set the gameActive variable to false?
   */
  void startGame() {
    for (int i = 0; i < settings.maximumRounds; i++) {
      Game game(players, settings, specialPositions);
      game.simulateHand();
      decidePlayersLifeCycle();
      arrangePlayersPosition();
      gameStatistics();
    }

    endGame();
  }

  /* Will print out all players with their chips etc.
   * Indicates the end of the program.
   */
  void endGame() {
    std::cout << "==========================================" << std::endl;
    std::cout << "                Game Over                  " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Final Standings:" << std::endl;

    std::cout << std::left << std::setw(4) << "#" << std::setw(15) << "Name"
              << std::setw(10) << "Chips" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Convert deque to vector using vector's range constructor
    std::vector<std::shared_ptr<Player>> sortedPlayers(players.begin(),
                                                       players.end());

    std::sort(sortedPlayers.begin(), sortedPlayers.end(),
              [](const std::shared_ptr<Player> &a,
                 const std::shared_ptr<Player> &b) -> bool {
                return a->getChips() > b->getChips();
              });

    // Display each player's final chips
    int rank = 1;
    for (const auto &player : sortedPlayers) {
      std::cout << std::left << std::setw(4) << rank << std::setw(15)
                << player->getName() << std::setw(10) << player->getChips()
                << std::endl;
      rank++;
    }

    std::cout << "------------------------------------------" << std::endl;

    // Identify the winner(s)
    if (!sortedPlayers.empty()) {
      money topChips = sortedPlayers.front()->getChips();
      std::vector<std::string> winners;

      for (const auto &player : sortedPlayers) {
        if (player->getChips() == topChips && topChips > 0) {
          winners.push_back(player->getName());
        } else {
          break;
        }
      }

      // Announce the winner(s)
      if (winners.size() == 1) {
        std::cout << "Winner: " << winners.front() << " with " << topChips
                  << " chips!" << std::endl;
      } else if (winners.size() > 1) {
        std::cout << "Winners: ";
        for (size_t i = 0; i < winners.size(); ++i) {
          std::cout << winners[i];
          if (i < winners.size() - 1) {
            std::cout << ", ";
          }
        }
        std::cout << " each with " << topChips << " chips!" << std::endl;
      } else {
        std::cout << "No winners. All players are out of chips." << std::endl;
      }
    } else {
      std::cout << "No players participated in the game." << std::endl;
    }

    std::cout << "==========================================" << std::endl;
  }

  /* Error catching function. We can't assign the same player to be a SB and a
   * BB. The game has to terminate as something clearly went wrong.
   */
  void handleTooFewPlayers() {
    throw std::runtime_error("Too few players to continue the game.");
  }

  position findNextValidPos(position start) {
    // Count active players
    if (activePlayers() < 3) {
      return INVALID_POS;
    }

    start = start % players.size();
    position initial = start;

    do {
      if (players[start] && players[start]->getIsActive()) {
        return start;
      }
      start = (start + 1) % players.size();
    } while (start != initial);

    return INVALID_POS;
  }

  /* For a betRound, move the dealer button. This will indirectly move the SB
   * and BB
   */
  void arrangePlayersPosition() {
    if (activePlayers() < 3) {
      return handleTooFewPlayers();
    }

    // Clear all blinds first
    for (auto &player : players) {
      player->setBlind(Blind::notBlind);
    }

    // Set new positions with validation
    position newDealerIndex = findNextValidPos(
        (specialPositions.dealerPosition + 1) % players.size());
    if (newDealerIndex == INVALID_POS) {
      handleTooFewPlayers();
      return;
    }

    position newSBIndex =
        findNextValidPos((newDealerIndex + 1) % players.size());
    if (newSBIndex == INVALID_POS || newSBIndex == newDealerIndex) {
      handleTooFewPlayers();
      return;
    }

    position newBBIndex = findNextValidPos((newSBIndex + 1) % players.size());
    if (newBBIndex == INVALID_POS || newBBIndex == newDealerIndex ||
        newBBIndex == newSBIndex) {
      handleTooFewPlayers();
      return;
    }

    // Set new positions and blinds
    specialPositions.dealerPosition = newDealerIndex;
    specialPositions.posSB = newSBIndex;
    specialPositions.posBB = newBBIndex;

    players[newDealerIndex]->setBlind(Blind::dealer);
    players[newSBIndex]->setBlind(Blind::smallBlind);
    players[newBBIndex]->setBlind(Blind::bigBlind);
  }

  /* If a player has no chips more, remove them from the game as they can't do
   * anyhting. This will ensure that no funny things happen when moving around
   * the dealersButton.
   * We set the player to inActive.
   */
  void decidePlayersLifeCycle() {
    for (auto &player : players) {
      if (player->getChips() == 0) {
        player->setIsActive(false);
        log(player->getName() + "is out of chips and inactive. ");
      }
    }
  }
};

struct ManagerTest {
  static void printBlindValues(const Manager &manager) {
    std::cout << "Current blind values:\n";
    for (size_t i = 0; i < manager.players.size(); i++) {
      std::cout << "Player " << i << " (" << manager.names[i] << ") blind: "
                << static_cast<int>(manager.players[i]->getBlind())
                << " Active status " << manager.players[i]->getIsActive()
                << "\n";
    }
    std::cout << std::endl;
  }

  /* Helper function to set player activity */
  static void setPlayerActive(Manager &manager, size_t playerIndex,
                              bool isActive) {
    if (playerIndex < manager.players.size()) {
      manager.players[playerIndex]->setIsActive(isActive);
    }
  }

  /* Test Case 1: All Players Active */
  static void testAllPlayersActive() {
    std::cout << "Test Case 1: All Players Active\n";
    Manager manager;

    // Ensure all players are active
    for (size_t i = 0; i < manager.players.size(); i++) {
      manager.players[i]->setIsActive(true);
    }

    // Print blind values
    std::cout << "After initialization:\n";
    printBlindValues(manager);

    // Verify initial positions
    auto initialPos = manager.getSpecialPositions();
    assert(manager.players[initialPos->dealerPosition]->getBlind() ==
           Blind::dealer);
    assert(manager.players[initialPos->posSB]->getBlind() == Blind::smallBlind);
    assert(manager.players[initialPos->posBB]->getBlind() == Blind::bigBlind);

    // Test rotation
    manager.arrangePlayersPosition();

    // Print blind values after rotation
    std::cout << "After rotation:\n";
    printBlindValues(manager);

    // Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 1); // Should move to position 1
    assert(newPos->posSB == 2);          // Should move to position 2
    assert(newPos->posBB == 3);          // Should move to position 3

    std::cout << "Test Case 1 Passed!\n\n";
  }

  /* Test Case 2: Some Players Inactive */
  static void testSomePlayersInactive() {
    std::cout << "Test Case 2: Some Players Inactive\n";
    Manager manager;

    // Initial blind assignment is already done in the Manager constructor via
    // initalizeSpecialPositions()

    // Print initial blind values
    std::cout << "Initial blind assignment:\n";
    printBlindValues(manager);

    // 1. First Rotation: Rotate blinds once
    manager.arrangePlayersPosition();
    std::cout << "After first rotation:\n";
    printBlindValues(manager);

    // 2. Set players 1 and 3 as inactive
    setPlayerActive(manager, 1, false); // Assuming player index 1 is "Doyle"
    setPlayerActive(manager, 3, false); // Assuming player index 3 is "Chris"
    std::cout << "After setting players 1 and 3 as inactive:\n";
    printBlindValues(manager);

    // 3. Second Rotation: Rotate blinds again
    manager.arrangePlayersPosition();
    std::cout << "After second rotation with some players inactive:\n";
    printBlindValues(manager);

    // 4. Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 2); // Should move to position 2 ("Daniel")
    assert(newPos->posSB == 4);          // Should move to position 4 ("Johnny")
    assert(newPos->posBB == 5);          // Should move to position 5 ("You")

    std::cout << "Test Case 2 Passed!\n\n";
  }

  /* Test Case 3: Minimum Active Players (3 Players) */
  static void testMinimumActivePlayers() {
    std::cout << "Test Case 3: Minimum Active Players (2 Players)\n";
    Manager manager;

    // Set players 3, 4, 5 as inactive
    for (size_t i = 3; i < manager.players.size(); i++) {
      setPlayerActive(manager, i, false);
    }

    // Assign blinds based on current active players
    manager.arrangePlayersPosition();

    // Print blind values after blind assignment
    std::cout << "After assigning blinds with minimum active players:\n";
    printBlindValues(manager);

    // Verify rotated positions
    auto newPos = manager.getSpecialPositions();
    assert(newPos->dealerPosition == 1); // Should move to position 1
    assert(newPos->posSB == 2);          // Should move to position 0
    assert(newPos->posBB == 0); // Should move to position 1 (Dealer also as BB)

    std::cout << "Test Case 3 Passed!\n\n";
  }

  /* Test Case 4: Multiple Rotations */
  static void testMultipleRotations() {
    std::cout << "Test Case 4: Multiple Rotations\n";
    Manager manager;

    // Ensure all players are active initially
    for (size_t i = 0; i < manager.players.size(); i++) {
      manager.players[i]->setIsActive(true);
    }

    std::cout << "Initial setup:\n";
    printBlindValues(manager);

    // Perform multiple rotations and verify positions
    for (int rotation = 1; rotation <= 3; rotation++) {
      std::cout << "\nRotation " << rotation << ":\n";

      manager.arrangePlayersPosition();
      auto pos = manager.getSpecialPositions();

      printBlindValues(manager);

      // Verify blinds are assigned correctly
      assert(manager.players[pos->dealerPosition]->getBlind() == Blind::dealer);
      assert(manager.players[pos->posSB]->getBlind() == Blind::smallBlind);
      assert(manager.players[pos->posBB]->getBlind() == Blind::bigBlind);

      // Verify positions are different
      assert(pos->dealerPosition != pos->posSB);
      assert(pos->dealerPosition != pos->posBB);
      assert(pos->posSB != pos->posBB);

      // After second rotation, deactivate some players
      if (rotation == 2) {
        setPlayerActive(manager, 1, false);
        setPlayerActive(manager, 3, false);
        std::cout << "Deactivated players 1 and 3\n";
      }
    }

    std::cout << "Test Case 4 Passed!\n\n";
  }

  // Update runAllTests
  static void runAllTests() {
    testAllPlayersActive();
    testSomePlayersInactive();
    testMinimumActivePlayers();
    testMultipleRotations();
    std::cout << "All ManagerTest cases passed successfully!\n";
  }
};

struct gameTest {};

// Update main function
int main() {
  ManagerTest::runAllTests();
  return 0;
}
