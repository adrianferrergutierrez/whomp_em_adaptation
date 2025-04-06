#pragma once
#ifndef _SMALLHEART_INCLUDE
#define _SMALLHEART_INCLUDE

#include "Item.h"
#include "Texture.h" // Necesaria para cargar la textura

class SmallHeart : public Item {
public:
    SmallHeart(ShaderProgram* shader); // Pasar shader al constructor base
    ~SmallHeart() override; // Override virtual destructor

    // Implementación de los métodos virtuales de Item
    void init(const glm::vec2& pos) override;
    void update(int deltaTime) override;
    void render(glm::mat4& modelview) override;
    void applyEffect(Player* player) override;
    glm::ivec2 getHitboxSize() const override;

private:
    Texture spritesheet; // Textura específica para el corazón
    float upwardSpeed;   // Velocidad de ascenso
    static const int ITEM_WIDTH = 10;
    static const int ITEM_HEIGHT = 10;


};

#endif // _SMALLHEART_INCLUDE
