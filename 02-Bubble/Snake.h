#ifndef _SNAKE_INCLUDE
#define _SNAKE_INCLUDE

#include "Enemy.h"
#include "Player.h"

class Snake : public Enemy
{
public:
    Snake();
    ~Snake();

    // Sobreescribir m�todos de Enemy
    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* player = nullptr);
    void update(int deltaTime) override;

private:
    int direction;        // Direcci�n de movimiento: -1 (izquierda) o 1 (derecha)
    float moveSpeed;      // Velocidad de movimiento horizontal
    bool onGround;        // Indica si la serpiente est� en el suelo
    Player* targetPlayer; // Puntero al jugador para comportamiento
};

#endif // _SNAKE_INCLUDE