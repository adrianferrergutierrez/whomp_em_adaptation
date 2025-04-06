#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Bamboo.h"

enum BambooAnims
{
    FALLING
};

Bamboo::Bamboo() : Enemy()
{
    landed = false;
    disappearTimer = 0.0f;
    groundY = 0.0f;
    damage = 10;
    hitboxSize = glm::ivec2(16, 16);
}

Bamboo::~Bamboo() {}

void Bamboo::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    //Inicializamos la clase padre
    Enemy::init(tileMapPos, shaderProgram);

    // Configuración específica del bambú
    spritesheet.loadFromFile("images/Enemigos2.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(32, 20), glm::vec2(0.25f, 0.25f), &spritesheet, &shaderProgram);
    sprite->setNumberAnimations(1); // Una animación: cayendo

    sprite->setAnimationSpeed(FALLING, 10);
    sprite->addKeyframe(FALLING, glm::vec2(0.0f, 0.0f));
    sprite->changeAnimation(FALLING);

    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

void Bamboo::update(int deltaTime)
{
    // Llamamos el update de la clase padre
    Enemy::update(deltaTime);

    float deltaTimeSeconds = deltaTime / 1000.0f;

    posEnemy.y += FALL_SPEED; //cae con un MRU de forma constante

    //Miramos si ha tocado el suelo, aunque no lo eliminamos, como en el juego original
    if (!landed && map != nullptr) {
        glm::ivec2 posAux = posEnemy;
        if (map->collisionMoveDown(posAux, hitboxSize, &posAux.y)) {
            landed = true;
            groundY = posAux.y;
        }
    }

    // Si ya ha tocado el suelo y está cayendo más allá del suelo lo eliminamos
    if (landed) {
        disappearTimer += deltaTimeSeconds;

        //lo hacemos desaparecer
        if (posEnemy.y > groundY + 100 || disappearTimer > DISAPPEAR_TIME) {
            health.takeDamage(100); // Con esto sabemos que debe desaparecer
        }
    }
    sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
}

// Override de GetHitboxPosition hecha por ia para centrar la hitbox 16x16 dentro del visual 16x24
glm::vec2 Bamboo::getHitboxPosition() const
{
    glm::vec2 visualTopLeft = Enemy::getPosition();
    float visualHeight = 20.0f;
    float hitboxHeight = float(hitboxSize.y); // Debería ser 16.0f
    float offsetY = (visualHeight - hitboxHeight) / 2.0f; // Debería ser (24-16)/2 = 4.0f

    // Devolver la posición del sprite + offset vertical
    return visualTopLeft + glm::vec2(0.0f, offsetY);
}

