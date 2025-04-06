#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()
#include "Tronco.h"

enum TroncoAnims
{
    STATIC
};

Tronco::Tronco()
{
    sprite = nullptr;
    map = nullptr;
    size = glm::ivec2(32, 16);
    amplitude = 0;
    speed = MOVEMENT_SPEED;
    time = 0.0f;
    verticalDisplacement = 0.0f;
}

Tronco::~Tronco()
{
    if (sprite != nullptr) {
        delete sprite;
    }
}

void Tronco::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    tileMapDispl = glm::ivec2(0, 0);
    position = glm::vec2(tileMapPos);
    initialY = position.y;
    currentY = initialY;

    float amplitudeVariation = ((rand() % 11) - 5) / 10.0f * (0.5f * BASE_MOVEMENT_AMPLITUDE);
    amplitude = BASE_MOVEMENT_AMPLITUDE + amplitudeVariation;

    time = (rand() % 1000) / 100.0f;

    spritesheet.loadFromFile("images/Enemigos2.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(32, 16), glm::vec2(0.25f, 0.25f), &spritesheet, &shaderProgram);

    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(STATIC, 1);
    sprite->addKeyframe(STATIC, glm::vec2(0.0f, 0.25));
    sprite->changeAnimation(STATIC);

    sprite->setPosition(glm::vec2(position.x, position.y));
}

void Tronco::update(int deltaTime)
{
    float deltaTimeSeconds = deltaTime / 1000.0f;

    sprite->update(deltaTime);

    float previousY = currentY;

    time += deltaTimeSeconds;

    currentY = initialY + amplitude * sin(speed * time);

    verticalDisplacement = currentY - previousY;

    position.y = currentY;
    sprite->setPosition(glm::vec2(position.x, position.y));
}

void Tronco::render(glm::mat4 modelview)
{
    sprite->render(modelview);
}

void Tronco::setTileMap(TileMap* tileMap)
{
    map = tileMap;
}

glm::vec2 Tronco::getPosition() const
{
    return position;
}

bool Tronco::isPlayerOnTop(const glm::vec2& playerPos, const glm::ivec2& playerSize) const
{
    float playerFeetY = playerPos.y + playerSize.y;
    float troncoTopY = position.y;

    bool isVerticallyAligned = (playerFeetY >= troncoTopY - 4.0f && playerFeetY <= troncoTopY + 4.0f);

    bool isHorizontallyAligned = (
        (playerPos.x + playerSize.x >= position.x) &&
        (playerPos.x <= position.x + size.x)
        );

    return isVerticallyAligned && isHorizontallyAligned;
}
