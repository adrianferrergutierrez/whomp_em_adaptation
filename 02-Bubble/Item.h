#pragma once
#ifndef _ITEM_INCLUDE
#define _ITEM_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "ShaderProgram.h"
#include "Player.h" 
#include "TileMap.h" 

enum class ItemType {
    SMALL_HEART,
    BIG_HEART,
    HELMET,
    BISONTE,
    CALABAZA,
};

class Item {
public:
    Item(ItemType type, ShaderProgram* shader);
    virtual ~Item();
    //Funciones virtuales para que todos los items le hagan override
    virtual void init(const glm::vec2& pos) = 0;
    virtual void update(int deltaTime) = 0;
    virtual void render(glm::mat4& modelview) = 0;
    virtual void applyEffect(Player* player) = 0;

    // Métodos comunes
    void setPosition(const glm::vec2& pos);
    const glm::vec2& getPosition() const;
    ItemType getType() const;
    virtual glm::vec2 getHitboxPosition() const;
    virtual glm::ivec2 getHitboxSize() const = 0;

    void setTileMap(TileMap* tileMap);

    bool shouldBeRemoved() const;
    void markForRemoval();
    bool isActive() const;

protected:
    glm::vec2 posItem;
    Sprite* sprite;
    ShaderProgram* shaderProg;
    ItemType itemType;
    TileMap* map;
    bool needsRemoval;

    float spawnTimer;
    bool active;

};

#endif // _ITEM_INCLUDE
