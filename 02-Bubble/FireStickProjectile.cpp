#include "FireStickProjectile.h"

enum FireAnim {
	FIRE_FLYING_R, FIRE_FLYING_L
};

FireStickProjectile::FireStickProjectile() {
    sprite = nullptr;
    speed = 4.0f;
    direction = 1;
    map = nullptr;
    active = true;
    size = glm::ivec2(16, 16); // Tamaño visual del sprite
}

FireStickProjectile::~FireStickProjectile() {
    if (sprite != nullptr)
        delete sprite;
}

void FireStickProjectile::init(const glm::ivec2& pos, ShaderProgram& shaderProgram, int dir) {
    texture.loadFromFile("images/projectile.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(size, glm::vec2(0.5f, 1.0f), &texture, &shaderProgram);
    sprite->setNumberAnimations(2);

    sprite->setAnimationSpeed(FIRE_FLYING_R, 12);
    sprite->addKeyframe(FIRE_FLYING_R, glm::vec2(0.0f, 0.0f));
    //sprite->addKeyframe(FIRE_FLYING, glm::vec2(0.5f, 0.0f));

    sprite->setAnimationSpeed(FIRE_FLYING_L, 12);
    sprite->addKeyframe(FIRE_FLYING_L, glm::vec2(0.5f, 0.0f));
    direction = dir;
    if(dir == 1)sprite->changeAnimation(FIRE_FLYING_R);
	else sprite->changeAnimation(FIRE_FLYING_L);
    posProj = pos;
    sprite->setPosition(glm::vec2(float(posProj.x), float(posProj.y)));
}

void FireStickProjectile::update(int deltaTime) {
    if (!active) return;

    float movement = speed * float(deltaTime) / 1000.0f * 60.0f;
    posProj.x += int(movement * direction);

    // Colisiones con el mapa
    if (map != nullptr) {
        bool collided = (direction == -1) ?
            map->collisionMoveLeft(posProj, getHitboxSize()) :
            map->collisionMoveRight(posProj, getHitboxSize());

        if (collided) {
            active = false;
            return;
        }
    }

    // Desactivar si sale de los límites del nivel
    // Get map width in pixels for boundary check
    float mapWidthPixels = map ? float(map->getMapWidth() * map->getTileSize()) : 2048.f; // Fallback if map is null

    if (posProj.x < 0 || posProj.x > mapWidthPixels) {
        active = false;
        return;
    }

    sprite->setPosition(glm::vec2(float(posProj.x), float(posProj.y)));
    sprite->update(deltaTime);
}

void FireStickProjectile::render(glm::mat4 modelview) {
    if (active && sprite != nullptr)
        sprite->render(modelview);
}

void FireStickProjectile::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

void FireStickProjectile::setPosition(const glm::vec2& pos) {
    posProj = glm::ivec2(pos);
    if (sprite != nullptr)
        sprite->setPosition(pos);
}

glm::vec2 FireStickProjectile::getPosition() const {
    return glm::vec2(posProj);
}

bool FireStickProjectile::isActive() const {
    return active;
}

void FireStickProjectile::deactivate() {
    active = false;
}

glm::ivec2 FireStickProjectile::getSize() const {
    return size;
}

glm::ivec2 FireStickProjectile::getHitboxSize() const {
    return glm::ivec2(16, 16); // Puedes ajustar esto si quieres un hitbox más pequeño
}


