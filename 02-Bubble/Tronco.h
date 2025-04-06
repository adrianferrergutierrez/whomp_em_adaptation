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

    // M�todos para posici�n y colisiones
    glm::vec2 getPosition() const;
    glm::ivec2 getSize() const { return size; }

    void setTileMap(TileMap* tileMap);

    // Comprobar si el jugador est� sobre el tronco
    bool isPlayerOnTop(const glm::vec2& playerPos, const glm::ivec2& playerSize) const;

    // Obtener el desplazamiento vertical desde la �ltima actualizaci�n
    float getVerticalDisplacement() const { return verticalDisplacement; }

private:
    glm::ivec2 tileMapDispl;
    glm::vec2 position;
    glm::ivec2 size;               // Tama�o del tronco
    Texture spritesheet;
    Sprite* sprite;
    TileMap* map;

    // Variables para el movimiento vertical
    float initialY;                // Posici�n Y inicial
    float currentY;                // Posici�n Y actual
    float amplitude;               // Amplitud del movimiento (en p�xeles)
    float speed;                   // Velocidad del movimiento
    float time;                    // Tiempo para calcular la posici�n
    float verticalDisplacement;    // Desplazamiento vertical desde la �ltima actualizaci�n

    // Constantes
    const float MOVEMENT_AMPLITUDE = 5.0f * 16.0f; // 5 tiles de movimiento vertical
    const float BASE_MOVEMENT_AMPLITUDE = 1.0f * 16.0f; // Reduced base to 1 tile of movement vertical
    const float MOVEMENT_SPEED = 3.0f;            // Velocidad de oscilacin
};

#endif // _TRONCO_INCLUDE
