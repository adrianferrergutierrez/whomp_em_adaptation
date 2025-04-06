#pragma once
#ifndef _BISONTE_INCLUDE
#define _BISONTE_INCLUDE

#include "Item.h"
#include "Texture.h"

class Bisonte : public Item {
public:
    Bisonte(ShaderProgram* shader);
    ~Bisonte() override;

    void init(const glm::vec2& pos) override;
    void update(int deltaTime) override;
    void render(glm::mat4& modelview) override;
    void applyEffect(Player* player) override;
    glm::ivec2 getHitboxSize() const override;

private:
    Texture spritesheet;
    static const int ITEM_WIDTH = 16;
    static const int ITEM_HEIGHT = 16;

    // Físicas
    bool onGround;
    static const int FALL_STEP = 3;
};

#endif 
