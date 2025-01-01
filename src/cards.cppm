/* This file implements the French-Suited Cards. It does so by providing 2
 * classes.
 * Card: Simulates a single card
 *    -getSuit returns the Suit of a card which is a enum class.
 *    -getRank returns the Rank of a card which is a enum class.
 *    -cardToString returns a string representation of the card.
 * Deck: Simulates a deck of cards
 *    -initializeDeck creates a deck of 52 cards.
 *    -shuffleDeck shuffles the deck of cards using a RNG.
 *    -dealCard removes the top card from the deck and returns it.
 *    -burnCard removes the top card from the deck and disposes this.
 *    -peekCard returns the top card from the deck and doesn't affect the deck.
 *    -printDeck prints the deck of cards.
 *    -size returns the number of cards in the deck.
 *    -resetDeck resets the deck of cards unshuffled.
 *    -operator[] returns a reference to the card at the given index.
 */

module; // <--- tells the compiler: everything that follows is global
        // (pre-module) code
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

export module cards;

export enum class Suit { Clubs, Diamonds, Hearts, Spades };

export enum class Rank : int {
  Two = 2,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Ace
};

export class Card {
private:
  Suit suit;
  Rank rank;

public:
  Card(Suit s, Rank r) : suit(s), rank(r) {}
  Suit getSuit() const { return suit; }
  Rank getRank() const { return rank; }

  std::string cardToString() const {
    static const std::string suits[] = {"Clubs", "Diamonds", "Hearts",
                                        "Spades"};
    static const std::string ranks[] = {
        "Two",  "Three", "Four", "Five",  "Six",  "Seven", "Eight",
        "Nine", "Ten",   "Jack", "Queen", "King", "Ace"};
    return ranks[static_cast<int>(rank) - 2] + " of " +
           suits[static_cast<int>(suit)];
  }
};

export class Deck {
private:
  std::vector<Card> cards;

  void initializeDeck() {
    for (int s = static_cast<int>(Suit::Clubs);
         s <= static_cast<int>(Suit::Spades); s++) {
      for (int r = static_cast<int>(Rank::Two);
           r <= static_cast<int>(Rank::Ace); r++) {
        cards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
      }
    }
  }

public:
  Deck() { initializeDeck(); }

  void shuffleDeck() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
  }

  Card dealCard() {
    if (cards.empty()) {
      throw std::out_of_range("No cards left in the deck");
    }
    Card c = cards.back();
    cards.pop_back();
    return c;
  }

  void burnCard() {
    if (cards.empty()) {
      throw std::out_of_range("No cards left to burn");
    }
    cards.pop_back();
  }

  Card peekCard() {
    if (cards.empty()) {
      throw std::out_of_range("No cards to peek at");
    }
    return cards.back();
  }

  void printDeck() const {
    for (auto const &card : cards) {
      std::cout << card.cardToString() << std::endl;
    }
  }

  size_t size() const { return cards.size(); }

  void resetDeck() {
    cards.clear();
    initializeDeck();
  }

  Card &operator[](size_t index) { return cards.at(index); }

  const Card &operator[](size_t index) const { return cards.at(index); }
};
