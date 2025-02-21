// tests/cards_test.cpp
#include "../include/cards.hpp"
#include <gtest/gtest.h>

// Test the Card class
TEST(CardTest, CardInitialization) {
  Card card(Suit::Hearts, Rank::Ace);
  EXPECT_EQ(card.getSuit(), Suit::Hearts);
  EXPECT_EQ(card.getRank(), Rank::Ace);
  EXPECT_EQ(card.cardToString(), "Ace of Hearts");
}

TEST(CardTest, CardToString) {
  Card card(Suit::Clubs, Rank::Two);
  EXPECT_EQ(card.cardToString(), "Two of Clubs");
}

// Test the Deck class
TEST(DeckTest, DeckInitialization) {
  Deck deck;
  EXPECT_EQ(deck.size(), 52);
}

TEST(DeckTest, ShuffleDeck) {
  Deck deck1;
  Deck deck2;
  deck1.shuffleDeck();
  deck2.shuffleDeck();

  // Ensure both decks have the same size
  EXPECT_EQ(deck1.size(), deck2.size());

  // Check that the order is different
  bool decksAreDifferent = false;
  for (size_t i = 0; i < deck1.size(); ++i) {
    if (deck1[i].getSuit() != deck2[i].getSuit() ||
        deck1[i].getRank() != deck2[i].getRank()) {
      decksAreDifferent = true;
      break;
    }
  }
  EXPECT_TRUE(decksAreDifferent);
}

TEST(DeckTest, DealCard) {
  Deck deck;
  size_t initial_size = deck.size();
  deck.dealCard();
  EXPECT_EQ(deck.size(), initial_size - 1);
}

TEST(DeckTest, BurnCard) {
  Deck deck;
  size_t initial_size = deck.size();
  deck.burnCard();
  EXPECT_EQ(deck.size(), initial_size - 1);
}

TEST(DeckTest, DealCardThrowsOnEmptyDeck) {
  Deck deck;
  // Deal all 52 cards
  for (int i = 0; i < 52; ++i) {
    deck.dealCard();
  }
  // Now, deck is empty; dealing another card should throw
  EXPECT_THROW(deck.dealCard(), std::out_of_range);
}

TEST(DeckTest, BurnCardThrowsOnEmptyDeck) {
  Deck deck;
  // Burn all 52 cards
  for (int i = 0; i < 52; ++i) {
    deck.burnCard();
  }
  // Now, deck is empty; burning another card should throw
  EXPECT_THROW(deck.burnCard(), std::out_of_range);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
