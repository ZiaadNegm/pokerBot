class Game {
public:
    Game(/* constructor arguments if needed */) {
        // Initialization code
    }
};

class Manager {
public:
    Manager() : game(/* constructor arguments if needed */) {
        // Initialization code
    }

private:
    Game game; // The game containing all the core logic.
};