#pragma once
#include "Sprite.h"
#include "TileMap.h"

class FireStickProjectile {
public:
    FireStickProjectile();
    ~FireStickProjectile();

    void init(const glm::ivec2& pos, ShaderProgram& shaderProgram, int direction);
    void update(int deltaTime);
    void render(glm::mat4 modelview);

    void setTileMap(TileMap* tileMap);
    void setPosition(const glm::vec2& pos);
    glm::vec2 getPosition() const;
    bool isActive() const;
    void deactivate();

    glm::ivec2 getSize() const;
    glm::ivec2 getHitboxSize() const;

private:
    Texture texture;
    Sprite* sprite;
    glm::ivec2 posProj;
    glm::ivec2 size;
    int direction;
    float speed;
    TileMap* map;
    bool active;
};


