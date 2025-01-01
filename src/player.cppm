/* This file implements the Player class. It does so by providing a class
 * Player: Simulates a player in a poker game.
 *    -getName returns the name of the player.
 *    -getId returns the id of the player.
 *    -getHand returns the hand of the player.
 *    -getChips returns the amount of chips the player has.
 *    -getCurrentBet returns the amount of chips the player has bet in the
 * current round.
 *    -canPlayerCheck returns if the player can check.
 *    -getBlind returns the blind of the player.
 *    -hasPlayerFolded returns if the player has folded.
 *    -setChips sets the amount of chips the player has.
 *    -setCurrentBet sets the amount of chips the player has bet in the current
 * round.
 *    -setCanCheck sets if the player can check.
 *    -setBlind sets the blind of the player.
 *    -setHasFolded sets if the player has folded.
 *    -receiveCards sets the hand of the player.
 *    -resetCards resets the hand of the player.
 *    -addChips adds amount to the player's chips.
 *    -deductChips deducts amount from the player's chips.
 *    -bet subtracts the amount the player wants to bet from the player's chips
 * and returns the betted amount.
 *    -raise raises the player's current bet with raiseAmount.
 *    -check does nothing.
 *    -call calls the current bet.
 *    -fold folds the player's hand.
 *    -resetCurrentBet resets the player's current bet.
 */
module; // <--- global module fragment
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

export module player; // <--- now declare the actual module interface
import cards;

export enum class Blind { bigBlind, smallBlind, notBlind };

export class Player {
private:
  std::string name;
  int id;
  std::vector<Card> hand;
  uint32_t chips;
  uint32_t currentBet;
  bool canCheck;
  Blind blind;
  bool hasFolded;

public:
  Player(std::string name, int id, std::vector<Card> &&hand, uint32_t chips,
         Blind blind, uint32_t currentBet = 0)
      : name(name), id(id), hand(std::move(hand)), chips(chips),
        currentBet(currentBet), canCheck(false), blind(blind),
        hasFolded(false) {}

  // Getters.
  const std::string &getName() const { return name; }
  int getId() const { return id; }
  const std::vector<Card> &getHand() const { return hand; }
  uint32_t getChips() const { return chips; }
  uint32_t getCurrentBet() const { return currentBet; }
  bool canPlayerCheck() const { return canCheck; }
  Blind getBlind() const { return blind; }
  bool hasPlayerFolded() const { return hasFolded; }

  // Setters.
  void setChips(uint32_t newChips) { chips = newChips; }
  void setCurrentBet(uint32_t newBet) { currentBet = newBet; }
  void setCanCheck(bool check) { canCheck = check; }
  void setBlind(Blind newBlind) { blind = newBlind; }
  void setHasFolded(bool folded) { hasFolded = folded; }

  void receiveCards(const std::vector<Card> &cards) { hand = cards; }

  void resetCards() { hand.clear(); }

  // Adds amount to chips
  void addChips(int amount) {

    if (amount < 0) {
      return;
    }
    chips += amount;
  }

  // Deducts amount from chips
  void deductChips(int amount) {
    if (amount < 0) {
      return;
    }
    chips -= amount;
  }

  // Subtracts what player wants to bet, and returns the betted amount for game
  // purposes.
  uint32_t bet(uint32_t amount) {
    if (amount > chips) {
      throw std::invalid_argument("Bet amount exceeds available chips");
    }
    deductChips(amount);
    currentBet += amount;
    return amount;
  }

  // Raises the players currentBet with raiseAmount.
  int raise(uint32_t globalCurrentBet, uint32_t raiseAmount) {
    uint32_t totalRaise = (globalCurrentBet - currentBet) + raiseAmount;
    if (totalRaise > chips) {
      throw std::invalid_argument("Raise amount exceeds available chips");
    }
    deductChips(totalRaise);
    currentBet += totalRaise;
    return totalRaise;
  }

  // Does nothing.
  void check();

  int call(int globalCurrentBet) {
    uint32_t toCall = globalCurrentBet - currentBet;
    if (toCall > chips) {
      toCall = chips;
    }
    deductChips(toCall);
    currentBet += toCall; // Update player's current bet
    return toCall;
  }

  void fold() { hasFolded = true; }

  void resetCurrentBet() { currentBet = 0; }
};
