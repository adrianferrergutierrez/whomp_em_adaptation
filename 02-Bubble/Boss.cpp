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
	// Start at ground level for spawning sequence
	// Set Y position *after* other initializations, just before setting sprite position
	posEnemy.y = GROUND_Y;
	sprite->changeAnimation(SPAWN_APPEAR); // Will be invisible initially due to state logic
	sprite->setPosition(glm::vec2(posEnemy.x, posEnemy.y));

	// std::cout << "Boss Initialized at (" << posEnemy.x << ", " << posEnemy.y << ") - State: SPAWNING" << std::endl; 
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

	// Placeholder animations for VANISHING and DEFEATED
	// Using GROUND_IDLE frame (0.25f, 0.0f) for now
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

	// std::cout << "Boss::update - Before leaf update loop. activeLeaves.size() = " << activeLeaves.size() << std::endl; 
	for (BossLeaf* leaf : activeLeaves) {
		if (leaf) {
			// std::cout << "  Updating leaf: " << leaf << std::endl;
			leaf->update(deltaTime);
			// std::cout << "  Checking collision for leaf: " << leaf << std::endl;
			if (player && leaf->isActive() && leaf->checkCollision(player)) {
				// std::cout << "    Leaf collision with player!" << std::endl;
				player->takeDamage(leaf->getDamage());
			}
		}
		else {
			// std::cout << "  Skipping null leaf pointer in update loop." << std::endl;
		}
	}
	// std::cout << "Boss::update - After leaf update loop. Before cleanup." << std::endl;

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
	// std::cout << "Boss::update - After leaf cleanup. activeLeaves.size() = " << activeLeaves.size() << std::endl; 

	if (sprite) {
		// std::cout << "Boss update end: State=" << static_cast<int>(currentState) << " Pos=(" << posEnemy.x << ", " << posEnemy.y << ") Alive=" << isAlive() << std::endl;
		sprite->setPosition(glm::vec2(float(posEnemy.x), float(posEnemy.y)));
		sprite->update(deltaTime);
	}
	else {
		// std::cerr << "Error: Boss sprite is null before final setPosition/update!" << std::endl;
	}
}

void Boss::render(glm::mat4 modelview)
{
	// std::cout << "Boss render start: State=" << static_cast<int>(currentState) << " Pos=(" << posEnemy.x << ", " << posEnemy.y << ") Timer=" << stateTimer << std::endl;

	bool shouldRenderBoss = true;
	if (currentState == BossState::SPAWNING && stateTimer <= SWIRL_DURATION) {
		// std::cout << "  -> Boss::render: Not rendering because state is SPAWNING and swirl is active" << std::endl;
		shouldRenderBoss = false;
	}
	if (currentState == BossState::VANISHING) {
		// std::cout << "  -> Boss::render: Not rendering because state is VANISHING" << std::endl;
		shouldRenderBoss = false;
	}
	if (currentState == BossState::INACTIVE) {
		// std::cout << "  -> Boss::render: Not rendering because state is INACTIVE" << std::endl;
		shouldRenderBoss = false;
	}
	if (currentState == BossState::DEFEATED) {
		// std::cout << "  -> Boss::render: Not rendering because state is DEFEATED (final)" << std::endl;
		shouldRenderBoss = false;
	}

	if (!sprite) {
		// std::cout << "  -> Boss::render: Not rendering because sprite is null" << std::endl;
		shouldRenderBoss = false;
	}

	if (shouldRenderBoss) {
		Enemy::render(modelview);
	}
	else {
		// std::cout << "  -> Boss::render: Final decision is NOT to render boss sprite." << std::endl;
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
		currentState != BossState::REAPPEARING_GROUND &&
		currentState != BossState::ASCENDING)
	{
		Enemy::takeDamage(dmg);
		// std::cout << "Boss took " << dmg << " damage. Current HP: " << health.getCurrentHealth() << std::endl; 
	}
}

int Boss::getCurrentHealthOranges() const
{
	if (!isAlive()) return 0;
	float healthPercentage = (float)health.getCurrentHealth() / (float)(MAX_HEALTH_ORANGES * BASE_HEALTH_PER_ORANGE);
	return static_cast<int>(ceil(healthPercentage * MAX_HEALTH_ORANGES));
}

void Boss::changeState(BossState newState)
{
	if (currentState == newState) return;

	// std::cout << "Boss changing state from " << static_cast<int>(currentState) << " to " << static_cast<int>(newState) << std::endl;

	if (currentState == BossState::VANISHING) {
		// std::cout << "   Clearing leaves on exiting VANISHING." << std::endl;
		clearLeaves();
	}

	currentState = newState;
	stateTimer = 0.f;
	groundAttackTriggeredThisCycle = false;

	switch (newState)
	{
	case BossState::SPAWNING:
		// std::cout << "  Entering SPAWNING state." << std::endl;
		break;
	case BossState::FLYING:
		// std::cout << "  Entering FLYING state." << std::endl;
		if (sprite)
		{
			sprite->changeAnimation(FLY);
			posEnemy.y = FLY_HEIGHT_Y;
			bambooDropTimer = BAMBOO_DROP_COOLDOWN;
		}
		break;
	case BossState::DESCENDING:
		// std::cout << "  Entering DESCENDING state." << std::endl;
		if (sprite) sprite->changeAnimation(DESCEND);
		descendStartX = posEnemy.x;
		descendZigzagAngle = 0.f;
		break;
	case BossState::ATTACKING_GROUND:
		// std::cout << "  Entering ATTACKING_GROUND state." << std::endl;
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::VANISHING:
		// std::cout << "  Entering VANISHING state." << std::endl;
		if (sprite) sprite->changeAnimation(VANISHING);
		// std::cout << "  VANISHING entry: Triggering OUTWARD swirl and BAMBOO RAIN." << std::endl;
		triggerLeafSwirl(true);
		triggerBambooRain();
		groundAttackTriggeredThisCycle = true;
		break;
	case BossState::REAPPEARING_GROUND:
		// std::cout << "  Entering REAPPEARING_GROUND state." << std::endl;
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::ASCENDING:
		// std::cout << "  Entering ASCENDING state." << std::endl;
		if (sprite) sprite->changeAnimation(FLY);
		break;
	case BossState::FALLING_DEFEATED:
		// std::cout << "  Entering FALLING_DEFEATED state." << std::endl;
		if (sprite) sprite->changeAnimation(DESCEND);
		break;
	case BossState::DYING_GROUND:
		// std::cout << "  Entering DYING_GROUND state." << std::endl;
		if (sprite) sprite->changeAnimation(GROUND_IDLE);
		posEnemy.y = GROUND_Y;
		break;
	case BossState::DEFEATED:
		// std::cout << "  Entering DEFEATED state." << std::endl;
		if (sprite) sprite->changeAnimation(DEFEATED);
		clearLeaves();
		break;
	case BossState::INACTIVE:
		// std::cout << "  Entering INACTIVE state." << std::endl;
		clearLeaves();
		break;
	}
}

void Boss::updateInactive(float dt) {
}

void Boss::updateSpawning(float dt) {
	if (stateTimer <= SWIRL_DURATION) {
		if (activeLeaves.empty() && stateTimer < 0.05f) {
			// std::cout << "Spawning Phase 1: Triggering inward swirl." << std::endl;
			triggerLeafSwirl(false);
		}
	}
	else {
		const float phase2StartTime = SWIRL_DURATION;
		if (stateTimer - dt < phase2StartTime && stateTimer >= phase2StartTime) {
			// std::cout << "Spawning Phase 2: Boss Appearing." << std::endl;
			clearLeaves();
			if (sprite) sprite->changeAnimation(SPAWN_APPEAR);
			// std::cout << "  Boss appearance triggered at current posEnemy: (" << posEnemy.x << ", " << posEnemy.y << ")" << std::endl;
			if (sprite) sprite->setPosition(glm::vec2(posEnemy.x, posEnemy.y));
		}

		const float appearDuration = 0.5f;
		if (stateTimer >= phase2StartTime + appearDuration) {
			// std::cout << "Spawning Phase 3: Appear duration over. Transitioning to ASCENDING. Current Y = " << posEnemy.y << std::endl;
			changeState(BossState::ASCENDING);
		}
	}
}

void Boss::updateFlying(float dt) {
	// stateTimer is incremented globally in Boss::update

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
				// std::cout << "  !! Approaching Right Wall: nextX=" << nextX << " limit=" << ARENA_RIGHT_X << std::endl;
				flyDirection = -1; // Reverse direction
				wallHit = true;
				// std::cout << "Boss reversing direction (Right wall)." << std::endl;
				remainingDeltaX = 0; // Stop moving further this frame after hitting wall
			}
			else if (flyDirection == -1 && nextX < ARENA_LEFT_X) {
				// std::cout << "  !! Approaching Left Wall: nextX=" << nextX << " limit=" << ARENA_LEFT_X << std::endl;
				flyDirection = 1; // Reverse direction
				wallHit = true;
				// std::cout << "Boss reversing direction (Left wall)." << std::endl;
				remainingDeltaX = 0; // Stop moving further this frame after hitting wall
			}

			// Apply movement only if no wall hit in this step
			if (!wallHit) {
				posEnemy.x = nextX;
				remainingDeltaX -= currentStep;
			}

			safetyBreak--;
			if (safetyBreak <= 0) {
				// std::cerr << "Warning: Boss flying movement safety break triggered!" << std::endl;
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
		// std::cout << "Flying duration over. Transitioning to DESCENDING." << std::endl;
		changeState(BossState::DESCENDING);
	}
}

void Boss::updateDescending(float dt) {
	// stateTimer is incremented globally in Boss::update

	// --- Vertical Movement --- 
	float intendedMovementY = DESCEND_SPEED_Y * dt;
	float nextY = posEnemy.y + intendedMovementY;

	// Check if next position reaches or passes ground
	if (nextY >= GROUND_Y) {
		// std::cout << "Boss landing. Snapping position to Y=" << GROUND_Y << std::endl;
		posEnemy.y = GROUND_Y; // Snap to ground Y exactly
		changeState(BossState::ATTACKING_GROUND);
		return; // Don't do horizontal movement or further checks this frame
	}
	else {
		// Apply vertical movement if not landing yet
		posEnemy.y = nextY;
	}

	// --- Horizontal Zigzag --- 
	descendZigzagAngle += DESCEND_ZIGZAG_FREQ * 2.f * glm::pi<float>() * dt;
	float offsetX = DESCEND_ZIGZAG_AMP * sin(descendZigzagAngle);
	// Apply horizontal offset relative to the captured starting X
	float targetX = descendStartX + offsetX;
	// Clamp horizontal position within arena boundaries
	posEnemy.x = static_cast<int>(glm::clamp(targetX, ARENA_LEFT_X, ARENA_RIGHT_X - static_cast<float>(hitboxSize.x)));

}

void Boss::updateAttackingGround(float dt) {
	// stateTimer is incremented globally in Boss::update

	// Wait for a short duration on the ground
	if (stateTimer >= GROUND_WAIT_DURATION) {
		// std::cout << "Ground wait finished. Transitioning to VANISHING." << std::endl;
		changeState(BossState::VANISHING);
	}
}

void Boss::updateVanishing(float dt)
{
	// stateTimer is incremented globally in Boss::update
	// std::cout << "Boss::updateVanishing - Timer: " << stateTimer << std::endl;

	// Phase 1: Outward Swirl (Boss invisible from start of state)
	// This phase lasts for SWIRL_DURATION seconds.
	// Attacks (swirl + rain) were triggered on entering the state.
	if (stateTimer <= SWIRL_DURATION) {
		// std::cout << "  Vanishing Phase 1: Outward swirl active (Boss invisible)." << std::endl;
	}
	// Phase 2: Trigger Inward Swirl
	else if (stateTimer <= SWIRL_DURATION * 2.0f) { // Phase 2 lasts another SWIRL_DURATION
		float phase2StartTime = SWIRL_DURATION;
		if (stateTimer - dt < phase2StartTime && stateTimer >= phase2StartTime) {
			// std::cout << "  Vanishing Phase 2: Outward swirl finished. Triggering INWARD swirl." << std::endl;
			clearLeaves(); // Clear the outward leaves
			triggerLeafSwirl(false); // false = inward
		}
		// std::cout << "  Vanishing Phase 2: Inward swirl active (Boss invisible)." << std::endl;
	}
	// Phase 3: Inward Swirl Complete - Transition to REAPPEARING_GROUND
	else {
		// std::cout << "  Vanishing Phase 3: Inward swirl finished. Transitioning to REAPPEARING_GROUND." << std::endl;
		clearLeaves(); // Clear the inward leaves
		// Reset the ground attack flag for the *next* cycle
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
	// stateTimer is incremented globally in Boss::update
	// ADD LOG: Check starting position
	// std::cout << "Boss::updateAscending START - Current Y: " << posEnemy.y << " Target Y: " << FLY_HEIGHT_Y << " Timer: " << stateTimer << std::endl;

	// Move upwards
	posEnemy.y -= ASCEND_SPEED * dt;

	// Check if reached or passed the target flying height
	if (posEnemy.y <= FLY_HEIGHT_Y)
	{
		// std::cout << "Reached fly height. Snapping Y (" << FLY_HEIGHT_Y << ") and transitioning to FLYING." << std::endl;
		posEnemy.y = FLY_HEIGHT_Y; // Snap to exact height
		changeState(BossState::FLYING);
	}
}

void Boss::updateFallingDefeated(float dt)
{
	// std::cout << "Boss::updateFallingDefeated - Current Y: " << posEnemy.y << " Target Y: " << GROUND_Y << std::endl;
	float intendedMovementY = DESCEND_SPEED_Y * dt;
	float nextY = posEnemy.y + intendedMovementY;
	if (nextY >= GROUND_Y) {
		// std::cout << "Defeated boss landed. Snapping position to Y=" << GROUND_Y << std::endl;
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
		std::cout << "  Loop i = " << i << ": Calculating parameters..." << std::endl;
		float currentAngle = i * angleIncrement;
		glm::vec2 spawnPos;
		spawnPos.x = centerPoint.x + initialRadius * cos(currentAngle);
		spawnPos.y = centerPoint.y + initialRadius * sin(currentAngle);
		std::cout << "  Loop i = " << i << ": Calculated spawnPos = (" << spawnPos.x << ", " << spawnPos.y << ")" << std::endl;

		std::cout << "  Loop i = " << i << ": Creating new BossLeaf()..." << std::endl;
		BossLeaf* newLeaf = new BossLeaf();
		std::cout << "  Loop i = " << i << ": Calling newLeaf->init(...)..." << std::endl;
		newLeaf->init(shaderProgram, spawnPos, centerPoint, currentAngle, targetRadius, SWIRL_DURATION, outward);
		std::cout << "  Loop i = " << i << ": Finished newLeaf->init(...)." << std::endl;

		std::cout << "  Loop i = " << i << ": Pushing leaf to vector..." << std::endl;
		activeLeaves.push_back(newLeaf); // Add the leaf to the active leaves vector
		std::cout << "  Loop i = " << i << ": Leaf pushed." << std::endl;
	}
}

void Boss::triggerBambooDrop() {
	std::cout << "Triggering single bamboo drop" << std::endl;
	if (scene && player) {
		// Calculate spawn position at boss's feet
		glm::vec2 spawnPos = glm::vec2(posEnemy.x + hitboxSize.x / 2.f, posEnemy.y + hitboxSize.y);
		scene->spawnSingleBamboo(spawnPos);
	}
}

void Boss::triggerBambooRain() {
	std::cout << "Triggering bamboo rain" << std::endl;
	if (scene) {
		// Spawn rain starting from the desired fixed height
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


