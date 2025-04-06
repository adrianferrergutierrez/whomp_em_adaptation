#pragma once
#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include "Sprite.h"
#include "TileMap.h"
#include "Health.h"

// Clase base para todos los enemigos del juego
class Enemy
{
public:
    Enemy();
    virtual ~Enemy();

    virtual void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
    virtual void update(int deltaTime);
    virtual void render(glm::mat4 modelview);

    // Métodos para posición y colisiones
    glm::vec2 getPosition() const;
    virtual glm::vec2 getHitboxPosition() const;
    virtual glm::ivec2 getHitboxSize() const;

    void setTileMap(TileMap* tileMap);
    void setPosition(const glm::vec2& pos);

    // Propiedades básicas de un enemigo
    int getDamage() const { return damage; }
    bool isAlive() const { return health.isAlive(); }

    // Métodos para recibir daño
    virtual void takeDamage(int damage);

protected:
    // Atributos comunes para todos los enemigos
    Health health;
    glm::ivec2 tileMapDispl, posEnemy;
    glm::ivec2 hitboxSize;
    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;
    int damage;
    bool falling;
};

#endif 