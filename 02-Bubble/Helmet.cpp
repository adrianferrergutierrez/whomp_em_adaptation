#include "Helmet.h"
#include "Player.h" 
#include "TileMap.h" 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Helmet::Helmet(ShaderProgram* shader)
    : Item(ItemType::HELMET, shader), onGround(false)
{
}

Helmet::~Helmet() {
}

void Helmet::init(const glm::vec2& pos) {
    posItem = pos;
    onGround = false;

    //cargamos el sprite con protección a errores
    if (!spritesheet.loadFromFile("images/Items1.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando images/Items1.png para Helmet!" << std::endl;
        markForRemoval();
        return;
    }

    // Cargamos el sprite del spritesheet de Items1 donde el casco es el segundo item de los 2 que hay que son 16x16
    sprite = Sprite::createSprite(glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT), glm::vec2(0.5f, 1.0f), &spritesheet, shaderProg);

    if (sprite == nullptr) {
        std::cout << "Error: Sprite::createSprite devolvió nullptr para Helmet!" << std::endl;
        markForRemoval();
        return;
    }

    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.5f, 0.0f));
    sprite->changeAnimation(0);
    sprite->setAnimationSpeed(0, 100);

    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
    active = true;
}

void Helmet::update(int deltaTime) {
    if (needsRemoval || !active) return;

    // Aplicamos gravedad como a todos los items(menos el corazon pequeño)
    if (!onGround) {
        posItem.y += FALL_STEP;
        glm::ivec2 currentPosInt = glm::ivec2(posItem.x, posItem.y);
        int correctedY = currentPosInt.y;
        if (map != nullptr && map->collisionMoveDown(currentPosInt, getHitboxSize(), &correctedY)) {
            posItem.y = float(correctedY);
            onGround = true;
        }
    }

    if (sprite != nullptr) {
        sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
        sprite->update(deltaTime);
    }
}

void Helmet::render(glm::mat4& modelview) {
    if (active && !needsRemoval && sprite != nullptr) {
        sprite->render(modelview);
    }
}

void Helmet::applyEffect(Player* player) {
    //Hacemos que el player se equipe el casco, que le reducirá el daño a la mitad durante 4 golpes
    if (active && player) {
        player->equipHelmet();
        markForRemoval();
    }
}

glm::ivec2 Helmet::getHitboxSize() const {
    return glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT);
}
