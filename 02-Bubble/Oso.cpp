#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Oso.h"

Oso::Oso() : Enemy()
{
    // Valores iniciales específicos del Oso
    health = Health(150); // Más vida que el enemigo base?
    damage = 20;          // Más daño?
    hitboxSize = glm::ivec2(32, 32); // Tamaño de la hitbox
}

Oso::~Oso()
{
    // El sprite se libera en el destructor de Enemy
}

void Oso::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    Enemy::init(tileMapPos, shaderProgram); // Llama al init base

    // Cargar spritesheet y crear sprite
    spritesheet.loadFromFile("images/Enemigos.png", TEXTURE_PIXEL_FORMAT_RGBA);
    // Tamaño visual y relativo de textura (asumiendo hoja 128x128)
    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25f, 0.25f), &spritesheet, &shaderProgram);

    sprite->setNumberAnimations(2); // WALK_LEFT, WALK_RIGHT

    // Configurar animación WALK_LEFT (Asumir fila Y=0.75)
    sprite->setAnimationSpeed(WALK_LEFT, 8); // Ajustar velocidad
    sprite->addKeyframe(WALK_LEFT, glm::vec2(0.0f, 0.75f));
    sprite->addKeyframe(WALK_LEFT, glm::vec2(0.25f, 0.75f));
    sprite->addKeyframe(WALK_LEFT, glm::vec2(0.50f, 0.75f));
    sprite->addKeyframe(WALK_LEFT, glm::vec2(0.75f, 0.75f));

    // Configurar animación WALK_RIGHT (Asumir fila Y=0.5)
    sprite->setAnimationSpeed(WALK_RIGHT, 8); // Ajustar velocidad
    sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.0f, 0.5f));
    sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.25f, 0.5f));
    sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.50f, 0.5f));
    sprite->addKeyframe(WALK_RIGHT, glm::vec2(0.75f, 0.5f));

    // Animación inicial (por defecto, puede cambiar en update)
    sprite->changeAnimation(WALK_RIGHT);

    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

void Oso::update(int deltaTime)
{
    Enemy::update(deltaTime); // Llama al update base (actualiza sprite)

    // Lógica de movimiento simple (ej: moverse de lado a lado)
    // (Aquí iría la IA básica del oso, por ahora solo actualiza el sprite)
    // Ejemplo: Si se mueve a la izquierda, cambiar animación a WALK_LEFT
    // Si se mueve a la derecha, cambiar animación a WALK_RIGHT

    // Actualizar la posición final del sprite
    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

// Nota: getHitboxPosition y otros métodos se heredan de Enemy y usan hitboxSize.
// Si necesitas centrar la hitbox de manera diferente, sobrescríbelos aquí.
