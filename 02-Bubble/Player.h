#ifndef _PLAYER_INCLUDE
#define _PLAYER_INCLUDE


#include "Sprite.h"
#include "TileMap.h"
#include "Health.h"
#include "Tronco.h"


// Player is basically a Sprite that represents the player. As such it has
// all properties it needs to track its movement, jumping, and collisions.


class Player
{

public:

	void takeDamage(int damage);
	bool isAlive() const { return health.isAlive(); }

	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram);
	void update(int deltaTime, vector<Tronco*> troncos);
	void render(glm::mat4 modelview);
	void setTileMap(TileMap* tileMap);
	void setPosition(const glm::vec2& pos);
	glm::vec2 getPosition() const { return posPlayer; }
	void setCameraPosition(const glm::vec2& pos);
	void collectPumpkin();

	int getPumpkinCount() const { return pumpinkcount; }
	int getClockCount() const { return clockscount; }
	bool getHasFlint() const { return hasFlint; }
	bool getHasHelmet() const { return hasHelmet; }
	int getMaxHealth() const { return maxHealth; }
	int getCurrentHealth() const { return health.getCurrentHealth(); }
	int getDamage() const { return damage; }
	void addHealth(int amount);
	void healToMax();
	void equipHelmet();
	void becomeInvulnerable(float duration);
	void becomeGOD();
	void becomeHuman();

private:
	void calculatAndUpdateMaxHealth();

	//atributos como vida, items, efectos
	Health health;
	int maxHealth;
	Texture spritesheet;
	Sprite* sprite;
	Sprite* spriteLanza;

	TileMap* map;
	glm::vec2 cameraPos;
	glm::ivec2 tileMapDispl, posPlayer;
	int damage;

	bool isGOD = false;
	bool isAttacking = false;
	bool izq = false;
	int attackTimer = 0;
	glm::vec2 lanzaOffset;



	bool bJumping;
	int jumpAngle, startY;

	// Variables para el parpadeo cuando es invulnerable
	bool isBlinking;
	bool visible;
	int blinkTime;


	//efectos items
	int pumpinkcount;
	int clockscount;
	bool hasFlint;
	bool hasHelmet;
	int helmet_usages;
};


#endif 


