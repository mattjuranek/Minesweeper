#include "Board.h"


Board::Board(const string& fileName) {
    ifstream file(fileName);
    file >> colCount >> rowCount >> mineCount;
    file.close();
}

void Board::initializeBoard() {
    // Resize tiles vector to rowCount and colCount
    tiles.resize(rowCount, vector<Tile>(colCount, Tile()));

    // Place mines randomly
    random_device randomDevice;
    mt19937 randomGen(randomDevice());
    uniform_int_distribution<int> distribution(0, rowCount * colCount - 1);

    for (int i = 0; i < mineCount; i++) {
        int index = distribution(randomGen);
        int row = index / colCount;
        int col = index % colCount;

        while (tiles[row][col].getIsMine()) {
            index = distribution(randomGen);
            row = index / colCount;
            col = index % colCount;
        }

        tiles[row][col].setMine();
    }

    // Initialize non-mine tiles with number of adjacent mines
    for (int row = 0; row < rowCount; row++) {
        for (int col = 0; col < colCount; col++) {

            // If tile is not a mine
            if (!tiles[row][col].getIsMine()) {
                int adjacentMines = 0;

                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        int adjRow = row + i;
                        int adjCol = col + j;

                        // Check if adjacent tile is on board and is a mine
                        if (adjRow >= 0 && adjRow < rowCount && adjCol >= 0 && adjCol < colCount && tiles[adjRow][adjCol].getIsMine()) {
                            adjacentMines++;
                        }
                    }
                }

                // Set the value of adjacent mines for the tile
                tiles[row][col].setAdjacentMines(adjacentMines);
            }
        }
    }
}

void Board::drawBoard(sf::RenderWindow &window) {
    for (int row = 0; row < rowCount; row++) {
        for (int col = 0; col < colCount; col++) {

            // Get size and position of tile
            Tile& tile = getTile(row, col);
            sf::Vector2u tileSize = tile.getTileSize();
            sf::Vector2f tilePosition(col * tileSize.x, row * tileSize.y);

            // Set sprite as tileHidden sprite by default
            sf::Sprite& tileSprite = tile.getTileHiddenSprite();

            // If tile is revealed, set sprite as tileRevealed sprite
            if (tile.getIsRevealed()) {
                tileSprite = tile.getTileRevealedSprite();
            }

            // Set position and draw sprite for the tile
            tileSprite.setPosition(tilePosition);
            window.draw(tileSprite);


            // If tile is revealed is a mine, draw mineSprite
            if (tile.getIsRevealed() && tile.getIsMine()) {
                sf::Sprite& mineSprite = tile.getMineSprite();
                mineSprite.setPosition(tilePosition);
                window.draw(mineSprite);
            }

            // If tile is revealed and has adjacent mines, draw numberSprite
            else if (tile.getIsRevealed() && tile.getAdjacentMines() > 0) {
                sf::Sprite& numberSprite = tile.getNumberSprite();
                numberSprite.setPosition(tilePosition);
                window.draw(numberSprite);
            }

            // If tile is flagged, draw flagSprite
            else if (tile.getIsFlagged()) {
                sf::Sprite& flagSprite = tile.getFlagSprite();
                flagSprite.setPosition(tilePosition);
                window.draw(flagSprite);
            }
        }
    }
}

void Board::revealTiles(int row, int col) {
    Tile& tile = getTile(row, col);

    // If tile is flagged or already revealed, skip it
    if (tile.getIsFlagged() || tile.getIsRevealed()) {
        return;
    }

    // Reveal current tile
    tile.revealTile();

    // If tile is adjacent to a mine, reveal only that tile
    if (tile.getAdjacentMines() != 0) {
        return;
    }

    // Check every row and col from current tile until one has adjacent mines
    if (tile.getAdjacentMines() == 0) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {

                // Skip the current tile
                if (i == 0 && j == 0) {
                    continue;
                }

                // Set new row and col
                int newRow = row + i;
                int newCol = col + j;

                // Check if new row and col are within bounds, call revealTiles recursively
                if (newRow >= 0 && newRow < getRowCount() && newCol >= 0 && newCol < getColCount()) {
                    revealTiles(newRow, newCol);
                }
            }
        }
    }
}

Tile& Board::getTile(int row, int col) {
    return tiles[row][col];
}

int Board::getColCount() {
    return colCount;
}

int Board::getRowCount() {
    return rowCount;
}

int Board::getMineCount() {
    return mineCount;
}

bool Board::checkWin() {
    for (int row = 0; row < rowCount; row++) {
        for (int col = 0; col < colCount; col++) {

            Tile& tile = getTile(row, col);

            // Check if any non-mine tiles are not revealed, return false
            if (!tile.getIsMine() && !tile.getIsRevealed()) {
                return false;
            }
        }
    }

    // If all non-mine tiles revealed, return true
    return true;
}

vector<pair<int, string>> Board::getHighScores() {
    string line;
    bool hasAsterisk = false;
    ifstream leaderboardFile("../files/leaderboard.txt");

    if (!leaderboardFile.is_open()) {
        cout << "Cannot open!" << endl;
        return highScores;
    }

    // Read lines from leaderboardFile
    while (getline(leaderboardFile, line)) {
        stringstream stringStream(line);
        string timeString;
        string username;

        getline(stringStream, timeString, ',');
        getline(stringStream, username);

        int minutes;
        int seconds;
        char colon;

        stringstream timeStream(timeString);
        timeStream >> minutes >> colon >> seconds;

        int totalSeconds = (minutes * 60) + seconds;

        // Check is a line has an asterisk and remove it
        if (username.back() == '*') {
            username.pop_back();
            hasAsterisk = true;
        }

        highScores.emplace_back(totalSeconds, username);
    }

    // Close file and sort vector
    leaderboardFile.close();
    sort(highScores.begin(), highScores.end());

    // If a high score has an asterisk, rewrite it without
    if (hasAsterisk) {
        ofstream leaderboardFileWrite("../files/leaderboard.txt");

        if (!leaderboardFileWrite.is_open()) {
            cout << "Cannot open!" << endl;
            return highScores;
        }

        for (int i = 0; i < highScores.size(); i++) {
            int minutes = highScores[i].first / 60;
            int seconds = highScores[i].first % 60;

            // Format minutesString
            string minutesString;
            if (minutes < 10) {
                minutesString = "0" + to_string(minutes);
            }

            else {
                minutesString = to_string(minutes);
            }

            // Format secondsString
            string secondsString;
            if (seconds < 10) {
                secondsString = "0" + to_string(seconds);
            }

            else {
                secondsString = to_string(seconds);
            }

            // Write score to leaderboardFile in correct format
            leaderboardFileWrite << minutesString << ":" << secondsString << "," << highScores[i].second << endl;
        }

        leaderboardFileWrite.close();
    }

    return highScores;
}

void Board::writeScore(vector<pair<int, string>> highScores, string username, int finalTime) {
    // Insert new score to vector, adding asterisk
    highScores.emplace_back(finalTime, username + "*");

    // Sort vector
    sort(highScores.begin(), highScores.end());

    // Resize vector to a limit of 5
    if (highScores.size() > 5) {
        highScores.resize(5);
    }

    ofstream leaderboardFile("../files/leaderboard.txt");

    if (!leaderboardFile.is_open()) {
        cout << "Cannot open!" << endl;
        return;
    }

    // Calculate time for each score
    for (int i = 0; i < highScores.size(); i++) {
        int minutes = highScores[i].first / 60;
        int seconds = highScores[i].first % 60;

        // Format minutesString
        string minutesString;
        if (minutes < 10) {
            minutesString = "0" + to_string(minutes);
        }

        else {
            minutesString = to_string(minutes);
        }

        // Format secondsString
        string secondsString;
        if (seconds < 10) {
            secondsString = "0" + to_string(seconds);
        }

        else {
            secondsString = to_string(seconds);
        }

        // Write score to leaderboardFile in correct format
        leaderboardFile << minutesString << ":" << secondsString << "," << highScores[i].second << endl;
    }

    leaderboardFile.close();
}

