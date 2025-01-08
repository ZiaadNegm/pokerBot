module;
#include <deque>
#include <stdexcept> // for std::runtime_error
#include <vector>

export module bestHand;
import player;
import cards;

// Add 'export' keyword before the class declaration
export class determineBestHand {
private:
  // Hold references to players and community cards to avoid copying
  const std::deque<Player> &players;
  const std::vector<Card> &communityCards;

  // Return all players who haven't folded
  std::vector<const Player *> DetermineActiveHands() const {
    std::vector<const Player *> activePlayers;
    for (const auto &p : players) { // Use const reference to avoid copying
      if (!p.hasPlayerFolded()) {
        activePlayers.emplace_back(&p); // Store pointers to avoid copying
      }
    }
    return activePlayers;
  }

  // Find the highest card rank among one player's 2-hole + 5-community
  int findHighestRank(const Player &player) const {
    int maxRankValue = 0;

    // Check player's hole cards
    for (const auto &c : player.getHand()) {
      int rankVal = static_cast<int>(c.getRank());
      if (rankVal > maxRankValue) {
        maxRankValue = rankVal;
      }
    }

    // Check community cards
    for (const auto &c : communityCards) {
      int rankVal = static_cast<int>(c.getRank());
      if (rankVal > maxRankValue) {
        maxRankValue = rankVal;
      }
    }
    return maxRankValue;
  }

public:
  // Constructor now takes references to players and community cards
  determineBestHand(const std::deque<Player> &players_,
                    const std::vector<Card> &communityCards_)
      : players(players_), communityCards(communityCards_) {}

  // Determine the winner by highest single card among active players
  const Player *determineWinnerByHighestCard() const {
    // Gather players who haven't folded
    std::vector<const Player *> activePlayers = DetermineActiveHands();
    if (activePlayers.empty()) {
      throw std::runtime_error("No active players in the showdown.");
    }

    // Start with the first active player as "best"
    const Player *bestPlayer = activePlayers[0];
    int bestRank = findHighestRank(*bestPlayer);

    // Compare all active players' highest card
    for (size_t i = 1; i < activePlayers.size(); ++i) {
      int rankVal = findHighestRank(*activePlayers[i]);
      if (rankVal > bestRank) {
        bestRank = rankVal;
        bestPlayer = activePlayers[i];
      }
    }

    // Return pointer to the winning player
    return bestPlayer;
  }
};
