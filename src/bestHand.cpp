#include "bestHand.hpp"
#include <algorithm>

std::vector<std::shared_ptr<Player>> determineBestHand::determineActiveHands() const {
    std::vector<std::shared_ptr<Player>> activePlayers;
    activePlayers.reserve(players.size());
    for (const auto &p : players) {
        if (!p->hasPlayerFolded() && p->getIsActive()) {
            activePlayers.push_back(p);
        }
    }
    return activePlayers;
}

int determineBestHand::findHighestRank(const Player &player) const {
    int maxRankValue = 0;
    // Lambda to update max if the current card's rank is higher.
    auto updateMax = [&maxRankValue](const Card &card) {
        int rankVal = static_cast<int>(card.getRank());
        if (rankVal > maxRankValue) {
            maxRankValue = rankVal;
        }
    };

    // Process player's hand.
    for (const auto &card : player.getHand()) {
        updateMax(card);
    }
    // Process community cards.
    for (const auto &card : communityCards) {
        updateMax(card);
    }
    return maxRankValue;
}

determineBestHand::determineBestHand(
    const std::vector<std::shared_ptr<Player>> &players,
    const std::vector<Card> &communityCards)
    : players(players), communityCards(communityCards) {}

std::shared_ptr<Player> determineBestHand::determineWinnerByHighestCard() const {
    auto activePlayers = determineActiveHands();
    if (activePlayers.empty()) {
        throw std::runtime_error("No active players in the showdown.");
    }

    auto bestPlayer = activePlayers.front();
    int bestRank = findHighestRank(*bestPlayer);

    // Use std::max_element to scan remaining players.
    for (size_t i = 1; i < activePlayers.size(); ++i) {
        int currentRank = findHighestRank(*activePlayers[i]);
        if (currentRank > bestRank) {
            bestRank = currentRank;
            bestPlayer = activePlayers[i];
        }
    }
    return bestPlayer;
}