#include "Bisonte.h"
#include "Player.h" 
#include "TileMap.h" 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Bisonte::Bisonte(ShaderProgram* shader)
    : Item(ItemType::BISONTE, shader), onGround(false)
{
}

Bisonte::~Bisonte() {
}

void Bisonte::init(const glm::vec2& pos) {
    posItem = pos;
    onGround = false;

    //cargamos el sprite con protección de errores del spritesheet
    if (!spritesheet.loadFromFile("images/Items1.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error: Sprite::no ha cargado correctament el sprite sheet" << std::endl;
        markForRemoval();
        return;
    }

    //usamos el sprite sheet Items1.png que es 32x16, 2 iconos de 16x16 y cogemos el primer item
    sprite = Sprite::createSprite(glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT), glm::vec2(0.5f, 1.0f), &spritesheet, shaderProg);

    if (sprite == nullptr) {
        std::cout << "Error: Sprite::createSprite devolvió nullptr para Bisonte!" << std::endl;
        markForRemoval();
        return;
    }

    //definimos solo 1 animación ya que es "estatico"
    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.0f, 0.0f));
    sprite->changeAnimation(0);
    sprite->setAnimationSpeed(0, 100);

    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
    active = true;
}

void Bisonte::update(int deltaTime) {
    if (needsRemoval || !active) return;

    // Aplicamos gravedad para que no se queden flotando en el sitio de spawn
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

void Bisonte::render(glm::mat4& modelview) {
    if (active && !needsRemoval && sprite != nullptr) {
        sprite->render(modelview);
    }
}

void Bisonte::applyEffect(Player* player) {
    //La piel de bisonte (o piel de Ciervo si seguimos los nombres del juego) hace que el jugador sea invulnerable durante 5 segundos
    if (active && player) {
        player->becomeInvulnerable(5.0f);
        markForRemoval();
    }
}

glm::ivec2 Bisonte::getHitboxSize() const {
    return glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT);
}
