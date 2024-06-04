#include <SFML/Graphics.hpp>
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <chrono>
using namespace std;


class Tile {
private:
    sf::Sprite tileHiddenSprite;
    sf::Sprite tileRevealedSprite;
    sf::Sprite mineSprite;
    sf::Sprite flagSprite;
    sf::Sprite numberSprite;
    sf::Vector2u tileTextureSize;
    bool isRevealed;
    bool isMine;
    bool isFlagged;
    int adjacentMines;
public:
    Tile();
    void revealTile();
    void toggleFlag();
    void setMine();
    void setAdjacentMines(int value);
    sf::Sprite& getTileHiddenSprite();
    sf::Sprite& getTileRevealedSprite();
    sf::Sprite& getMineSprite();
    sf::Sprite& getFlagSprite();
    sf::Sprite& getNumberSprite();
    bool getIsRevealed() const;
    bool getIsMine() const;
    bool getIsFlagged() const;
    int getAdjacentMines() const;
    sf::Vector2u getTileSize() const;
};

