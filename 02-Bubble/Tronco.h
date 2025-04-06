#ifndef _TRONCO_INCLUDE
#define _TRONCO_INCLUDE

#include "Sprite.h"
#include "TileMap.h"

class Tronco
{
public:
    Tronco();
    ~Tronco();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
    void update(int deltaTime);
    void render(glm::mat4 modelview);

    // Métodos para posición y colisiones
    glm::vec2 getPosition() const;
    glm::ivec2 getSize() const { return size; }

    void setTileMap(TileMap* tileMap);

    // Comprobar si el jugador está sobre el tronco
    bool isPlayerOnTop(const glm::vec2& playerPos, const glm::ivec2& playerSize) const;

    // Obtener el desplazamiento vertical desde la última actualización
    float getVerticalDisplacement() const { return verticalDisplacement; }

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 position;
    glm::ivec2 size;               // Tamaño del tronco
    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;

    // Variables para el movimiento vertical
    float initialY;                // Posición Y inicial
    float currentY;                // Posición Y actual
    float amplitude;               // Amplitud del movimiento (en píxeles)
    float speed;                   // Velocidad del movimiento
    float time;                    // Tiempo para calcular la posición
    float verticalDisplacement;    // Desplazamiento vertical desde la última actualización

    // Constantes
    const float MOVEMENT_AMPLITUDE = 5.0f * 16.0f; // 5 tiles de movimiento vertical
    const float BASE_MOVEMENT_AMPLITUDE = 2.0f * 16.0f; // Base 2 tiles de movimiento vertical
    const float MOVEMENT_SPEED = 1.0f;            // Velocidad de oscilación
};

#endif // _TRONCO_INCLUDE
