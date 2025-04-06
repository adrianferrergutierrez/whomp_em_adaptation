#include <GL/glew.h>
#include "Enemy.h"

Enemy::Enemy()
{
    health = Health(100);
    damage = 10;
    falling = true;
    map = nullptr;
    sprite = nullptr;
    hitboxSize = glm::ivec2(32, 32); // Tamaño por defecto
}

Enemy::~Enemy()
{
    if (sprite != nullptr) {
        delete sprite;
    }
}

void Enemy::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    tileMapDispl = glm::ivec2(0, 0);
    posEnemy = tileMapPos;
}

void Enemy::update(int deltaTime)
{
    if (sprite != nullptr) {
        sprite->update(deltaTime);
    }

    health.update(static_cast<float>(deltaTime) / 1000.0f);
}

void Enemy::render(glm::mat4 modelview)
{
    if (sprite != nullptr) {
        sprite->render(modelview);
    }
}

glm::vec2 Enemy::getPosition() const
{

    return glm::vec2(float(tileMapDispl.x + posEnemy.x), float(tileMapDispl.y + posEnemy.y));
}

glm::vec2 Enemy::getHitboxPosition() const
{
    return glm::vec2(float(tileMapDispl.x + posEnemy.x + (32 - hitboxSize.x) / 2),
        float(tileMapDispl.y + posEnemy.y));
}

glm::ivec2 Enemy::getHitboxSize() const
{
    return hitboxSize;
}

void Enemy::setTileMap(TileMap* tileMap)
{
    map = tileMap;
}

void Enemy::setPosition(const glm::vec2& pos)
{
    posEnemy = glm::ivec2(pos.x, pos.y);
    if (sprite != nullptr) {
        sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
    }
}

void Enemy::takeDamage(int dmg)
{
    health.takeDamage(dmg);
}