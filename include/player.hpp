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
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "cards.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using money = std::uint32_t;

enum class Blind { dealer, smallBlind, bigBlind, notBlind };

class Player {
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
  // Constructor
  Player(std::string name, money chips, Blind blind = Blind::notBlind,
         money currentBet = 0, std::vector<Card> hand = {});

  // Getters.
  static int getNextId();
  const std::string &getName() const;
  int getId() const;
  const std::vector<Card> &getHand() const;
  money getChips() const;
  money getCurrentBet() const;
  bool canPlayerCheck() const;
  Blind getBlind() const;
  bool getIsActive() const;
  bool hasPlayerFolded() const;

  // Setters.
  void setChips(money newChips);
  void setIsActive(bool activity);
  void setCurrentBet(money newBet);
  void setCanCheck(bool check);
  void setBlind(Blind newBlind);
  void setHasFolded(bool folded);

  // Methods.
  void receiveCards(Card card);
  void resetCards();
  void addChips(int amount);
  void deductChips(money amount);

  // Betting actions.
  int bet(money betAmount);
  int raise(money totalRaise);
  void check();
  int call(int globalCurrentBet);
  void fold();
  void resetCurrentBet();
};

#endif // PLAYER_HPP