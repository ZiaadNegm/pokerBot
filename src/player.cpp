#include "../include/player.hpp"

// Constructor.
Player::Player(std::string name, money chips, Blind blind, money currentBet,
               std::vector<Card> hand)
    : id(getNextId()), name(std::move(name)), hand(std::move(hand)),
      chips(chips), currentBet(currentBet), canCheck(false), blind(blind),
      hasFolded(false), isActive(true) {}

// Getters.
int Player::getNextId() { return nextId++; }
const std::string &Player::getName() const { return name; }
int Player::getId() const { return id; }
const std::vector<Card> &Player::getHand() const { return hand; }
money Player::getChips() const { return chips; }
money Player::getCurrentBet() const { return currentBet; }
bool Player::canPlayerCheck() const { return canCheck; }
Blind Player::getBlind() const { return blind; }
bool Player::getIsActive() const { return isActive; }
bool Player::hasPlayerFolded() const { return hasFolded; }

// Setters.
void Player::setChips(money newChips) { chips = newChips; }
void Player::setIsActive(bool activity) { isActive = activity; }
void Player::setCurrentBet(money newBet) { currentBet = newBet; }
void Player::setCanCheck(bool check) { canCheck = check; }
void Player::setBlind(Blind newBlind) { blind = newBlind; }
void Player::setHasFolded(bool folded) { hasFolded = folded; }

// Methods.
void Player::receiveCards(Card card) { hand.push_back(card); }
void Player::resetCards() { hand.clear(); }

void Player::addChips(int amount) {
  if (amount < 0) {
    throw std::invalid_argument("Can't add negative chips");
  }
  chips += amount;
}

// Private helper.
void Player::deductChips(money amount) {
  if (amount > chips) {
    throw std::invalid_argument("Not enough chips");
  }
  chips -= amount;
}

// Betting actions.
int Player::bet(money betAmount) {
  if (betAmount > chips) {
    throw std::invalid_argument("Not enough chips to bet");
  }
  deductChips(betAmount);
  currentBet += betAmount;
  return betAmount;
}

int Player::raise(money totalRaise) {
  money diff = totalRaise - currentBet;
  if (diff > chips) {
    throw std::invalid_argument("Not enough chips to raise by that amount.");
  }
  deductChips(diff);
  currentBet = totalRaise;
  return diff;
}

void Player::check() {
  // Does nothing.
}

int Player::call(int globalCurrentBet) {
  deductChips(globalCurrentBet);
  currentBet += globalCurrentBet;
  return globalCurrentBet;
}

void Player::fold() { hasFolded = true; }

void Player::resetCurrentBet() { currentBet = 0; }