#include "BossLeaf.h"
#include <cmath> 
#include <glm/gtc/constants.hpp> 

const int LEAF_DAMAGE = 10;
const glm::ivec2 LEAF_SIZE = glm::ivec2(9, 8);
const glm::vec2 LEAF_UV_SIZE = glm::vec2(0.5f, 1.0f);
const glm::vec2 LEAF_UV_POS = glm::vec2(0.0f, 0.0f); 


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

	if (!leafTexture.loadFromFile("images/Items_boss.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
		active = false;
		return;
	}

	startRadius = glm::distance(position, swirlCenter);
	currentRadius = startRadius;

	angularSpeed = (2.f * 2.f * glm::pi<float>()) / lifetime;
	if (abs(lifetime) > 0.01f) {
		expansionSpeed = (targetRadius - startRadius) / lifetime;
	}
	else {
		expansionSpeed = 0.f;
	}
	
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

	// Update del angulo/radio
	if (outward) {
		currentAngle -= angularSpeed * dt;
	}
	else {
		currentAngle += angularSpeed * dt;
	}
	currentRadius += expansionSpeed * dt;

	// calculo nueva pos
	position.x = swirlCenter.x + currentRadius * cos(currentAngle);
	position.y = swirlCenter.y + currentRadius * sin(currentAngle);

	if (sprite == nullptr) {
		active = false;
		return;
	}

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
	glm::ivec2 playerSize(32, 32);

	// miramos colisiones
	bool collisionX = position.x + hitboxSize.x >= playerPos.x && playerPos.x + playerSize.x >= position.x;
	bool collisionY = position.y + hitboxSize.y >= playerPos.y && playerPos.y + playerSize.y >= position.y;

	if (collisionX && collisionY) {
		return true;
	}

	return false;
}

