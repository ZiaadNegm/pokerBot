#ifndef BESTHAND_HPP
#define BESTHAND_HPP

#include "player.hpp"
#include "cards.hpp"
#include <memory>
#include <stdexcept>
#include <vector>
#

class determineBestHand {
private:
    // References to the players and community cards
    const std::vector<std::shared_ptr<Player>> &players;
    const std::vector<Card> &communityCards;

    // Returns all players who are active and have not folded.
    std::vector<std::shared_ptr<Player>> determineActiveHands() const;

    // Finds the highest card rank from the player's hand and community cards.
    int findHighestRank(const Player &player) const;

public:
    // Constructor 
    determineBestHand(const std::vector<std::shared_ptr<Player>> &players,
                      const std::vector<Card> &communityCards);

    // Determines the winning player by comparing the highest card.
    // (For now, ties are resolved in favor of the first encountered.)
    std::shared_ptr<Player> determineWinnerByHighestCard() const;
};

#endif // BESTHAND_HPP