#ifndef _BAMBOO_INCLUDE
#define _BAMBOO_INCLUDE

#include "Enemy.h"

class Bamboo : public Enemy
{
public:
    Bamboo();
    ~Bamboo();

    // Sobreescribimos m�todos de Enemy
    void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram) override;
    void update(int deltaTime) override;

    // Sobreescribir para corregir posici�n de hitbox
    glm::vec2 getHitboxPosition() const override;

    // Funcion especifica para saber si el bamb� ha tocado el suelo
    bool hasLanded() const { return landed; }

    // Pasa a trav�s del suelo sin detenerse
    bool shouldPassThroughGround() const { return true; }
    bool shouldBeRemoved() const { return needsRemoval; }
    void markForRemoval();
private:

    bool landed;
    float groundY;        // Posici�n Y del suelo si ha tocado
    float disappearTimer; // Tiempo para desaparecer despu�s de tocar el suelo
    bool needsRemoval;
    const float FALL_SPEED = 4.0f;
    const float DISAPPEAR_TIME = 2.0f; // Segundos que tardar� en desaparecer despu�s de tocar el suelo
};

#endif