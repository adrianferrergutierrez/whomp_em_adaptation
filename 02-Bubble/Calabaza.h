#pragma once
#ifndef _CALABAZA_INCLUDE
#define _CALABAZA_INCLUDE

#include "Item.h"
#include "Texture.h"

class Calabaza : public Item {
public:
    Calabaza(ShaderProgram* shader);
    ~Calabaza() override;

    void init(const glm::vec2& pos) override;
    void update(int deltaTime) override;
    void render(glm::mat4& modelview) override;
    void applyEffect(Player* player) override;
    glm::ivec2 getHitboxSize() const override;

private:
    Texture spritesheet;
    static const int ITEM_WIDTH = 8;
    static const int ITEM_HEIGHT = 16;

    // Físicas
    bool onGround;
    static const int FALL_STEP = 3;
};

#endif 
