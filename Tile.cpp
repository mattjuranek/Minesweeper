#include "Tile.h"


Tile::Tile() {
    isRevealed = false;
    isMine = false;
    isFlagged = false;
    adjacentMines = 0;

    sf::Texture& tileHiddenTexture = TextureManager::getTexture("tile_hidden");
    tileHiddenSprite.setTexture(tileHiddenTexture);
    tileTextureSize = tileHiddenTexture.getSize();

    sf::Texture& tileRevealedTexture = TextureManager::getTexture("tile_revealed");
    tileRevealedSprite.setTexture(tileRevealedTexture);

    sf::Texture& mineTexture = TextureManager::getTexture("mine");
    mineSprite.setTexture(mineTexture);

    sf::Texture& flagTexture = TextureManager::getTexture("flag");
    flagSprite.setTexture(flagTexture);
}

void Tile::revealTile() {
    if (!isRevealed) {
        isRevealed = true;

        // Set revealed texture
        sf::Texture& tileRevealedTexture = TextureManager::getTexture("tile_revealed");
        tileRevealedSprite.setTexture(tileRevealedTexture);

        // If tile is a mine
        if (isMine) {

            // Set mine texture
            sf::Texture& mineTexture = TextureManager::getTexture("mine");
            mineSprite.setTexture(mineTexture);
        }

        // If tile has adjacent mines
        else if (adjacentMines > 0) {

            // Set corresponding number texture
            sf::Texture& numberTexture = TextureManager::getTexture("number_" + to_string(adjacentMines));
            numberSprite.setTexture(numberTexture);
        }
    }
}

void Tile::toggleFlag() {
    // If not revealed, toggle flag
    if (!isRevealed) {
        isFlagged = !isFlagged;
    }
}

void Tile::setMine() {
    isMine = true;
}

void Tile::setAdjacentMines(int value) {
    adjacentMines = value;
}

sf::Sprite& Tile::getTileHiddenSprite() {
    return tileHiddenSprite;
}

sf::Sprite& Tile::getTileRevealedSprite() {
    return tileRevealedSprite;
}

sf::Sprite& Tile::getMineSprite() {
    return mineSprite;
}

sf::Sprite& Tile::getFlagSprite() {
    return flagSprite;
}

sf::Sprite& Tile::getNumberSprite() {
    return numberSprite;
}

bool Tile::getIsRevealed() const {
    return isRevealed;
}

bool Tile::getIsMine() const {
    return isMine;
}

bool Tile::getIsFlagged() const {
    return isFlagged;
}

int Tile::getAdjacentMines() const {
    return adjacentMines;
}

sf::Vector2u Tile::getTileSize() const {
    return tileTextureSize;
}