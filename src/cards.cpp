#include "cards.hpp"

// Static arrays for string representation.
static const std::string suits[] = { "Clubs", "Diamonds", "Hearts", "Spades" };
static const std::string rankStrings[] = {
    "Two", "Three", "Four", "Five", "Six", "Seven",
    "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"
};

std::string Card::cardToString() const {
    // Since Rank::Two has the integer value 2, subtract 2 to index into rankStrings.
    return rankStrings[static_cast<int>(rank) - 2] + " of " + suits[static_cast<int>(suit)];
}

void Deck::initializeDeck() {
    cards.clear();
    for (int s = static_cast<int>(Suit::Clubs); s <= static_cast<int>(Suit::Spades); ++s) {
        for (int r = static_cast<int>(Rank::Two); r <= static_cast<int>(Rank::Ace); ++r) {
            cards.emplace_back(static_cast<Suit>(s), static_cast<Rank>(r));
        }
    }
}

Deck::Deck() {
    initializeDeck();
}

void Deck::shuffleDeck() {
    // Use static random objects so they're not reinitialized on every call.
    static std::random_device rd;
    static std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
}

Card Deck::dealCard() {
    if (cards.empty()) {
        throw std::out_of_range("No cards left in the deck");
    }
    Card c = cards.back();
    cards.pop_back();
    return c;
}

void Deck::burnCard() {
    if (cards.empty()) {
        throw std::out_of_range("No cards left to burn");
    }
    cards.pop_back();
}

Card Deck::peekCard() {
    if (cards.empty()) {
        throw std::out_of_range("No cards to peek at");
    }
    return cards.back();
}

size_t Deck::size() const{
    return cards.size();
}

Card &Deck::operator[](size_t index){
    return cards.at(index);
}

const Card &Deck::operator[](size_t index) const {
    return cards.at(index);
}