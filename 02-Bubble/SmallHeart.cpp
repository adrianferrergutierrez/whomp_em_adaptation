#include "SmallHeart.h"
#include "Game.h" // Para acceso a constantes como CAMERA_WIDTH/HEIGHT si es necesario
#include <iostream> // Para std::cout temporal
#include <glm/gtc/matrix_transform.hpp> 

// Definir constante para el delay
const float SPAWN_DELAY_SECONDS = 1.0f;

SmallHeart::SmallHeart(ShaderProgram* shader)
    : Item(ItemType::SMALL_HEART, shader), upwardSpeed(20.0f) // Llamar constructor base y inicializar velocidad
{
    // El shader ya se pasó al constructor base
}

SmallHeart::~SmallHeart() {
    // El destructor de Item se encargará de borrar el sprite si existe
    // La textura se libera automáticamente al salir del alcance
}

void SmallHeart::init(const glm::vec2& pos) {
    posItem = pos; // Guardar posición inicial

    // Cargamos la textura de Corazones.png (el mismo spriteSheet que hemos usado en la GUI para los corazones)
    if (!spritesheet.loadFromFile("images/Corazones.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando images/Corazones.png para SmallHeart!" << std::endl; // Actualizar mensaje de error también
        markForRemoval();
        return;
    }

    // Creamos el sprite, cogemos el 5to elemento de la hoja de sprites de corazones
    sprite = Sprite::createSprite(glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT), glm::vec2(0.2f, 1.0f), &spritesheet, shaderProg);

    if (sprite == nullptr) {
        std::cout << "Error: Sprite::createSprite devolvió nullptr para SmallHeart!" << std::endl;
        markForRemoval();
        return;
    }

    //Solo tiene 1 animacion porque es "estatico" aunque flotee hacia arriba
    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.8f, 0.0f));
    sprite->changeAnimation(0);
    sprite->setAnimationSpeed(0, 100);
    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));

}

void SmallHeart::update(int deltaTime) {
    if (needsRemoval || sprite == nullptr) return;

    float dt = deltaTime / 1000.0f;

    // Aplicamos un delay de activacion
    if (!active) {
        spawnTimer += dt;
        if (spawnTimer >= SPAWN_DELAY_SECONDS) {
            active = true;
        }
        else {
            return;
        }
    }

    //Si esta activo hacemos que se mueva hacia arriba de forma constante

    posItem.y -= upwardSpeed * dt;
    sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
    sprite->update(deltaTime);

    if (posItem.y < -ITEM_HEIGHT) {
        markForRemoval();
    }
}

void SmallHeart::render(glm::mat4& modelview) {
    // Solo renderizamos si esta activo i no marcado para borrar
    if (active && !needsRemoval && sprite != nullptr) {
        sprite->render(modelview);
    }
}

void SmallHeart::applyEffect(Player* player) {
    // Curamos al player 1 corazon entero cuando lo toque
    if (active && player) {
        player->addHealth(30);
        markForRemoval();
    }
}

glm::ivec2 SmallHeart::getHitboxSize() const {
    if (active) {
        return glm::ivec2(ITEM_WIDTH, ITEM_HEIGHT);
    }
    else {
        return glm::ivec2(0, 0);
    }
}
