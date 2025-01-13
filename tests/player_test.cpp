#include <memory>
import player;
import cards;

#include <gtest/gtest.h>

static std::vector<Card> mockHand() {
  std::vector<Card> hand;
  Deck deck;
  deck.shuffleDeck();
  for (int i = 0; i < 2; i++) {
    hand.push_back(deck.dealCard());
  }
  return hand;
}

class PlayerTest : public ::testing::Test {
protected:
  std::unique_ptr<Player> p;

  void SetUp() override {
    std::vector<Card> hand = mockHand();
    p = std::make_unique<Player>("Alice", 1000, Blind::smallBlind, 50,
                                 std::move(hand));
  }
};

TEST_F(PlayerTest, ConstructorAndGetters) {
  EXPECT_EQ(p->getName(), "Alice");
  EXPECT_EQ(p->getId(), 0); // Changed expectation to 0 since IDs start at 0
  EXPECT_EQ(p->getHand().size(), 2);
  EXPECT_EQ(p->getChips(), 1000);
  EXPECT_EQ(p->getCurrentBet(), 50);
  EXPECT_FALSE(p->canPlayerCheck());
  EXPECT_EQ(p->getBlind(), Blind::smallBlind);
  EXPECT_FALSE(p->hasPlayerFolded());
}

TEST_F(PlayerTest, SettersAndGetters) {
  // setChips
  p->setChips(2000);
  EXPECT_EQ(p->getChips(), 2000u);

  // setCurrentBet
  p->setCurrentBet(100);
  EXPECT_EQ(p->getCurrentBet(), 100u);

  // setCanCheck
  p->setCanCheck(true);
  EXPECT_TRUE(p->canPlayerCheck());

  // setBlind
  p->setBlind(Blind::bigBlind);
  EXPECT_EQ(p->getBlind(), Blind::bigBlind);

  // setHasFolded
  p->setHasFolded(true);
  EXPECT_TRUE(p->hasPlayerFolded());
}

TEST_F(PlayerTest, ReceiveAndResetCards) {
  // Original hand had 2 cards, let's set new cards
  std::vector<Card> newHand;
  newHand.emplace_back(Suit::Diamonds, Rank::Two);
  newHand.emplace_back(Suit::Clubs, Rank::Three);

  p->receiveCards(newHand);
  EXPECT_EQ(p->getHand().size(), 2u);
  EXPECT_EQ(p->getHand()[0].getRank(), Rank::Two);
  EXPECT_EQ(p->getHand()[1].getRank(), Rank::Three);

  p->resetCards();
  EXPECT_TRUE(p->getHand().empty());
}

TEST_F(PlayerTest, AddAndDeductChips) {
  // addChips
  p->addChips(500);
  EXPECT_EQ(p->getChips(), 1500u);

  // addChips with negative value: does nothing
  p->addChips(-300);
  EXPECT_EQ(p->getChips(), 1500u);

  // deductChips
  p->deductChips(200);
  EXPECT_EQ(p->getChips(), 1300u);

  // deductChips with negative value: does nothing
  p->deductChips(-100);
  EXPECT_EQ(p->getChips(), 1300u);
}

TEST_F(PlayerTest, Bet) {
  // Player has 1000 chips originally, but we added 500, then subtracted 200
  // from the previous test => 1300 chips left
  p->setChips(1000);
  p->setCurrentBet(50);

  // Bet with an amount less than or equal to chips
  uint32_t betAmount = p->bet(200);
  EXPECT_EQ(betAmount, 200u);
  EXPECT_EQ(p->getChips(), 800u);      // 1000 - 200
  EXPECT_EQ(p->getCurrentBet(), 250u); // 50 + 200

  // Bet with an amount more than available => should throw
  EXPECT_THROW(p->bet(2000), std::invalid_argument);
}

TEST_F(PlayerTest, Raise) {
  // Reset player state
  p->setChips(1000);
  p->setCurrentBet(100);

  // Suppose the global current bet is 200
  // A raise means we must at least call (200 - currentBet = 100) plus
  // raiseAmount Example: raise(200, 50) => totalRaise = 100 (call) + 50 = 150
  int totalRaise = p->raise(200, 50);
  EXPECT_EQ(totalRaise, 150);
  EXPECT_EQ(p->getChips(), 850u); // 1000 - 150
  EXPECT_EQ(p->getCurrentBet(), 250u);

  // Attempt a raise that exceeds chips
  EXPECT_THROW(p->raise(1000, 2000), std::invalid_argument);
}

TEST_F(PlayerTest, Call) {
  // Reset
  p->setChips(500);
  p->setCurrentBet(100);

  // If global bet is 200, we only need to pay 100 more
  int toCall = p->call(200);
  EXPECT_EQ(toCall, 100);
  EXPECT_EQ(p->getChips(), 400u);
  EXPECT_EQ(p->getCurrentBet(), 200u);

  // If global bet is extremely high, we can only call up to our chips
  p->setCurrentBet(200); // Suppose we've matched the bet up to 200
  p->setChips(50);       // We only have 50 left
  toCall = p->call(1000);
  EXPECT_EQ(toCall, 50);               // All in
  EXPECT_EQ(p->getChips(), 0u);        // 50 - 50
  EXPECT_EQ(p->getCurrentBet(), 250u); // 200 + 50
}

TEST_F(PlayerTest, FoldAndResetCurrentBet) {
  p->fold();
  EXPECT_TRUE(p->hasPlayerFolded());

  p->resetCurrentBet();
  EXPECT_EQ(p->getCurrentBet(), 0u);
}

class PlayerBasicTest : public ::testing::Test {};

TEST_F(PlayerBasicTest, BasicConstruction) {
  std::unique_ptr<Player> p =
      std::make_unique<Player>("Alice", 1000, Blind::notBlind);
  EXPECT_EQ(p->getName(), "Alice");
  EXPECT_EQ(p->getChips(), 1000);
}
