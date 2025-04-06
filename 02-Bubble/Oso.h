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
    // A�adir aqu� variables espec�ficas del Oso si son necesarias m�s adelante
    enum OsoAnims
    {
        WALK_LEFT, WALK_RIGHT
        // A�adir m�s animaciones si es necesario (IDLE, ATTACK, etc.)
    };
};

#endif // _OSO_INCLUDE
