#include <algorithm>
#include "Health.h"

Health::Health(int maxHealth)
{
    this->maxHealth = maxHealth;
    this->currentHealth = maxHealth;
    this->isInvulnerable = false;
    this->invulnerabilityTimer = 0.0f;
}

Health::~Health()
{
}

void Health::takeDamage(int damage)
{
    if (!isInvulnerable && damage > 0) {
        currentHealth = std::max(0, currentHealth - damage);
        isInvulnerable = true;
        invulnerabilityTimer = 2.0f; // 2 segundos de invulnerabilidad
    }
}

void Health::heal(int amount)
{
    if (amount > 0) {
        currentHealth = std::min(maxHealth, currentHealth + amount);
    }
}

void Health::beInvulnerable(float deltaTime)
{
    invulnerabilityTimer = 5.0f; // Hecho para el objecto de bisonte

}

void Health::update(float deltaTime)
{
    // Actualizar temporizador de invulnerabilidad
    if (isInvulnerable) {
        invulnerabilityTimer -= deltaTime;
        if (invulnerabilityTimer <= 0.0f) {
            isInvulnerable = false;
            invulnerabilityTimer = 0.0f;
        }
    }
}

void Health::setMaxHealth(int value)
{
    if (value > 0) {
        maxHealth = value;
        //no podemos tener mas vida que la maxima
        currentHealth = std::min(currentHealth, maxHealth);
    }
}

void Health::reset()
{
    currentHealth = maxHealth;
    isInvulnerable = true;
    invulnerabilityTimer = 2.0f;
}