module;
#include <stdexcept> // for std::runtime_error
#include <vector>

#include <memory>

export module bestHand;
import player;
import cards;

// Add 'export' keyword before the class declaration
export class determineBestHand {
private:
  // Store references to a vector of shared_ptr<Player> and the community cards.
  const std::vector<std::shared_ptr<Player>> &players;
  const std::vector<Card> &communityCards;

  // Return all players who haven't folded and are still active.
  std::vector<std::shared_ptr<Player>> DetermineActiveHands() const {
    std::vector<std::shared_ptr<Player>> activePlayers;
    activePlayers.reserve(players.size());

    for (auto &p : players) {
      if (!p->hasPlayerFolded() && p->getIsActive()) {
        activePlayers.push_back(p);
      }
    }
    return activePlayers;
  }

  // Find the highest rank among one player's 2-hole cards + the community
  // cards. For simplicity, we look only at the single highest card rank.
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
  // Constructor now takes a reference to a vector of shared_ptr<Player> and a
  // reference to community cards
  determineBestHand(const std::vector<std::shared_ptr<Player>> &players_,
                    const std::vector<Card> &communityCards_)
      : players(players_), communityCards(communityCards_) {}

  // Determine the winner by comparing only the highest single card among active
  // players.
  std::shared_ptr<Player> determineWinnerByHighestCard() const {
    // Gather players who haven't folded
    std::vector<std::shared_ptr<Player>> activePlayers = DetermineActiveHands();
    if (activePlayers.empty()) {
      throw std::runtime_error("No active players in the showdown.");
    }

    // Start with the first active player as "best"
    std::shared_ptr<Player> bestPlayer = activePlayers[0];
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