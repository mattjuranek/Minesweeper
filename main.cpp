#include "Board.h"


void setText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

void setSprite(sf::Sprite &sprite, float x, float y) {
    sprite.setPosition(sf::Vector2f(x, y));
}

void setRect(sf::RectangleShape &rect, float x, float y) {
    rect.setPosition(sf::Vector2f(x, y));
}

sf::Font loadFont() {
    sf::Font font;

    if (!font.loadFromFile("../files/font.ttf")) {
        cout << "Cannot load!" << endl;
    }

    return font;
}

map<int, sf::Sprite> splitDigits(sf::Sprite digits){
    map<int, sf::Sprite> digitsMap;

    // Set sprite for digits (0-9)
    for(int i = 0; i < 10; i++){
        sf::IntRect rect(i * 21,0,21,32);
        digits.setTextureRect(rect);
        sf::Sprite sprite = digits;
        digitsMap.emplace(i, sprite);
    }

    // Set sprite for negative (-)
    sf::IntRect rectNegative(10*21, 0, 21, 32); // The X position is '10*21' if it's right after the '9'
    digits.setTextureRect(rectNegative);
    sf::Sprite sprite = digits;
    digitsMap.emplace(-1, sprite);

    return digitsMap;
}

vector<string> splitString(const string &str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);

    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

void loadCounter(sf::RenderWindow& window, const map<int, sf::Sprite>& digitsMap, int flagCount) {
    bool isNegative = flagCount < 0;
    flagCount = abs(flagCount);
    Board board("../files/config.cfg");
    sf::Sprite sprite;

    // Calculate digit indexes
    int hundreds = (flagCount / 100) % 10;
    int tens = (flagCount / 10) % 10;
    int ones = flagCount % 10;

    // Draw negative sprite
    if (isNegative) {
        sf::Sprite negativeSprite = digitsMap.at(-1);
        negativeSprite.setPosition(12, 32 * (board.getRowCount() + 0.5) + 16);
        window.draw(negativeSprite);
    }

    // Draw hundreds sprite
    sprite = digitsMap.at(hundreds);
    sprite.setPosition(33, 32 * (board.getRowCount() + 0.5) + 16);
    window.draw(sprite);

    // Draw tens sprite
    sprite = digitsMap.at(tens);
    sprite.setPosition(54, 32 * (board.getRowCount() + 0.5) + 16);
    window.draw(sprite);

    // Draw ones sprite
    sprite = digitsMap.at(ones);
    sprite.setPosition(75, 32 * (board.getRowCount() + 0.5) + 16);
    window.draw(sprite);
}

bool loadLeaderboardWindow() {
    Board board("../files/config.cfg");
    int LBWindowWidth = board.getColCount() * 16;
    int LBWindowHeight = (board.getRowCount() * 16) + 50;
    vector<string> leaderboardEntries;
    string line;

    sf::RenderWindow leaderboardWindow(sf::VideoMode(LBWindowWidth, LBWindowHeight), "Minesweeper");
    sf::Font font = loadFont();

    // Leaderboard
    sf::Text leaderboardText;
    leaderboardText.setString("LEADERBOARD");
    leaderboardText.setFont(font);
    leaderboardText.setCharacterSize(20);
    leaderboardText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    leaderboardText.setFillColor(sf::Color::White);
    setText(leaderboardText, LBWindowWidth / 2.0f, (LBWindowHeight / 2.0f) - 120);

    // Open leaderboard file
    ifstream leaderboardFile("../files/leaderboard.txt");

    if (!leaderboardFile.is_open()) {
        cout << "Cannot open!" << endl;
        return false;
    }

    // Read file and store contents in vector
    while (getline(leaderboardFile, line)) {
        leaderboardEntries.push_back(line);
    }

    leaderboardFile.close();

    while (leaderboardWindow.isOpen()) {

        // EVENT HANDLING
        sf::Event event;
        while (leaderboardWindow.pollEvent(event)) {

            // If user closes leaderboard window
            if (event.type == sf::Event::Closed) {
                leaderboardWindow.close();
                return false;
            }
        }

        // Display leaderboard content
        int rank = 1;
        string leaderboardContents;

        // Display top 5 entries
        for (int i = 0; (i < leaderboardEntries.size()) && (rank <= 5); i++) {
            const string &entryLine = leaderboardEntries[i];
            vector<string> entry = splitString(entryLine, ',');

            leaderboardContents += to_string(rank) + ".\t" + entry[0] + "\t" + entry[1] + "\n\n";
            rank++;
        }

        leaderboardWindow.clear(sf::Color::Blue);
        leaderboardWindow.draw(leaderboardText);

        // Draw text
        sf::Text rankText;
        rankText.setFont(font);
        rankText.setString(leaderboardContents);
        rankText.setCharacterSize(18);
        rankText.setFillColor(sf::Color::White);
        rankText.setStyle(sf::Text::Bold);
        setText(rankText, LBWindowWidth / 2.0f, (LBWindowHeight / 2.0f)  + 20);
        leaderboardWindow.draw(rankText);

        leaderboardWindow.display();
    }

    return true;
}

void loadGameWindow(string username) {
    Board board("../files/config.cfg");
    chrono::high_resolution_clock::time_point startTime = chrono::high_resolution_clock::now();
    chrono::high_resolution_clock::time_point pauseTime = chrono::high_resolution_clock::now();
    auto elapsedPausedTime = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - pauseTime).count();
    int colCount = board.getColCount();
    int rowCount = board.getRowCount();
    int windowWidth = board.getColCount() * 32;
    int windowHeight = (board.getRowCount() * 32) + 100;
    int flagCount = board.getMineCount();
    vector<pair<int, string>> highScores = board.getHighScores();
    bool gameOver = false;
    bool gamePaused = false;
    bool prevPaused = false;
    bool debugOn = false;
    bool leaderboardOpen = false;
    bool scoreWritten = false;
    int minutes;
    int seconds;

    // Load game window and initialize board
    sf::RenderWindow gameWindow(sf::VideoMode(windowWidth, windowHeight), "Minesweeper");
    board.initializeBoard();
    sf::Font font = loadFont();

    // INITIALIZE TEXTURES AND SPRITES
    // Tile sprites
    sf::Texture &tileRevealedTexture = TextureManager::getTexture("tile_revealed");
    sf::Texture &tileHiddenTexture = TextureManager::getTexture("tile_hidden");

    sf::Sprite tileRevealedSprite;
    tileRevealedSprite.setTexture(tileRevealedTexture);

    sf::Sprite tileHiddenSprite;
    tileHiddenSprite.setTexture(tileHiddenTexture);

    // Mine sprite
    sf::Texture &mineTexture = TextureManager::getTexture("mine");

    sf::Sprite mineSprite;
    mineSprite.setTexture(mineTexture);

    // Happy Face Button
    sf::Texture &happyFaceTexture = TextureManager::getTexture("face_happy");

    sf::Sprite happyFaceSprite;
    happyFaceSprite.setTexture(happyFaceTexture);
    setSprite(happyFaceSprite, (colCount / 2.0f * 32) - 32, (rowCount + 0.5) * 32);

    sf::Texture &loseFaceTexture = TextureManager::getTexture("face_lose");

    sf::Sprite loseFaceSprite;
    loseFaceSprite.setTexture(loseFaceTexture);
    setSprite(loseFaceSprite, (colCount / 2.0f * 32) - 32, (rowCount + 0.5) * 32);

    sf::Texture &winFaceTexture = TextureManager::getTexture("face_win");

    sf::Sprite winFaceSprite;
    winFaceSprite.setTexture(winFaceTexture);
    setSprite(winFaceSprite, (colCount / 2.0f * 32) - 32, (rowCount + 0.5) * 32);

    sf::RectangleShape happyFaceRect(sf::Vector2f(happyFaceTexture.getSize().x, happyFaceTexture.getSize().y));

    happyFaceRect.setFillColor(sf::Color(255,165,0));
    setRect(happyFaceRect, (colCount / 2.0f * 32) - 32, (rowCount + 0.5) * 32);

    // Debug Button
    sf::Texture &debugTexture = TextureManager::getTexture("debug");

    sf::Sprite debugSprite;
    debugSprite.setTexture(debugTexture);
    setSprite(debugSprite, (colCount * 32) - 304, (rowCount + 0.5) * 32);

    sf::RectangleShape debugRect(sf::Vector2f(debugTexture.getSize().x, debugTexture.getSize().y));
    setRect(debugRect, (colCount * 32) - 304, (rowCount + 0.5) * 32);

    // Pause / Play Button
    sf::Texture &pauseTexture = TextureManager::getTexture("pause");
    sf::Texture &playTexture = TextureManager::getTexture("play");

    sf::Sprite pauseSprite;
    pauseSprite.setTexture(pauseTexture);
    setSprite(pauseSprite, (colCount * 32) - 240, (rowCount + 0.5) * 32);

    sf::Sprite playSprite;
    playSprite.setTexture(playTexture);
    setSprite(playSprite, (colCount * 32) - 240, (rowCount + 0.5) * 32);

    sf::RectangleShape pauseRect(sf::Vector2f(pauseTexture.getSize().x, pauseTexture.getSize().y));
    setRect(pauseRect, (colCount * 32) - 240, (rowCount + 0.5) * 32);

    // Leaderboard Button
    sf::Texture &leaderboardTexture = TextureManager::getTexture("leaderboard");

    sf::Sprite leaderboardSprite;
    leaderboardSprite.setTexture(leaderboardTexture);
    setSprite(leaderboardSprite, (colCount * 32) - 176, (rowCount + 0.5) * 32);

    sf::RectangleShape leaderboardRect(sf::Vector2f(leaderboardTexture.getSize().x, leaderboardTexture.getSize().y));
    setRect(leaderboardRect, (colCount * 32) - 176, (rowCount + 0.5) * 32);

    // Digits / Timer
    sf::Texture &digitsTexture = TextureManager::getTexture("digits");

    sf::Sprite timerSprite;
    timerSprite.setTexture(digitsTexture);
    setSprite(timerSprite, 33, 32 * (rowCount + 0.5) + 16);
    map<int, sf::Sprite> digitsMap = splitDigits(timerSprite);

    // MAIN GAME LOOP
    while (gameWindow.isOpen()) {

        // EVENT HANDLING
        sf::Event event;
        while (gameWindow.pollEvent(event)) {

            // If user closes game window
            if (event.type == sf::Event::Closed) {
                gameWindow.close();
            }

            // If user clicks mouse
            if (event.type == sf::Event::MouseButtonPressed) {
                // Get mouse position
                sf::Vector2i mouse = sf::Mouse::getPosition(gameWindow);

                // If user clicks on leaderboard button
                if (leaderboardRect.getGlobalBounds().contains(gameWindow.mapPixelToCoords(mouse))) {
                    prevPaused = gamePaused;

                    if (!gamePaused) {
                        pauseTime = chrono::high_resolution_clock::now();
                        gamePaused = true;
                    }

                    // Draw all tiles as revealed
                    for (int row = 0; row < rowCount; row++) {
                        for (int col = 0; col < colCount; col++) {
                            tileRevealedSprite.setPosition(col * board.getTile(0, 0).getTileSize().x, row * board.getTile(0, 0).getTileSize().y);
                            gameWindow.draw(tileRevealedSprite);
                        }
                    }

                    gameWindow.display();
                    leaderboardOpen = loadLeaderboardWindow();

                    if (!leaderboardOpen && !prevPaused) {
                        chrono::high_resolution_clock::time_point unPausedTime = chrono::high_resolution_clock::now();
                        elapsedPausedTime += chrono::duration_cast<chrono::seconds>(unPausedTime - pauseTime).count();
                        gamePaused = false;
                    }
                }

                // If user clicks on happy face button
                if (happyFaceRect.getGlobalBounds().contains(gameWindow.mapPixelToCoords(mouse))) {

                    // Close current game window and open a new one
                    gameWindow.close();
                    loadGameWindow(username);
                }

                // If user clicks on debug button
                if (debugRect.getGlobalBounds().contains(gameWindow.mapPixelToCoords(mouse)) && !gamePaused && !gameOver && !board.checkWin()) {
                    debugOn = !debugOn;
                }

                // If user clicks on pause/play button
                if (pauseRect.getGlobalBounds().contains(gameWindow.mapPixelToCoords(mouse)) && !gameOver && !board.checkWin()) {
                    gamePaused = !gamePaused;

                    if (gamePaused) {
                        pauseTime = chrono::high_resolution_clock::now();
                    }

                    else {
                        chrono::high_resolution_clock::time_point unPausedTime = chrono::high_resolution_clock::now();
                        elapsedPausedTime += chrono::duration_cast<chrono::seconds>(unPausedTime - pauseTime).count();
                    }
                }

                // If user clicks on tile
                for (int row = 0; row < board.getRowCount(); row++) {
                    for (int col = 0; col < board.getColCount(); col++) {
                        Tile &tile = board.getTile(row, col);
                        sf::Sprite &tileSprite = tile.getTileHiddenSprite();

                        // Check bounds of tile and game conditions
                        if (tileSprite.getGlobalBounds().contains(gameWindow.mapPixelToCoords(mouse)) && !gameOver && !gamePaused && !board.checkWin()) {

                            // If right click, place flag on tile
                            if (event.mouseButton.button == sf::Mouse::Right && !gamePaused) {
                                tile.toggleFlag();

                                // Decrement flag count
                                if (tile.getIsFlagged()) {
                                    flagCount -= 1;
                                }

                                // Increment flag count
                                else if (!tile.getIsRevealed()){
                                    flagCount += 1;
                                }

                                // Update flag counter
                                loadCounter(gameWindow, digitsMap, flagCount);
                            }

                            // If left click, reveal tile
                            if (event.mouseButton.button == sf::Mouse::Left) {

                                // Get location of tile the mouse is on
                                int boardX = mouse.x / tile.getTileSize().x;
                                int boardY = mouse.y / tile.getTileSize().y;

                                // If mouse location is on the board and game is not over, reveal tiles
                                if (boardX >= 0 && boardX < board.getColCount() && boardY >= 0 && boardY < board.getRowCount() && !gameOver) {
                                    Tile &tile = board.getTile(boardY, boardX);
                                    board.revealTiles(boardY, boardX);

                                    // If user clicks on mine
                                    if (tile.getIsMine() && !tile.getIsFlagged()) {
                                        for (int col = 0; col < colCount; col++) {
                                            for (int row = 0; row < rowCount; row++) {
                                                Tile &tile = board.getTile(row, col);

                                                // Remove current flagged tiles
                                                if (tile.getIsFlagged()) {
                                                    tile.toggleFlag();
                                                }

                                                // Reveal all mines
                                                if (tile.getIsMine()) {
                                                    tile.revealTile();
                                                }
                                            }
                                        }

                                        gameOver = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Set digits for flag counter
        int hundreds = (abs(flagCount) / 100) % 10;
        int tens = (abs(flagCount) / 10) % 10;
        int ones = abs(flagCount) % 10;

        // Calculate elapsed time
        auto gameDuration = chrono::duration_cast<chrono::seconds>(chrono::high_resolution_clock::now() - startTime);
        int totalTime = gameDuration.count();

        // Continue timer if game is not paused, lost, or won
        if(!gamePaused && !gameOver && !board.checkWin()) {
            totalTime =  totalTime - elapsedPausedTime;
            minutes = totalTime / 60;
            seconds = totalTime % 60;
        }

        // Separate time by index
        int minutes0 = minutes / 10 % 10; // minutes index 0
        int minutes1 = minutes % 10; // minutes index 1
        int seconds0 = seconds / 10 % 10; // seconds index 0
        int seconds1 = seconds % 10; // seconds index 1

        gameWindow.clear(sf::Color::White);

        // Set position and draw timer
        loadCounter(gameWindow, digitsMap, flagCount);

        digitsMap[minutes0].setPosition((colCount * 32) - 97, 32 * (rowCount + 0.5) + 16);
        gameWindow.draw(digitsMap[minutes0]);

        digitsMap[minutes1].setPosition((colCount * 32) - 76,  32 * (rowCount + 0.5) + 16);
        gameWindow.draw(digitsMap[minutes1]);

        digitsMap[seconds0].setPosition((colCount * 32) - 54,  32 * (rowCount + 0.5) + 16);
        gameWindow.draw(digitsMap[seconds0]);

        digitsMap[seconds1].setPosition((colCount * 32) - 33,  32 * (rowCount + 0.5) + 16);
        gameWindow.draw(digitsMap[seconds1]);

        // Draw board rects/sprites
        gameWindow.draw(happyFaceRect);
        gameWindow.draw(leaderboardRect);
        gameWindow.draw(happyFaceSprite);
        gameWindow.draw(debugSprite);
        gameWindow.draw(pauseSprite);
        gameWindow.draw(leaderboardSprite);
        board.drawBoard(gameWindow);

        // If game is won
        if (board.checkWin() && !scoreWritten) {

            // Place flags on all mines that are not flagged
            for (int row = 0; row < rowCount; row++) {
                for (int col = 0; col < colCount; col++) {
                    Tile &tile = board.getTile(row, col);

                    // If tile is a mine and is not flagged, flag it
                    if (tile.getIsMine() && !tile.getIsFlagged()) {

                        sf::Sprite flagSprite = tile.getFlagSprite();
                        flagSprite.setPosition(col * board.getTile(0, 0).getTileSize().x, row * board.getTile(0, 0).getTileSize().y);
                        gameWindow.draw(flagSprite);

                        tile.toggleFlag();
                        flagCount -= 1;
                    }
                }
            }

            // Update winFace and draw counter
            gameWindow.draw(winFaceSprite);
            happyFaceSprite = winFaceSprite;
            loadCounter(gameWindow,digitsMap, flagCount);
            gameWindow.display();

            // Calculate final time
            auto endTime = chrono::high_resolution_clock::now();
            auto playTime = chrono::duration_cast<chrono::seconds>(endTime - startTime - chrono::seconds(elapsedPausedTime));
            int finalTime = playTime.count();

            // Write score to leaderboard file
            board.writeScore(highScores, username, finalTime);
            scoreWritten = true;

            // Load the leaderboard window after drawing flags
            loadLeaderboardWindow();
        }

        // If game lost, draw loseFace and reset counter
        if (gameOver) {
            gameWindow.draw(loseFaceSprite);
            flagCount = board.getMineCount();
        }

        // If debug is on, reveal mines
        if (debugOn) {
            for (int row = 0; row < rowCount; row++) {
                for (int col = 0; col < colCount; col++) {
                    Tile & tile = board.getTile(row, col);

                    // Reveal position of all mines
                    if (tile.getIsMine()) {
                        mineSprite.setPosition(col * board.getTile(0, 0).getTileSize().x, row * board.getTile(0, 0).getTileSize().y);
                        tileHiddenSprite.setPosition(col * board.getTile(0, 0).getTileSize().x, row * board.getTile(0, 0).getTileSize().y);
                        gameWindow.draw(tileHiddenSprite);
                        gameWindow.draw(mineSprite);
                    }
                }
            }
        }

        // If game paused
        if (gamePaused && !leaderboardOpen) {
            for (int row = 0; row < rowCount; row++) {
                for (int col = 0; col < colCount; col++) {

                    // Hide current game board
                    tileRevealedSprite.setPosition(col * board.getTile(0, 0).getTileSize().x, row * board.getTile(0, 0).getTileSize().y);
                    gameWindow.draw(tileRevealedSprite);
                }
            }

            // Change pauseSprite to playSprite
            gameWindow.draw(playSprite);
        }

        gameWindow.display();
    }
}

void loadWelcomeWindow() {
    Board board("../files/config.cfg");
    int windowWidth = board.getColCount() * 32;
    int windowHeight = (board.getRowCount() * 32) + 100;
    bool gameStart = false;
    string username;

    sf::RenderWindow welcomeWindow(sf::VideoMode(windowWidth, windowHeight), "Minesweeper");
    sf::Font font = loadFont();

    // Welcome to Minesweeper!
    sf::Text welcomeText;
    welcomeText.setString("WELCOME TO MINESWEEPER!");
    welcomeText.setFont(font);
    welcomeText.setCharacterSize(24);
    welcomeText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    welcomeText.setFillColor(sf::Color::White);

    setText(welcomeText, windowWidth / 2.0f, (windowHeight / 2.0f) - 150);

    // Enter Your Name:
    sf::Text nameText;
    nameText.setString("Enter your name:");
    nameText.setFont(font);
    nameText.setCharacterSize(20);
    nameText.setStyle(sf::Text::Bold);
    nameText.setFillColor(sf::Color::White);

    setText(nameText, windowWidth / 2.0f, (windowHeight / 2.0f) - 75);

    // User Name Input
    sf::Text userNameText;
    userNameText.setFont(font);
    userNameText.setCharacterSize(18);
    userNameText.setStyle(sf::Text::Bold);
    userNameText.setFillColor(sf::Color::Yellow);
    userNameText.setPosition(sf::Vector2f(windowWidth / 2.0f, (windowHeight / 2.0f) - 45));

    while (welcomeWindow.isOpen()) {

        // EVENT HANDLING
        sf::Event event;
        while (welcomeWindow.pollEvent(event)) {

            // If user closes welcome window
            if (event.type == sf::Event::Closed) {
                welcomeWindow.close();
            }

            if (event.type == sf::Event::KeyPressed) {

                // If user clicks backspace
                if (event.key.code == sf::Keyboard::Backspace && !username.empty()) {

                    // Remove last character of username
                    username.pop_back();
                }

                // If user clicks enter while username is not empty
                if (event.key.code == sf::Keyboard::Enter && !username.empty()) {

                    // Close welcome window and start game
                    gameStart = true;
                    welcomeWindow.close();
                }
            }

            // If user enters text
            if (event.type == sf::Event::TextEntered) {

                // Limit size of username to 10 characters
                if (username.size() < 10) {
                    char nameChar  = static_cast<char>(event.text.unicode);

                    // Limit entry to alphabetical characters
                    if (isalpha(nameChar)) {
                        username += nameChar;

                        // First char = uppercase
                        username[0] = toupper(username[0]);

                        // Other chars = lowercase
                        for (int i = 1; i < username.size(); i++) {
                            username[i] = tolower(username[i]);
                        }
                    }
                }
            }
        }

        // Draw text/rects
        welcomeWindow.clear(sf::Color::Blue);
        welcomeWindow.draw(welcomeText);
        welcomeWindow.draw(nameText);
        userNameText.setString(username + "|");
        sf::FloatRect userNameRect = userNameText.getLocalBounds();
        userNameText.setOrigin(userNameRect.left + userNameRect.width / 2.0f, userNameRect.top + userNameRect.height / 2.0f);
        welcomeWindow.draw(userNameText);

        welcomeWindow.display();
    }

    // Load game window with specified username
    if (gameStart) {
        loadGameWindow(username);
    }
}


int main() {
    loadWelcomeWindow();
    return 0;
}