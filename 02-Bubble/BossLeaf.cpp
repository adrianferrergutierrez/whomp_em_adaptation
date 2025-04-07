#include "BossLeaf.h"
#include <cmath> // For sin, cos
#include <glm/gtc/constants.hpp> // For pi

// Constants
const int LEAF_DAMAGE = 10;
const glm::ivec2 LEAF_SIZE = glm::ivec2(9, 8);
// UV coordinates for the leaf in Items_boss.png (Assuming 18x8 texture with 2 leaves)
const glm::vec2 LEAF_UV_SIZE = glm::vec2(0.5f, 1.0f);
const glm::vec2 LEAF_UV_POS = glm::vec2(0.0f, 0.0f); // Use the first leaf


BossLeaf::BossLeaf()
{
	shaderProgram = nullptr;
	sprite = nullptr;
	active = false;
	damage = LEAF_DAMAGE;
	hitboxSize = LEAF_SIZE;
}

BossLeaf::~BossLeaf()
{
	if (sprite != nullptr) {
		delete sprite;
	}
}

void BossLeaf::init(ShaderProgram* shader,
	const glm::vec2& spawnPos, const glm::vec2& centerPoint,
	float initialAngle, float targetRad, float duration, bool isOutwardSwirl)
{
	shaderProgram = shader;
	position = spawnPos;
	swirlCenter = centerPoint;
	startAngle = initialAngle;
	currentAngle = initialAngle;
	targetRadius = targetRad;
	lifetime = duration;
	outward = isOutwardSwirl;
	age = 0.f;
	active = true;

	// Load texture internally
	if (!leafTexture.loadFromFile("images/Items_boss.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		// cout << "Error loading Items_boss.png in BossLeaf::init!" << endl;
		active = false;
		return;
	}

	// Calculate initial radius
	startRadius = glm::distance(position, swirlCenter);
	currentRadius = startRadius;
	// std::cout << "    Leaf init: startR=" << startRadius << " targetR=" << targetRadius << " lifetime=" << lifetime << " outward=" << outward << std::endl;

	// Calculate speeds
	// Angular speed: Let's make it rotate 2 full circles during its lifetime? Adjust as needed.
	angularSpeed = (2.f * 2.f * glm::pi<float>()) / lifetime;
	// Expansion speed: Change in radius over time
	if (abs(lifetime) > 0.01f) {
		expansionSpeed = (targetRadius - startRadius) / lifetime;
	}
	else {
		expansionSpeed = 0.f;
	}
	// std::cout << "    Leaf init: angularSpeed=" << angularSpeed << " expansionSpeed=" << expansionSpeed << std::endl;

	// Create Sprite using the internal texture
	sprite = Sprite::createSprite(LEAF_SIZE, LEAF_UV_SIZE, &leafTexture, shaderProgram);
	sprite->setNumberAnimations(1);
	sprite->addKeyframe(0, LEAF_UV_POS); // Single frame animation
	sprite->setAnimationSpeed(0, 8);
	sprite->changeAnimation(0);
	sprite->setPosition(position);
}

void BossLeaf::update(int deltaTime)
{
	if (!active) return;

	float dt = static_cast<float>(deltaTime) / 1000.0f;
	age += dt;

	if (age >= lifetime) {
		active = false;
		return;
	}

	// Update angle and radius
	if (outward) {
		currentAngle -= angularSpeed * dt;
	}
	else {
		currentAngle += angularSpeed * dt;
	}
	currentRadius += expansionSpeed * dt;

	// Calculate new position
	position.x = swirlCenter.x + currentRadius * cos(currentAngle);
	position.y = swirlCenter.y + currentRadius * sin(currentAngle);

	if (sprite == nullptr) {
		// std::cout << "  Error: sprite is null in BossLeaf::update!" << std::endl;
		active = false;
		return;
	}
	// std::cout << "    Leaf update: age="<< age <<" dt="<< dt << " angle=" << currentAngle << " radius=" << currentRadius << " pos = (" << position.x << ", " << position.y << ")" << std::endl;

	sprite->setPosition(position);
	sprite->update(deltaTime);
}

void BossLeaf::render(glm::mat4 modelview)
{
	if (active && sprite != nullptr) {
		sprite->render(modelview);
	}
}

bool BossLeaf::checkCollision(Player* player)
{
	if (!active || !player || !player->isAlive()) return false;

	glm::vec2 playerPos = player->getPosition();
	// Assuming player size is 32x32, replace with actual player hitbox if different
	glm::ivec2 playerSize(32, 32);

	// AABB Collision Check
	bool collisionX = position.x + hitboxSize.x >= playerPos.x && playerPos.x + playerSize.x >= position.x;
	bool collisionY = position.y + hitboxSize.y >= playerPos.y && playerPos.y + playerSize.y >= position.y;

	if (collisionX && collisionY) {
		// Collision detected!
		return true;
	}

	return false;
}

