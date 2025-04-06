#define _USE_MATH_DEFINES // Para M_PI si se usa cmath directamente, aunque preferimos glm::pi
#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/constants.hpp> // Para glm::pi()
#include <glm/common.hpp>       // Para glm::clamp()
#include "Rana.h"
#include "Game.h" 
// Constante local para gravedad 
const int FALL_STEP = 4;
#define TILESIZE 16

Rana::Rana() : Enemy()
{
    // Constructor vacío, inicialización en init
}

Rana::~Rana()
{
    // El sprite se libera en el destructor de Enemy
}

// --- init con lógica de salto inicial ---
void Rana::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* playerInstance)
{
    Enemy::init(tileMapPos, shaderProgram);
    this->player = playerInstance;

    // Atributos específicos de Rana
    health = Health(20);
    damage = 10; //un corazon son 30, asi que hace 1/3
    hitboxSize = glm::ivec2(26, 32); // Tamaño 32x32 de esa versión

    // Cargar spritesheet y crear sprite (32x32, 4 frames)
    spritesheet.loadFromFile("images/Enemigos.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(26, 32), glm::vec2(0.12f, 0.24f), &spritesheet, &shaderProgram);

    // Aumentar número de animaciones
    sprite->setNumberAnimations(4); // IDLE_LEFT, IDLE_RIGHT, JUMP_LEFT, JUMP_RIGHT, donde IDLE es estar en el suelo

    // Animación IDLE_LEFT 
    sprite->setAnimationSpeed(IDLE_LEFT, 8);
    sprite->addKeyframe(IDLE_LEFT, glm::vec2(0.0f, 0.24f));

    // Animación IDLE_RIGHT
    sprite->setAnimationSpeed(IDLE_RIGHT, 8);
    sprite->addKeyframe(IDLE_RIGHT, glm::vec2(0.48f, 0.0f));

    // JUMP_LEFT (4 frames)
    sprite->setAnimationSpeed(JUMP_LEFT, 4);
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.0f, 0.24f));
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.10f, 0.24f));
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.20f, 0.24f));
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.30f, 0.24f));

    // JUMP_RIGHT (4 frames) 
    sprite->setAnimationSpeed(JUMP_RIGHT, 4);
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.48f, 0.0f));
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.36f, 0.0f));
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.24f, 0.0f));
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.12f, 0.0f));

    // Inicializar variables de estado del salto
    isJumping = false;

    //Las ranas tienen un cooldown de salto, por eso el timer, ademas podemos modificar que tan lejos/alto llegan
    jumpTimer = 0.0f;
    jumpMaxTime = JUMP_DURATION_SECONDS;
    jumpHeight = JUMP_HEIGHT_PIXELS;
    jumpSpeedX = (JUMP_DISTANCE_TILES * TILESIZE) / JUMP_DURATION_SECONDS;
    jumpDirection = 1;
    timeSinceLastJump = JUMP_COOLDOWN_SECONDS;

    // Empezar en estado IDLE
    sprite->changeAnimation(IDLE_RIGHT);
}


void Rana::update(int deltaTime)
{
    Enemy::update(deltaTime);
    if (!isAlive()) return;

    float dtSeconds = deltaTime / 1000.0f;

    if (isJumping)
    {
        jumpTimer += dtSeconds;
        float jumpProgress = glm::clamp(jumpTimer / jumpMaxTime, 0.0f, 1.0f);
        float currentHeight = jumpHeight * sin(jumpProgress * glm::pi<float>());
        float targetX = jumpStartPosition.x + (jumpSpeedX * jumpTimer * jumpDirection);
        float targetY = jumpStartPosition.y - currentHeight;
        glm::vec2 movementDelta = glm::vec2(targetX - posEnemy.x, targetY - posEnemy.y);

        // --- Movimiento Horizontal y Colisión (Incremental para evitar tunneling) ---
        if (abs(movementDelta.x) > 0.01f) { // Solo si hay movimiento horizontal significativo
            float remainingDeltaX = movementDelta.x;
            // Definir un tamaño máximo de paso (ej: la mitad del ancho de la hitbox o del tilesize)
            const float maxStepX = hitboxSize.x / 2.0f;
            int safetyBreak = 100; // Prevenir bucles infinitos

            while (abs(remainingDeltaX) > 0.01f && safetyBreak > 0) {
                float currentStep = glm::clamp(remainingDeltaX, -maxStepX, maxStepX);

                // Guardar posición antes del paso
                glm::vec2 originalPos = posEnemy;
                // Aplicar el paso actual tentativo
                posEnemy.x += currentStep;

                bool collided = false;
                if (currentStep < 0) { // Moviendo a la izquierda
                    if (map->collisionMoveLeft(posEnemy, hitboxSize)) {
                        // Colisión detectada. Revertir a posición justo tocando la pared.
                        int collidedTileX = floor(posEnemy.x / map->getTileSize());
                        posEnemy.x = (collidedTileX + 1) * map->getTileSize(); // Borde derecho del tile colisionado
                        collided = true;
                        // std::cout << "Collision Left (Incremental)! Corrected X to: " << posEnemy.x << std::endl; 
                    }
                }
                else { // Moviendo a la derecha
                    if (map->collisionMoveRight(posEnemy, hitboxSize)) {
                        // Colisión detectada. Revertir a posición justo tocando la pared.
                        int collidedTileX = floor((posEnemy.x + hitboxSize.x - 1.f) / map->getTileSize()); // Tile con el que colisiona el borde derecho
                        posEnemy.x = collidedTileX * map->getTileSize() - hitboxSize.x; // Borde izquierdo del tile colisionado - ancho
                        collided = true;
                        // std::cout << "Collision Right (Incremental)! Corrected X to: " << posEnemy.x << std::endl;
                    }
                }

                if (collided) {
                    remainingDeltaX = 0; // Detener el movimiento horizontal para este frame
                }
                else {
                    // Si no hubo colisión en este paso, confirmar la posición movida está bien
                    // y reducir el delta restante
                    remainingDeltaX -= currentStep;
                }

                safetyBreak--;
                if (safetyBreak <= 0) {
                    std::cerr << "Warning: Rana horizontal movement safety break triggered!" << std::endl;
                    remainingDeltaX = 0; // Salir del bucle forzosamente
                }
            }
        }

        // Movimiento Vertical y Colisión
        if (movementDelta.y != 0) {
            posEnemy.y += movementDelta.y;

            if (movementDelta.y > 0 && map->collisionMoveDown(posEnemy, hitboxSize, &posEnemy.y)) {
                isJumping = false; // Aterrizó
                jumpTimer = 0.0f;
                timeSinceLastJump = 0.0f;
                // Cambiar a IDLE según la última dirección
                if (jumpDirection == 1) sprite->changeAnimation(IDLE_RIGHT);
                else sprite->changeAnimation(IDLE_LEFT);
            }
        }
        // Fin de salto por tiempo
        if (jumpTimer >= jumpMaxTime && isJumping)
        {
            isJumping = false;
            jumpTimer = 0.0f;
            timeSinceLastJump = 0.0f;
            posEnemy.x = jumpStartPosition.x + (jumpSpeedX * jumpMaxTime * jumpDirection);
            posEnemy.y = jumpStartPosition.y;
            map->collisionMoveDown(posEnemy, hitboxSize, &posEnemy.y);
            // Cambiar a IDLE según la última dirección
            if (jumpDirection == 1) sprite->changeAnimation(IDLE_RIGHT);
            else sprite->changeAnimation(IDLE_LEFT);
        }
    }
    else // No está saltando
    {
        timeSinceLastJump += dtSeconds;
        posEnemy.y += FALL_STEP;
        bool onGround = map->collisionMoveDown(posEnemy, hitboxSize, &posEnemy.y);

        if (onGround && player != nullptr && player->isAlive())
        {
            if (timeSinceLastJump >= JUMP_COOLDOWN_SECONDS) {
                glm::vec2 playerPos = player->getPosition();
                float distanceX = abs(playerPos.x - posEnemy.x);
                // Usar Game::instance().getTileSize()
                float triggerDistance = (JUMP_DISTANCE_TILES * TILESIZE) * 1.5f;

                if (distanceX < triggerDistance)
                {
                    // Iniciar salto
                    isJumping = true;
                    jumpStartPosition = posEnemy;
                    jumpDirection = (playerPos.x >= posEnemy.x + hitboxSize.x / 2) ? 1 : -1;
                    jumpTimer = 0.0f;

                    // Cambiar a animación JUMP
                    if (jumpDirection == 1) {
                        sprite->changeAnimation(JUMP_RIGHT);
                    }
                    else {
                        sprite->changeAnimation(JUMP_LEFT);
                    }
                }
                else {
                    // Jugador no está en rango de salto -> Cambiar a IDLE si no lo está ya
                    // (Necesitamos saber la dirección para el IDLE correcto)
                    int currentDir = (playerPos.x >= posEnemy.x + hitboxSize.x / 2) ? 1 : -1;
                    if (currentDir == 1) {
                        if (sprite->animation() != IDLE_RIGHT) sprite->changeAnimation(IDLE_RIGHT);
                    }
                    else {
                        if (sprite->animation() != IDLE_LEFT) sprite->changeAnimation(IDLE_LEFT);
                    }
                }
            }
            else {
                // Está en cooldown -> Cambiar a IDLE si no lo está ya
                // (Necesitamos saber la dirección para el IDLE correcto)
                glm::vec2 playerPos = player->getPosition(); // Obtenemos pos del jugador para saber a dónde mirar
                int currentDir = (playerPos.x >= posEnemy.x + hitboxSize.x / 2) ? 1 : -1;
                if (currentDir == 1) {
                    if (sprite->animation() != IDLE_RIGHT) sprite->changeAnimation(IDLE_RIGHT);
                }
                else {
                    if (sprite->animation() != IDLE_LEFT) sprite->changeAnimation(IDLE_LEFT);
                }
            }
        }

    }

    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

