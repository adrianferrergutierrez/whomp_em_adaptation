#include "Scene.h" 
#include "Game.h"
#include <cmath>    
#include <glm/gtc/constants.hpp> 
#include <glm/common.hpp>       
#include "Boss.h"

// Constants
const int BASE_HEALTH_PER_ORANGE = 30;

Boss::Boss() : Enemy()
{
	player = nullptr;
	scene = nullptr;
	currentState = BossState::INACTIVE;
}

Boss::~Boss()
{
	clearLeaves();
}

void Boss::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* playerInstance, TileMap* mapInstance, Scene* sceneInstance)
{
	Enemy::init(tileMapPos, shaderProgram);

	this->player = playerInstance;
	this->scene = sceneInstance;
	this->map = mapInstance;
	this->shaderProgram = &shaderProgram;

	health = Health(MAX_HEALTH_ORANGES * BASE_HEALTH_PER_ORANGE);

	damage = BOSS_CONTACT_DAMAGE;
	hitboxSize = glm::ivec2(36, 48);

	if (!spritesheet.loadFromFile("images/boss_36_48.png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		cerr << "Error loading boss spritesheet (boss_36_48.png)" << endl;
	}

	if (!bossItemsTexture.loadFromFile("images/Items_boss.png", TEXTURE_PIXEL_FORMAT_RGBA))
	{
		cerr << "Error loading boss items spritesheet (Items_boss.png)" << endl;
	}

	sprite = Sprite::createSprite(hitboxSize, glm::vec2(0.25f, 1.0f), &spritesheet, this->shaderProgram);
	setupAnimations();

	currentState = BossState::SPAWNING;
	stateTimer = 0.f;
	posEnemy.y = GROUND_Y;
	sprite->changeAnimation(SPAWN_APPEAR); 
	sprite->setPosition(glm::vec2(posEnemy.x, posEnemy.y));
}

void Boss::setupAnimations()
{
	sprite->setNumberAnimations(6);

	sprite->setAnimationSpeed(FLY, 8);
	sprite->addKeyframe(FLY, glm::vec2(0.0f, 0.0f));

	sprite->setAnimationSpeed(GROUND_IDLE, 8);
	sprite->addKeyframe(GROUND_IDLE, glm::vec2(0.25f, 0.0f));

	sprite->setAnimationSpeed(SPAWN_APPEAR, 8);
	sprite->addKeyframe(SPAWN_APPEAR, glm::vec2(0.5f, 0.0f));

	sprite->setAnimationSpeed(DESCEND, 8);
	sprite->addKeyframe(DESCEND, glm::vec2(0.75f, 0.0f));

	sprite->setAnimationSpeed(VANISHING, 8);
	sprite->addKeyframe(VANISHING, glm::vec2(0.25f, 0.0f));

	sprite->setAnimationSpeed(DEFEATED, 8);
	sprite->addKeyframe(DEFEATED, glm::vec2(0.25f, 0.0f));
}

void Boss::update(int deltaTime)
{
	Enemy::update(deltaTime);

	// --- Death Check ---
	if (!health.isAlive() &&
		currentState != BossState::FALLING_DEFEATED &&
		currentState != BossState::DYING_GROUND &&
		currentState != BossState::DEFEATED)
	{
		// std::cout << "Boss health zero! Transitioning to FALLING_DEFEATED." << std::endl;
		clearLeaves();
		changeState(BossState::FALLING_DEFEATED);
	}

	float dt = static_cast<float>(deltaTime) / 1000.0f;
	if (currentState != BossState::DEFEATED) {
		stateTimer += dt;

	}

	switch (currentState)
	{
	case BossState::INACTIVE:
		updateInactive(dt);
		break;
	case BossState::SPAWNING:
		updateSpawning(dt);
		break;
	case BossState::FLYING:
		updateFlying(dt);
		break;
	case BossState::DESCENDING:
		updateDescending(dt);
		break;
	case BossState::ATTACKING_GROUND:
		updateAttackingGround(dt);
		break;
	case BossState::VANISHING:
		updateVanishing(dt);
		break;
	case BossState::REAPPEARING_GROUND:
		updateReappearingGround(dt);
		break;
	case BossState::ASCENDING:
		updateAscending(dt);
		break;
	case BossState::FALLING_DEFEATED:
		updateFallingDefeated(dt);
		break;
	case BossState::DYING_GROUND:
		updateDyingGround(dt);
		break;
	case BossState::DEFEATED:
		updateDefeated(dt);
		break;
	}

	for (BossLeaf* leaf : activeLeaves) {
		if (leaf) {
			leaf->update(deltaTime);
			if (player && leaf->isActive() && leaf->checkCollision(player)) {
				player->takeDamage(leaf->getDamage());
			}
		}
	
	}
	activeLeaves.erase(
		std::remove_if(activeLeaves.begin(), activeLeaves.end(),
			[](BossLeaf* leaf) {
				bool shouldRemove = (leaf != nullptr && !leaf->isActive());
				if (shouldRemove) {
					delete leaf;
				}
				return shouldRemove;
			}),
		activeLeaves.end()
	);

	if (sprite) {
		sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
		sprite->update(deltaTime);
	}
	
}

void Boss::render(glm::mat4 modelview)
{

	bool shouldRenderBoss = true;
	if (currentState == BossState::SPAWNING && stateTimer <= SWIRL_DURATION) {
		shouldRenderBoss = false;
	}
	if (currentState == BossState::VANISHING) {
		shouldRenderBoss = false;
	}
	if (currentState == BossState::INACTIVE) {
		shouldRenderBoss = false;
	}
	if (currentState == BossState::DEFEATED) {
		shouldRenderBoss = false;
	}

	if (!sprite) {
		shouldRenderBoss = false;
	}

	if (shouldRenderBoss) {
		Enemy::render(modelview);
	}
	

	for (BossLeaf* leaf : activeLeaves) {
		if (leaf) leaf->render(modelview);
	}
}

void Boss::takeDamage(int dmg)
{
	if (currentState != BossState::DEFEATED &&
		currentState != BossState::SPAWNING &&
		currentState != BossState::VANISHING &&
		currentState != BossState::FALLING_DEFEATED &&
		currentState != BossState::DYING_GROUND &&
		currentState != BossState::REAPPEARING_GROUND
		)
	{
		Enemy::takeDamage(dmg);
	}
}


void Boss::changeState(BossState newState)
{
	if (currentState == newState) return;

	if (currentState == BossState::VANISHING) {
		clearLeaves();
	}

	currentState = newState;
	stateTimer = 0.f;
	groundAttackTriggeredThisCycle = false;

	switch (newState)
	{
	case BossState::SPAWNING:
		break;
	case BossState::FLYING:
		if (sprite)
		{
			sprite->changeAnimation(FLY);
			posEnemy.y = FLY_HEIGHT_Y;
			bambooDropTimer = BAMBOO_DROP_COOLDOWN;
		}
		break;
	case BossState::DESCENDING:
		if (sprite) sprite->changeAnimation(DESCEND);
		descendStartX = posEnemy.x;
		descendZigzagAngle = 0.f;
		break;
	case BossState::ATTACKING_GROUND:
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::VANISHING:
		if (sprite) sprite->changeAnimation(VANISHING);
		triggerLeafSwirl(true);
		triggerBambooRain();
		groundAttackTriggeredThisCycle = true;
		break;
	case BossState::REAPPEARING_GROUND:
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::ASCENDING:
		if (sprite) sprite->changeAnimation(FLY);
		break;
	case BossState::FALLING_DEFEATED:
		if (sprite) sprite->changeAnimation(DESCEND);
		break;
	case BossState::DYING_GROUND:
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::DEFEATED:
		if (sprite) sprite->changeAnimation(DEFEATED);
		clearLeaves();
		break;
	case BossState::INACTIVE:
		clearLeaves();
		break;
	}
}

void Boss::updateInactive(float dt) {
}

void Boss::updateSpawning(float dt) {
	if (stateTimer <= SWIRL_DURATION) {
		if (activeLeaves.empty() && stateTimer < 0.05f) {
			triggerLeafSwirl(false);
		}
	}
	else {
		const float phase2StartTime = SWIRL_DURATION;
		if (stateTimer - dt < phase2StartTime && stateTimer >= phase2StartTime) {
			clearLeaves();
			if (sprite) sprite->changeAnimation(SPAWN_APPEAR);
			if (sprite) sprite->setPosition(glm::vec2(posEnemy.x, posEnemy.y));
		}

		const float appearDuration = 0.5f;
		if (stateTimer >= phase2StartTime + appearDuration) {
			changeState(BossState::ASCENDING);
		}
	}
}

void Boss::updateFlying(float dt) {

	// --- Movement --- 
	float intendedMovementX = flyDirection * FLY_SPEED * dt;
	if (abs(intendedMovementX) > 0.01f) {
		float remainingDeltaX = intendedMovementX;
		const float maxStepX = hitboxSize.x / 2.0f;
		int safetyBreak = 100;

		while (abs(remainingDeltaX) > 0.01f && safetyBreak > 0) {
			float currentStep = glm::clamp(remainingDeltaX, -maxStepX, maxStepX);
			float nextX = posEnemy.x + currentStep;
			bool wallHit = false;

			// Check arena boundaries BEFORE applying the step
			if (flyDirection == 1 && nextX + hitboxSize.x > ARENA_RIGHT_X) {
				flyDirection = -1; // Reverse direction
				wallHit = true;
				remainingDeltaX = 0; // Stop moving further this frame after hitting wall
			}
			else if (flyDirection == -1 && nextX < ARENA_LEFT_X) {
				flyDirection = 1; // Reverse direction
				wallHit = true;
				remainingDeltaX = 0; // Stop moving further this frame after hitting wall
			}

			// Apply movement only if no wall hit in this step
			if (!wallHit) {
				posEnemy.x = nextX;
				remainingDeltaX -= currentStep;
			}

			safetyBreak--;
			if (safetyBreak <= 0) {
				remainingDeltaX = 0;
			}
		}
	}

	// --- Attacks --- 
	bambooDropTimer -= dt;
	if (bambooDropTimer <= 0.f) {
		triggerBambooDrop();
		bambooDropTimer = BAMBOO_DROP_COOLDOWN; // Reset cooldown after dropping
	}

	// --- State Transition --- 
	if (stateTimer >= FLY_DURATION) {
		changeState(BossState::DESCENDING);
	}
}

void Boss::updateDescending(float dt) {

	// --- Vertical Movement --- 
	float intendedMovementY = DESCEND_SPEED_Y * dt;
	float nextY = posEnemy.y + intendedMovementY;

	// Check if next position reaches or passes ground
	if (nextY >= GROUND_Y) {
		posEnemy.y = GROUND_Y; 
		changeState(BossState::ATTACKING_GROUND);
		return; 
	}
	else {
		// Apply vertical movement if not landing yet
		posEnemy.y = nextY;
	}

	// --- Horizontal Zigzag --- 
	descendZigzagAngle += DESCEND_ZIGZAG_FREQ * 2.f * glm::pi<float>() * dt;
	float offsetX = DESCEND_ZIGZAG_AMP * sin(descendZigzagAngle);
	float targetX = descendStartX + offsetX;
	posEnemy.x = static_cast<int>(glm::clamp(targetX, ARENA_LEFT_X, ARENA_RIGHT_X - static_cast<float>(hitboxSize.x)));

}

void Boss::updateAttackingGround(float dt) {

	if (stateTimer >= GROUND_WAIT_DURATION) {
		changeState(BossState::VANISHING);
	}
}

void Boss::updateVanishing(float dt)
{

	if (stateTimer <= SWIRL_DURATION) {}
	else if (stateTimer <= SWIRL_DURATION * 2.0f) { 
		float phase2StartTime = SWIRL_DURATION;
		if (stateTimer - dt < phase2StartTime && stateTimer >= phase2StartTime) {
			clearLeaves(); // Clear 
			triggerLeafSwirl(false);
		}
	}
	else {
		clearLeaves(); //clear
		groundAttackTriggeredThisCycle = false;
		changeState(BossState::REAPPEARING_GROUND);
	}
}

void Boss::updateReappearingGround(float dt)
{

	if (stateTimer >= REAPPEAR_DURATION)
	{
		changeState(BossState::ASCENDING);
	}
}

void Boss::updateAscending(float dt)
{
	posEnemy.y -= ASCEND_SPEED * dt;

	if (posEnemy.y <= FLY_HEIGHT_Y)
	{
		posEnemy.y = FLY_HEIGHT_Y; // Snap to exact height
		changeState(BossState::FLYING);
	}
}

void Boss::updateFallingDefeated(float dt)
{
	float intendedMovementY = DESCEND_SPEED_Y * dt;
	float nextY = posEnemy.y + intendedMovementY;
	if (nextY >= GROUND_Y) {
		posEnemy.y = GROUND_Y;
		changeState(BossState::DYING_GROUND);
	}
	else {
		posEnemy.y = nextY;
	}
}

void Boss::updateDyingGround(float dt)
{
	if (stateTimer >= DEATH_POSE_DURATION)
	{
		changeState(BossState::DEFEATED);
	}
}

void Boss::updateDefeated(float dt) {

}

void Boss::triggerLeafSwirl(bool outward)
{
	clearLeaves();

	glm::vec2 centerPoint = glm::vec2(posEnemy.x + hitboxSize.x / 2.f, posEnemy.y + hitboxSize.y / 2.f);
	float initialRadius, targetRadius;

	if (outward) {
		initialRadius = 5.f;
		targetRadius = SWIRL_RADIUS;
	}
	else { // Inward
		initialRadius = SWIRL_RADIUS;
		targetRadius = 0.f;
	}

	float angleIncrement = (2.f * glm::pi<float>()) / SWIRL_LEAF_COUNT;

	if (!shaderProgram) {
		std::cerr << "Error: Boss shader program member is null in triggerLeafSwirl!" << std::endl;
		return;
	}
	std::cout << "   Shader program pointer in triggerLeafSwirl: " << shaderProgram << std::endl;

	for (int i = 0; i < SWIRL_LEAF_COUNT; ++i) {
		float currentAngle = i * angleIncrement;
		glm::vec2 spawnPos;
		spawnPos.x = centerPoint.x + initialRadius * cos(currentAngle);
		spawnPos.y = centerPoint.y + initialRadius * sin(currentAngle);
		BossLeaf* newLeaf = new BossLeaf();
		newLeaf->init(shaderProgram, spawnPos, centerPoint, currentAngle, targetRadius, SWIRL_DURATION, outward);
		activeLeaves.push_back(newLeaf); // Add the leaf to the active leaves vector
	}
}

void Boss::triggerBambooDrop() {
	if (scene && player) {
		glm::vec2 spawnPos = glm::vec2(posEnemy.x + hitboxSize.x / 2.f, posEnemy.y + hitboxSize.y);
		scene->spawnSingleBamboo(spawnPos);
	}
}

void Boss::triggerBambooRain() {
	if (scene) {
		const float BAMBOO_RAIN_SPAWN_Y = 30.f * 16.f; // 30 tiles high
		scene->spawnBambooRain(ARENA_LEFT_X, ARENA_RIGHT_X, BAMBOO_RAIN_SPAWN_Y);
	}
}

void Boss::clearLeaves() {
	for (BossLeaf* leaf : activeLeaves) {
		delete leaf;
	}
	activeLeaves.clear();
}


