#include "Calabaza.h"
#include "Player.h" 
#include "TileMap.h" 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Calabaza::Calabaza(ShaderProgram* shader)
    : Item(ItemType::CALABAZA, shader), onGround(false)
{
}

Calabaza::~Calabaza() {
}

void Calabaza::init(const glm::vec2& pos) {

    posItem = pos;
    onGround = false;

    //cargamos el sprite con protección a errores
    if (!spritesheet.loadFromFile("images/Items2.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando images/Items2.png para Calabaza!" << std::endl;
        markForRemoval();
        return;
    }
    //cargamos el sprite del sprite sheet de Items2 donde tenemos 3 elementos de 16x16, cogemos el primero
    sprite = Sprite::createSprite(glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT), glm::vec2(1.0f / 3.0f, 1.0f), &spritesheet, shaderProg);

    if (sprite == nullptr) {
        std::cout << "Error: Sprite::createSprite devolvió nullptr para Calabaza!" << std::endl;
        markForRemoval();
        return;
    }

    //le ponemos 1 animación ya es "estatico"
    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.0f, 0.0f));
    sprite->changeAnimation(0);
    sprite->setAnimationSpeed(0, 100);

    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
    active = true;
}

void Calabaza::update(int deltaTime) {
    if (needsRemoval || !active) return;

    // aplicamos la gravedad
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

void Calabaza::render(glm::mat4& modelview) {
    if (active && !needsRemoval && sprite != nullptr) {
        sprite->render(modelview);
    }
}

void Calabaza::applyEffect(Player* player) {
    //si el jugador la coge, llamamos a collectPumpkin() y la eliminamos, donde el collectPumpking suma en 1 la cantidad de calabazas del player
    if (active && player) {
        player->collectPumpkin(); // Usar el método existente
        markForRemoval();
    }
}

glm::ivec2 Calabaza::getHitboxSize() const {
    return glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT);
}
