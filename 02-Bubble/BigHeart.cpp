#include "BigHeart.h"
#include "Player.h" // Para applyEffect
#include "TileMap.h" // Necesario para la colisión
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

BigHeart::BigHeart(ShaderProgram* shader) :Item(ItemType::BIG_HEART, shader), onGround(false) {}

BigHeart::~BigHeart() {}

void BigHeart::init(const glm::vec2& pos) {

    posItem = pos;
    onGround = false;

    //cargamos el sprite con protección a errores
    if (!spritesheet.loadFromFile("images/Corazones2.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando images/Corazones2.png para BigHeart!" << std::endl;
        markForRemoval();
        return;
    }

    //En el spritesheet Corazones2 hemos de coger el primer elemento de la fila 0 (solo tiene 1 fila)
    sprite = Sprite::createSprite(glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT), glm::vec2(1.0f / 3.0f, 1.0f), &spritesheet, shaderProg);

    if (sprite == nullptr) {
        std::cout << "Error: Sprite::createSprite devolvió nullptr para BigHeart!" << std::endl;
        markForRemoval();
        return;
    }

    //Solo tenemos una animación ya que es un item "estatico"
    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.0f, 0.0f));
    sprite->changeAnimation(0);
    sprite->setAnimationSpeed(0, 100);
    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));

    //lo hacemos activo, esto esta para el small heart que tiene un delay al salir
    active = true;
}

void BigHeart::update(int deltaTime) {
    if (needsRemoval || !active) return;

    //hacemos que caiga el corazon al suelo, que no se quede flotando en el aire
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

void BigHeart::render(glm::mat4& modelview) {
    if (active && !needsRemoval && sprite != nullptr) {
        sprite->render(modelview);
    }
}

void BigHeart::applyEffect(Player* player) {

    //si el jugador lo toca, curaremos hasta max_health del jugador
    if (active && player) {
        player->healToMax();
        markForRemoval();
    }
}

glm::ivec2 BigHeart::getHitboxSize() const {
    return glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT);
}
