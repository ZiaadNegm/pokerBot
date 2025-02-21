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
module;
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

export module player;
import cards;

using money = std::uint32_t;

export enum class Blind { dealer, smallBlind, bigBlind, notBlind };

export class Player {
private:
  inline static int nextId = 0;
  int id;
  std::string name;
  std::vector<Card> hand;
  money chips;
  money currentBet;
  bool canCheck;
  Blind blind;
  bool hasFolded;
  bool isActive;

public:
  Player(std::string name, money chips, Blind blind = Blind::notBlind,
         money currentBet = 0, std::vector<Card> hand = {})
      : id(getNextId()), name(name), hand(hand), chips(chips),
        currentBet(currentBet), canCheck(false), blind(blind), hasFolded(false),
        isActive(true) {}

  // Getters.
  static int getNextId() { return nextId++; }
  const std::string &getName() const { return name; }
  int getId() const { return id; }
  const std::vector<Card> &getHand() const { return hand; }
  money getChips() const { return chips; }
  money getCurrentBet() const { return currentBet; }
  bool canPlayerCheck() const { return canCheck; }
  Blind getBlind() const { return blind; }
  bool getIsActive() const { return isActive; }
  bool hasPlayerFolded() const { return hasFolded; }

  // Setters.
  void setChips(money newChips) { chips = newChips; }
  void setIsActive(bool activity) { isActive = activity; }
  void setCurrentBet(money newBet) { currentBet = newBet; }
  void setCanCheck(bool check) { canCheck = check; }
  void setBlind(Blind newBlind) { blind = newBlind; }
  void setHasFolded(bool folded) { hasFolded = folded; }

  void receiveCards(Card card) { hand.emplace_back(card); }

  void resetCards() { hand.clear(); }

  // Adds amount to chips
  void addChips(int amount) {

    if (amount < 0) {
      throw std::invalid_argument("Can't add negative chips");
    }
    chips += amount;
  }

  // Deducts amount from chips
  void deductChips(int amount) {
    if (amount < 0) {
      throw std::invalid_argument("Underflowing");
    }
    chips -= amount;
  }

  // Subtracts what player wants to bet, and returns the betted amount for game
  // purposes.
  money bet(money amount) {
    if (amount > chips) {
      throw std::invalid_argument("Bet amount exceeds available chips");
    }
    deductChips(amount);
    currentBet += amount;
    return amount;
  }

  // Raises the players currentBet with raiseAmount.
  int raise(money totalRaise) {
    money diff = totalRaise - currentBet;
    if (diff > chips) {
      throw std::invalid_argument("Not enough chips to raise by that amount.");
    }
    chips -= diff;
    currentBet = totalRaise;
    return diff;
  }

  // Does nothing.
  void check(){};

  int call(int globalCurrentBet) {
    deductChips(globalCurrentBet);
    currentBet += globalCurrentBet;
    return globalCurrentBet;
  }

  void fold() { hasFolded = true; }

  void resetCurrentBet() { currentBet = 0; }
};