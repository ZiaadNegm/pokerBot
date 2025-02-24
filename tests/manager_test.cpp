#include "../include/manager.hpp"
#include <gtest/gtest.h>
#include <iostream>

// We do NOT put this in a different namespace (e.g. not in `namespace test {}`)
// because Manager is in the global namespace and we've declared a friend struct
// ManagerTest.

namespace {

// Optional helper for debugging
void printBlindValues(const Manager &manager) {
  std::cout << "Current blind values:\n";
  for (size_t i = 0; i < manager.players.size(); i++) {
    std::string name = manager.players[i]->getName();
    std::cout << "  Player " << i << " (" << name << ")"
              << ", Blind: " << static_cast<int>(manager.players[i]->getBlind())
              << ", Active: "
              << (manager.players[i]->getIsActive() ? "true" : "false") << "\n";
  }
  std::cout << std::endl;
}

// Another helper for setting active/inactive
void setPlayerActive(Manager &manager, size_t playerIndex, bool isActive) {
  if (playerIndex < manager.players.size()) {
    manager.players[playerIndex]->setIsActive(isActive);
  }
}

} // end anonymous namespace

// --------------------------
// GTest Fixture: ManagerTest
// --------------------------
struct ManagerTest : public ::testing::Test {
protected:
  Manager manager; // We can now see its private members (like manager.names)

  void SetUp() override {
    // Directly access 'names', even though it's private in Manager:
    manager.names[0] = "InitialTestName";
  }
};

// Demonstrate friend-level access

// Example test from your original code: AllPlayersActive
TEST_F(ManagerTest, AllPlayersActive) {
  // Activate all players
  for (size_t i = 0; i < manager.players.size(); i++) {
    manager.players[i]->setIsActive(true);
  }
  printBlindValues(manager);

  // Verify initial positions
  auto initialPos = manager.getSpecialPositions();
  ASSERT_EQ(manager.players[initialPos->dealerPosition]->getBlind(),
            Blind::dealer);
  ASSERT_EQ(manager.players[initialPos->posSB]->getBlind(), Blind::smallBlind);
  ASSERT_EQ(manager.players[initialPos->posBB]->getBlind(), Blind::bigBlind);

  // Rotate
  manager.arrangePlayersPosition();
  printBlindValues(manager);

  auto newPos = manager.getSpecialPositions();
  ASSERT_EQ(newPos->dealerPosition, 1);
  ASSERT_EQ(newPos->posSB, 2);
  ASSERT_EQ(newPos->posBB, 3);
}

TEST_F(ManagerTest, SomePlayersInactive) {
  std::cout << "[DEBUG] Initial blind assignment:\n";
  printBlindValues(manager);

  manager.arrangePlayersPosition();
  std::cout << "[DEBUG] After first rotation:\n";
  printBlindValues(manager);

  // Deactivate players 1 and 3
  setPlayerActive(manager, 1, false);
  setPlayerActive(manager, 3, false);

  manager.arrangePlayersPosition();
  std::cout << "[DEBUG] After second rotation:\n";
  printBlindValues(manager);

  auto newPos = manager.getSpecialPositions();
  ASSERT_EQ(newPos->dealerPosition, 2); // "Daniel"
  ASSERT_EQ(newPos->posSB, 4);          // "Johnny"
  ASSERT_EQ(newPos->posBB, 5);          // "You"
}

TEST_F(ManagerTest, MinimumActivePlayers) {
  std::cout << "[DEBUG] Test Case 3: MinimumActivePlayers\n";
  // Deactivate players 3, 4, 5 => leaving only players 0,1,2 active
  for (size_t i = 3; i < manager.players.size(); i++) {
    setPlayerActive(manager, i, false);
  }

  manager.arrangePlayersPosition();
  printBlindValues(manager);

  auto newPos = manager.getSpecialPositions();
  ASSERT_EQ(newPos->dealerPosition, 1);
  ASSERT_EQ(newPos->posSB, 2);
  ASSERT_EQ(newPos->posBB, 0);
}

TEST_F(ManagerTest, MultipleRotations) {
  // Ensure all are active at the start
  for (size_t i = 0; i < manager.players.size(); i++) {
    manager.players[i]->setIsActive(true);
  }
  std::cout << "[DEBUG] Initial setup:\n";
  printBlindValues(manager);

  // Do multiple rotations
  for (int rotation = 1; rotation <= 3; rotation++) {
    std::cout << "\n[DEBUG] Rotation #" << rotation << ":\n";

    manager.arrangePlayersPosition();
    auto pos = manager.getSpecialPositions();
    printBlindValues(manager);

    // Check the assigned blinds
    ASSERT_EQ(manager.players[pos->dealerPosition]->getBlind(), Blind::dealer);
    ASSERT_EQ(manager.players[pos->posSB]->getBlind(), Blind::smallBlind);
    ASSERT_EQ(manager.players[pos->posBB]->getBlind(), Blind::bigBlind);

    // Positions must differ
    ASSERT_NE(pos->dealerPosition, pos->posSB);
    ASSERT_NE(pos->dealerPosition, pos->posBB);
    ASSERT_NE(pos->posSB, pos->posBB);

    // After second rotation, deactivate some players
    if (rotation == 2) {
      setPlayerActive(manager, 1, false);
      setPlayerActive(manager, 3, false);
      std::cout << "[DEBUG] Deactivated players 1 and 3\n";
    }
  }
}
