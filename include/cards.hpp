#ifndef CARDS_HPP
#define CARDS_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <random>

// Suits & Ranks definitions.
enum class Suit { Clubs, Diamonds, Hearts, Spades };
enum class Rank { Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace };

class Card {
private:
    Suit suit;
    Rank rank;
public:
    Card(Suit s, Rank r) : suit(s), rank(r) {}
    
    Suit getSuit() const { return suit; }
    Rank getRank() const { return rank; }
    
    // Returns a string representation of the card, e.g., "Ace of Spades"
    std::string cardToString() const;
};

class Deck {
private:
    std::vector<Card> cards;
    // Builds a full deck of 52 cards.
    void initializeDeck();

public:
    Deck();

    // Shuffles the deck using a static random engine.
    void shuffleDeck();



    // Deals the top card from the deck.
    Card dealCard();

    // Returns the size of the deck.
    size_t size() const;

    // Burns (discards) the top card.
    void burnCard();

    // Peeks at the top card without removing it.
    Card peekCard();

    // Returns a reference to the card at the given index.
    Card &operator[](size_t index);

    // Returns a const reference to the card at the given index.
    const Card &operator[](size_t index) const;
};

#endif // CARDS_HPP