#ifndef _BOSS_LEAF_INCLUDE
#define _BOSS_LEAF_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "Player.h" // Needed for collision check
#include <iostream>

class BossLeaf
{

public:
	BossLeaf();
	~BossLeaf();

	// Initialization
	void init(ShaderProgram* shader, /* Texture* spritesheetTexture, */
		const glm::vec2& spawnPos, const glm::vec2& centerPoint,
		float initialAngle, float targetRadius, float duration, bool isOutwardSwirl);

	// Core methods
	void update(int deltaTime);
	void render(glm::mat4 modelview);

	// Collision & Status
	bool checkCollision(Player* player); // Checks collision and returns true if hit
	bool isActive() const { return active; }
	void markForRemoval() { active = false; }
	glm::vec2 getPosition() const { return position; }
	glm::ivec2 getHitboxSize() const { return hitboxSize; }
	int getDamage() const { return damage; }


private:
	ShaderProgram* shaderProgram;
	Sprite* sprite;
	Texture leafTexture;
	// Texture* texture; // Pointer received in init

	glm::vec2 position;
	glm::ivec2 hitboxSize;
	int damage;
	bool active;

	// Swirl parameters
	glm::vec2 swirlCenter;
	float startAngle;
	float currentAngle;
	float startRadius;
	float targetRadius;
	float currentRadius;
	float angularSpeed;   // Radians per second
	float expansionSpeed; // Radius change per second
	bool outward;

	// Lifetime
	float age;
	float lifetime;

};

#endif // _BOSS_LEAF_INCLUDE
