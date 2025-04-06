#ifndef _OSO_INCLUDE
#define _OSO_INCLUDE

#include "Enemy.h"

class Oso : public Enemy
{
public:
    Oso();
    ~Oso();

    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;

private:
    // Añadir aquí variables específicas del Oso si son necesarias más adelante
    enum OsoAnims
    {
        WALK_LEFT, WALK_RIGHT
        // Añadir más animaciones si es necesario (IDLE, ATTACK, etc.)
    };
};

#endif // _OSO_INCLUDE
