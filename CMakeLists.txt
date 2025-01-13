cmake_minimum_required(VERSION 3.25)
project(cards_game LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest REQUIRED)


add_library(cards)
target_sources(cards
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/cards.cppm
            src/player.cppm
            src/bestHand.cppm
    PRIVATE
        src/game.cpp
)


add_executable(poker_game src/game.cpp)
target_link_libraries(poker_game
    PRIVATE
        cards
)

add_executable(cards_test tests/cards_test.cpp)
target_link_libraries(cards_test
    PRIVATE
        cards
        GTest::GTest
        GTest::Main
)
add_test(NAME CardsTest COMMAND cards_test)

add_executable(player_test tests/player_test.cpp)
target_link_libraries(player_test
    PRIVATE
        cards
        GTest::GTest
        GTest::Main
)
add_test(NAME PlayerTest COMMAND player_test)