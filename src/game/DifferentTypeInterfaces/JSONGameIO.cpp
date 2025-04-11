#include "../include/IGameIO.hpp"
#include "game.hpp"
#include <iomanip>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vector>
using json = nlohmann::json;

class JSONGame : public IGameIO {
private:
  std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
  }
  void outputToServer(const json &jsonData) {
    std::cout << jsonData.dump(4) << std::endl;
  }

public:
  void checkHoleCards(textData playerName,
                      const std::vector<Card> &playerHand) override {
    json JSONPlayerHoleCards;
    JSONPlayerHoleCards["event"] = "holeCards";
    JSONPlayerHoleCards["playerName"] = playerName;
    const std::vector<std::string> cards;
    for (auto &card : playerHand) {
      JSONPlayerHoleCards["hand"].push_back(card.cardToString());
    }

    JSONPlayerHoleCards["timeStamp"] = getTimestamp();

    outputToServer(JSONPlayerHoleCards);
    return;
  }

  void printGameState(textData gameState) override {
    json JSONgameState;
    JSONgameState["event"] = "gameState";
    JSONgameState["state"] = gameState;
    return;
  }

  void showTurnInfo(const std::shared_ptr<Player> &currentPlayer,
                    const std::shared_ptr<Player> &nextPlayer, gameStates state,
                    money highestBet, money pot) override {
    json turnInfo;

    turnInfo["event"] = "turnInfo";
    turnInfo["timestamp"] = getTimestamp();

    turnInfo["currentPlayer"] = {{"name", currentPlayer->getName()},
                                 {"chips", currentPlayer->getChips()}};

    turnInfo["currentPlayer"]["hand"] = json::array();
    for (const auto &card : currentPlayer->getHand()) {
      turnInfo["currentPlayer"]["hand"].push_back(card.cardToString());
    }

    if (nextPlayer) {
      turnInfo["nextPlayer"] = {{"name", nextPlayer->getName()}};
    } else {
      turnInfo["nextPlayer"] = nullptr;
    }

    std::string stateStr;
    switch (state) {
    case gameStates::preFlop:
      stateStr = "preFlop";
      break;
    case gameStates::flop:
      stateStr = "flop";
      break;
    case gameStates::turn:
      stateStr = "turn";
      break;
    case gameStates::river:
      stateStr = "river";
      break;
    case gameStates::showDown:
      stateStr = "showDown";
      break;
    default:
      stateStr = "unknown";
      break;
    }
    turnInfo["gameState"] = stateStr;

    turnInfo["highestBet"] = highestBet;
    turnInfo["pot"] = pot;

    outputToServer(turnInfo);
  }

  void printPlayersTable(const playersPool &players) override {
    json playersTable;

    playersTable["event"] = "playersTable";
    playersTable["timestamp"] = getTimestamp();

    playersTable["players"] = json::array();

    int playerNumber = 1;
    for (const auto &player : players) {
      json playerObj;
      playerObj["position"] = playerNumber;
      playerObj["name"] = player->getName();
      playerObj["chips"] = player->getChips();

      // Format blind status
      std::string blindStatus;
      switch (player->getBlind()) {
      case Blind::dealer:
        blindStatus = "Dealer";
        break;
      case Blind::smallBlind:
        blindStatus = "SmallBlind";
        break;
      case Blind::bigBlind:
        blindStatus = "BigBlind";
        break;
      default:
        blindStatus = "None";
        break;
      }
      playerObj["blind"] = blindStatus;

      std::string status;
      if (!player->getIsActive()) {
        status = (player->getChips() == 0 ? "Out of Chips" : "Inactive");
      } else {
        status = (player->hasPlayerFolded() ? "Folded" : "Active");
      }
      playerObj["status"] = status;

      // Add this player to the array
      playersTable["players"].push_back(playerObj);
      playerNumber++;
    }

    outputToServer(playersTable);
  }

  std::vector<actions> offerOptions(actionMap validMoves) override {
    std::vector<actions> offeredOptions;
    offeredOptions.reserve(validMoves.size());

    json availableOptions;
    availableOptions["event"] = "availableActions";
    availableOptions["timestamp"] = getTimestamp();
    availableOptions["actions"] = json::array();

    int optionCounter = 1;

    for (const auto &[act, pairVal] : validMoves) {
      if (pairVal.first) {
        offeredOptions.push_back(act);

        auto it = actionmessages.find(act);
        std::string actionText =
            (it != actionmessages.end()) ? it->second : "Unknown action";

        json actionObj;
        actionObj["id"] = optionCounter;
        actionObj["action"] = actionText;

        actionObj["actionEnum"] = static_cast<int>(act);

        if (act == actions::raise || act == actions::bet ||
            act == actions::allIn) {
          actionObj["minAmount"] = pairVal.second;
        }

        availableOptions["actions"].push_back(actionObj);
        optionCounter++;
      }
    }

    outputToServer(availableOptions);
    return offeredOptions;
  }

  money promptForActionAmount(actions act, money minAmount) override {
    json promptJson;
    promptJson["event"] = "promptAmount";
    promptJson["timestamp"] = getTimestamp();

    std::string actionName = (act == actions::bet) ? "bet" : "raise";
    promptJson["action"] = actionName;
    promptJson["minAmount"] = minAmount;

    outputToServer(promptJson);

    // In a real JSON-based implementation, we would wait for input from the
    // client For now, we'll just return the minimum amount as this is a
    // placeholder In a real implementation, this would block until receiving a
    // response
    return minAmount; // Default to minimum amount
  }

  Action getInputPlayer(std::vector<actions> offeredOptions,
                        actionMap validMoves) override {
    // Create JSON for input request
    json inputRequestJson;
    inputRequestJson["event"] = "requestAction";
    inputRequestJson["timestamp"] = getTimestamp();
    inputRequestJson["message"] = "Waiting for player action...";

    outputToServer(inputRequestJson);

    // In a real JSON-based implementation, we would wait for input from the
    // client For now, we'll default to the first offered option In a real
    // implementation, this would block until receiving a response

    if (!offeredOptions.empty()) {
      actions defaultAction = offeredOptions[0];
      money defaultAmount = validMoves[defaultAction].second;
      return Action{defaultAction, defaultAmount, 0};
    }

    // Fallback if no options
    return Action{actions::fold, 0, 0};
  }

  void logActions(std::shared_ptr<Player> player, Action action) override {
    json actionLog;
    actionLog["event"] = "playerAction";
    actionLog["timestamp"] = getTimestamp();
    actionLog["playerName"] = player->getName();

    // Convert action enum to string
    std::string actionStr;
    switch (action.action) {
    case actions::fold:
      actionStr = "fold";
      break;
    case actions::check:
      actionStr = "check";
      break;
    case actions::call:
      actionStr = "call";
      break;
    case actions::raise:
      actionStr = "raise";
      break;
    case actions::allIn:
      actionStr = "allIn";
      break;
    case actions::bet:
      actionStr = "bet";
      break;
    default:
      actionStr = "unknown";
      break;
    }

    actionLog["action"] = actionStr;
    actionLog["amount"] = action.bet;

    // Add chips remaining after action
    actionLog["chipsRemaining"] = player->getChips();

    outputToServer(actionLog);
  }
};