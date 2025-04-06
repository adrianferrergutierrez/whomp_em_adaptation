#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Snake.h"
#include "Player.h"

enum SnakeAnims
{
    MOVE_LEFT, MOVE_RIGHT, FALL_LEFT, FALL_RIGHT
};

Snake::Snake() : Enemy()
{
    direction = -1;
    moveSpeed = 1.2f;
    onGround = false;
    targetPlayer = nullptr;
}

Snake::~Snake()
{
    // La limpieza del sprite se realiza en el destructor de Enemy
}

void Snake::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* player)
{
    // Primero llamamos al m�todo init de la clase base
    Enemy::init(tileMapPos, shaderProgram);
    health = Health(10);
    damage = 10;
    //hacemos override de estos atributos de la clase enemy

    // Configurar aspectos espec�ficos de la serpiente
    spritesheet.loadFromFile("images/Enemigos2.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.25), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(4); // Ahora tenemos 4 animaciones

    // Animaciones de movimiento horizontal
    sprite->setAnimationSpeed(MOVE_LEFT, 15);
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.f, 0.5f));
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.25f, 0.5f));

    sprite->setAnimationSpeed(MOVE_RIGHT, 15);
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5f, 0.75f));
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75f, 0.75f));

    // Animaciones de ca�da
    sprite->setAnimationSpeed(FALL_LEFT, 15);
    sprite->addKeyframe(FALL_LEFT, glm::vec2(0.5f, 0.5f));
    sprite->addKeyframe(FALL_LEFT, glm::vec2(0.75f, 0.5f));

    sprite->setAnimationSpeed(FALL_RIGHT, 15);
    sprite->addKeyframe(FALL_RIGHT, glm::vec2(0.f, 0.75f));
    sprite->addKeyframe(FALL_RIGHT, glm::vec2(0.25f, 0.75f));

    // Determinar direcci�n inicial basada en la posici�n del jugador
    targetPlayer = player;
    if (player != nullptr) {
        glm::vec2 playerPos = player->getPosition();
        // Si el jugador est� a la izquierda de la serpiente, la serpiente debe mirar a la izquierda
        direction = (playerPos.x < tileMapPos.x) ? -1 : 1;
    }
    else {
        direction = -1; // Valor por defecto si no hay jugador
    }

    // Iniciar con la animaci�n correcta basada en la direcci�n inicial
    sprite->changeAnimation(direction == -1 ? FALL_LEFT : FALL_RIGHT);
    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

void Snake::update(int deltaTime)
{
    // Primero llamamos al m�todo update de la clase base
    Enemy::update(deltaTime);

    // Convertir deltaTime a segundos para movimiento consistente
    float deltaTimeSeconds = deltaTime / 1000.0f;
    if (deltaTimeSeconds < 0.01f) deltaTimeSeconds = 0.016f; // ~60 FPS

    // Comprobamos si est� cayendo
    if (falling) {

        // Aplicamos gravedad
        posEnemy.y += 4.0f;

        // Comprobar colisi�n con el suelo
        if (map != nullptr && map->collisionMoveDown(posEnemy, hitboxSize, &posEnemy.y)) {
            falling = false;
            onGround = true;

            // Cambiar a animaci�n de movimiento seg�n direcci�n
            sprite->changeAnimation(direction == -1 ? MOVE_LEFT : MOVE_RIGHT);
        }
    }
    else if (onGround) {
        // Movimiento horizontal solo si est� en el suelo
        float movement = moveSpeed * deltaTimeSeconds * 60.0f;
        posEnemy.x += movement * direction;

        // Asegurarnos de que la animaci�n coincida con la direcci�n
        if (direction == -1) {
            if (sprite->animation() != MOVE_LEFT)
                sprite->changeAnimation(MOVE_LEFT);

            // Verificar colisi�n con el mapa
            if (map != nullptr && map->collisionMoveLeft(posEnemy, hitboxSize)) {
                posEnemy.x += movement * 2; // Corregir sobrepenetraci�n
                direction = 1;
                sprite->changeAnimation(MOVE_RIGHT);
            }
        }
        else { // direction == 1
            if (sprite->animation() != MOVE_RIGHT)
                sprite->changeAnimation(MOVE_RIGHT);

            // Verificar colisi�n con el mapa
            if (map != nullptr && map->collisionMoveRight(posEnemy, hitboxSize)) {
                posEnemy.x -= movement * 2; // Corregir sobrepenetraci�n
                direction = -1;
                sprite->changeAnimation(MOVE_LEFT);
            }
        }
    }

    // Actualizar posici�n del sprite
    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}


