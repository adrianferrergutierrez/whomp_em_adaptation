#ifndef _RANA_INCLUDE
#define _RANA_INCLUDE

#include "Enemy.h"
#include "Player.h" // Necesario para el puntero al jugador
#include "TileMap.h"

class Rana : public Enemy
{
public:
    Rana();
    ~Rana();

    // Firma que acepta el puntero al jugador
    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* playerInstance);
    void update(int deltaTime) override;

private:
    Player* player; // Puntero al jugador para saber su posición

   
    // Variables para el salto
    bool isJumping;
    float jumpTimer;
    float jumpMaxTime;
    float jumpSpeedX;
    float jumpHeight;
    glm::vec2 jumpStartPosition;
    int jumpDirection;

    // Variable para el tiempo de espera entre saltos
    float timeSinceLastJump;

    // Constantes del salto de la rana (ajustables)
    const float JUMP_DISTANCE_TILES = 5.0f; // Distancia horizontal 
    const float JUMP_HEIGHT_PIXELS = 3.5f * 16.0f; // Altura del salto 
    const float JUMP_DURATION_SECONDS = 0.7f; // Duración del salto
    const float JUMP_COOLDOWN_SECONDS = 1.0f; // Tiempo de espera entre saltos

    enum RanaAnims
    {
        IDLE_LEFT, IDLE_RIGHT,
        JUMP_LEFT, JUMP_RIGHT
    };

};

#endif // _RANA_INCLUDE
