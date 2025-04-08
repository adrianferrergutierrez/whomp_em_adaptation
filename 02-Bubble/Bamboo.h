#ifndef _BAMBOO_INCLUDE
#define _BAMBOO_INCLUDE

#include "Enemy.h"

class Bamboo : public Enemy
{
public:
    Bamboo();
    ~Bamboo();

    // Sobreescribimos métodos de Enemy
    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;

    // Sobreescribir para corregir posición de hitbox
    glm::vec2 getHitboxPosition() const override;

    // Funcion especifica para saber si el bambú ha tocado el suelo
    bool hasLanded() const { return landed; }

    // Pasa a través del suelo sin detenerse
    bool shouldPassThroughGround() const { return true; }
    bool shouldBeRemoved() const { return needsRemoval; }
    void markForRemoval();
private:

    bool landed;
    float groundY;        // Posición Y del suelo si ha tocado
    float disappearTimer; // Tiempo para desaparecer después de tocar el suelo
    bool needsRemoval;
    const float FALL_SPEED = 4.0f;
    const float DISAPPEAR_TIME = 2.0f; // Segundos que tardará en desaparecer después de tocar el suelo
};

#endif