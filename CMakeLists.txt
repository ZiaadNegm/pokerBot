cmake_minimum_required(VERSION 3.25)
project(cards_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

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

#
# 3) game_lib
#
file(GLOB GAME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp")

add_library(game_lib ${GAME_SOURCES})
target_include_directories(game_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game_lib
    PRIVATE
        cards_lib
        player_lib
)

#
# 4) manager_lib
#
add_library(manager_lib
    src/manager.cpp
)
target_include_directories(manager_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(manager_lib
    PRIVATE
        game_lib
)

#
# 5) poker_game executable
#
add_executable(poker_game
    src/main.cpp
)
target_link_libraries(poker_game
    PRIVATE
        manager_lib
)

#
# 6) cards_test
#
add_executable(cards_test tests/cards_test.cpp)
target_link_libraries(cards_test
    PRIVATE
        cards_lib
        GTest::GTest
        GTest::Main
)
add_test(NAME CardsTest COMMAND cards_test)

#
# 7) player_test
#
add_executable(player_test tests/player_test.cpp)
target_link_libraries(player_test
    PRIVATE
        player_lib 
        cards_lib 
        manager_lib
        GTest::GTest
        GTest::Main
)
add_test(NAME PlayerTest COMMAND player_test)

#
# 8) manager_test
#
add_executable(manager_test tests/manager_test.cpp)
target_link_libraries(manager_test
    PRIVATE
        manager_lib
        GTest::GTest
        GTest::Main
)
add_test(NAME ManagerTest COMMAND manager_test)
