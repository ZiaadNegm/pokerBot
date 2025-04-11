#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

using json = nlohmann::json;
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast/core.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = net::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class socket {
  bool connect() {}

  bool disconnect() {}
  bool isConnected() const {}
  // Just send, but don't receive anythng back.
  void outputToServer(const json &message) {}

  // Send and we expect something back so we block untill we received answer
  json expectingResponse(const json &request) {
    // determine what type of event to pass
    send(request);
    return waitForMessageType;
  }

  json waitForMessageType(const std::string &expectedType) {
    // block and wait untill we see expectedType.

    return (); // return actual unfiltered response
  }
};