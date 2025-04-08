#ifndef _HEALTH_INCLUDE
#define _HEALTH_INCLUDE

class Health {
private:
    int currentHealth;
    int maxHealth;
    bool isInvulnerable;
    float invulnerabilityTimer;

public:
    Health(int maxHealth = 100); //default 100 de vida, aunque realmente no lo uso.
    ~Health();

    void beInvulnerable(float deltaTime);

    void takeDamage(int damage, bool enemy);
    void heal(int amount);
    void update(float deltaTime);

    bool isAlive() const { return currentHealth > 0; }
    int getCurrentHealth() const { return currentHealth; }
    int getMaxHealth() const { return maxHealth; }
    bool getIsInvulnerable() const { return isInvulnerable; }

    void setMaxHealth(int value);
    void reset();
};

#endif 