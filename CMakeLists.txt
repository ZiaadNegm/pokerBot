cmake_minimum_required(VERSION 3.25)
project(cards_game LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
find_package(GTest REQUIRED)

# 1) cards_lib
add_library(cards_lib
    src/cards.cpp
    src/bestHand.cpp
)
target_include_directories(cards_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 2) player_lib
add_library(player_lib
    src/player.cpp
)
target_include_directories(player_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# If Deck references are needed inside player.cpp, then do:
# target_link_libraries(player_lib PRIVATE cards_lib)

# 3) poker_game executable
add_executable(poker_game
    src/main.cpp
    src/game.cpp  # <-- Make sure "game.cpp" does NOT have its own main()
)
target_link_libraries(poker_game
    PRIVATE
        cards_lib
        player_lib
)

# 4) cards_test
add_executable(cards_test tests/cards_test.cpp)
target_link_libraries(cards_test
    PRIVATE
        cards_lib
        GTest::GTest
        GTest::Main
)
add_test(NAME CardsTest COMMAND cards_test)

# 5) player_test
add_executable(player_test tests/player_test.cpp)
target_link_libraries(player_test
    PRIVATE
        player_lib
        cards_lib   # <-- Add this to fix the Deck reference
        GTest::GTest
        GTest::Main
)
add_test(NAME PlayerTest COMMAND player_test)
