#include "Item.h"

Item::Item(ItemType type, ShaderProgram* shader) : itemType(type), sprite(nullptr), shaderProg(shader), map(nullptr), needsRemoval(false), spawnTimer(0.0f), active(false) {
    // El puntero al shader se guarda
    // sprite se inicializará en el init() de la clase hija
}

Item::~Item() {
    // El sprite debe ser liberado por la clase hija
    // ya que ella lo crea
    if (sprite != nullptr) {
        delete sprite; // Asegurarse que las clases hijas no lo borren también
        sprite = nullptr;
    }
}

void Item::setPosition(const glm::vec2& pos) {
    posItem = pos;
    if (sprite) {
        sprite->setPosition(glm::vec2(float(posItem.x), float(posItem.y)));
    }
}

const glm::vec2& Item::getPosition() const {
    return posItem;
}

ItemType Item::getType() const {
    return itemType;
}


glm::vec2 Item::getHitboxPosition() const {
    return posItem;
}

void Item::setTileMap(TileMap* tileMap) {
    map = tileMap;
}

bool Item::shouldBeRemoved() const {
    return needsRemoval;
}

bool Item::isActive() const {
    return active;
}

void Item::markForRemoval() {
    needsRemoval = true;
}
