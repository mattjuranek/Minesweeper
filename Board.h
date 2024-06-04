#include "Tile.h"


class Board {
private:
    int colCount;
    int rowCount;
    int mineCount;
    vector<vector<Tile>> tiles;
    vector<pair<int, string>> highScores;
public:
    Board(const string& fileName);
    void initializeBoard();
    void drawBoard(sf::RenderWindow& window);
    void revealTiles(int row, int col);
    Tile& getTile(int row, int col);
    int getColCount();
    int getRowCount();
    int getMineCount();
    bool checkWin();
    vector<pair<int, string>> getHighScores();
    void writeScore(vector<pair<int, string>> highScores, string username, int finalTime);
};

