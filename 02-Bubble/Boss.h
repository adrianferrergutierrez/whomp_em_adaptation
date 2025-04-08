#ifndef _BOSS_INCLUDE
#define _BOSS_INCLUDE

#include <glm/glm.hpp>
#include <glm/common.hpp> // Include for glm::clamp
#include <vector>
#include "Enemy.h"
#include "Player.h"
#include "BossLeaf.h"
#include "ShaderProgram.h"
#include "TileMap.h" // Include TileMap for potential use
#include "Texture.h" // Include Texture for Texture member variable
#include <iostream>
// Forward declaration
class Scene;

class Boss : public Enemy
{

public:
	Boss();
	~Boss();

	// Overridden methods from Enemy
	void init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram, Player* playerInstance, TileMap* mapInstance, Scene* sceneInstance);
	void update(int deltaTime) override;
	void render(glm::mat4 modelview) override;
	void takeDamage(int damage) override;
	int getCurrentHealth() const { return health.getCurrentHealth(); }
	// Getters for GUI or Scene
	const int MAX_HEALTH_ORANGES = 10; // Public constant for GUI

private:
	// --- States & Animations ---
	enum class BossState {
		INACTIVE, // Before activation
		SPAWNING, // Initial leaf swirl in, appear on ground
		FLYING,   // Horizontal sweep, dropping bamboo
		DESCENDING, // Zigzag down
		ATTACKING_GROUND, // On ground, wait briefly
		VANISHING, // Leaf swirl out, invisible, leaf swirl in
		REAPPEARING_GROUND, // Briefly visible on ground after vanishing
		ASCENDING, // Flying up to FLY_HEIGHT_Y
		FALLING_DEFEATED, // Falling down after health reaches zero
		DYING_GROUND, // On ground after falling, before disappearing
		DEFEATED // Final state after death sequence (effectively inactive)
	};

	enum BossAnims {
		FLY = 0,
		GROUND_IDLE = 1, // Anim 2 on spritesheet (Used for ground wait, dying)
		SPAWN_APPEAR = 2,// Anim 3 on spritesheet (Used for initial spawn)
		DESCEND = 3,     // Anim 4 on spritesheet (Used for descending & falling defeated)
		VANISHING = 4,   // Placeholder for vanishing animation
		DEFEATED = 5     // Placeholder for defeated animation (Maybe unused now)
	};

	BossState currentState;

	// --- Core Components & References ---
	Player* player;  // Reference to the player
	Scene* scene;    // Reference to the scene to trigger spawns
	ShaderProgram* shaderProgram; // Store shader program pointer
	Texture bossItemsTexture; // Texture for leaves, etc.

	// --- Health & Damage ---
	// Inherits Health object from Enemy, configure in init.
	// Damage dealt by boss contact (override default Enemy damage)
	const int BOSS_CONTACT_DAMAGE = 20;

	// --- Movement Parameters ---
	const float FLY_DURATION = 3.5f; // Increased duration (was 2.0f)
	const float FLY_HEIGHT_Y = 30.f * 16.f; // Target Y for flying phase
	const float FLY_SPEED = 100.f; // Horizontal speed (pixels/sec), adjust!
	const float GROUND_Y = 37.f * 16.f; // Correct ground level (was 45.f)
	const float DESCEND_SPEED_Y = 80.f; // Vertical speed during descent (pixels/sec), adjust!
	const float ASCEND_SPEED = 120.f; // Speed for flying up
	const float DESCEND_ZIGZAG_AMP = 50.f; // Horizontal amplitude of zigzag (pixels), adjust!
	const float DESCEND_ZIGZAG_FREQ = 2.f; // Frequency of zigzag (cycles per second), adjust!
	float descendZigzagAngle = 0.f;
	int flyDirection = 1; // 1 for right, -1 for left
	float descendStartX = 0.f; // X position when descent starts

	// Boss arena boundaries (relative to map start 0,0)
	const float ARENA_LEFT_X = 240.f * 16.f; // Kept same relative start? (256 - 16)
	const float ARENA_RIGHT_X = 256.f * 16.f; // Adjusted to actual map edge (was 272.f)

	// --- Attack Parameters & Timers ---
	vector<BossLeaf*> activeLeaves;


	const int SWIRL_LEAF_COUNT = 8;
	const float SWIRL_RADIUS = 6.f * 16.f; // Radius for outward swirl
	const float SWIRL_DURATION = 2.f; // Seconds for swirl animation
	const float BAMBOO_DROP_COOLDOWN = 0.85f; // Decreased cooldown for more drops (was 1.5f)
	const float GROUND_WAIT_DURATION = 2.0f; // Pause on ground before vanishing
	const float REAPPEAR_DURATION = 0.25f;
	const float DEATH_POSE_DURATION = 3.0f; // How long boss stays on ground after falling defeated

	float stateTimer = 0.f;
	float bambooDropTimer = 0.f;
	bool groundAttackTriggeredThisCycle = false;

	// --- Helper Methods (Private) ---
	void changeState(BossState newState);

	// Update logic for each state
	void updateInactive(float dt);
	void updateSpawning(float dt);
	void updateFlying(float dt);
	void updateDescending(float dt);
	void updateAttackingGround(float dt);
	void updateVanishing(float dt);
	void updateReappearingGround(float dt);
	void updateAscending(float dt);
	void updateFallingDefeated(float dt);
	void updateDyingGround(float dt);
	void updateDefeated(float dt);

	// Attack helpers
	// void spawnLeafSwirl(bool inward); // <-- Old name, remove or keep commented
	void triggerLeafSwirl(bool outward); // <-- Add declaration
	void updateLeafSwirl(float dt, bool inward); // <-- Keep or remove? Currently unused.
	void triggerBambooDrop();
	void triggerBambooRain();
	void clearLeaves();

	// Setup
	void setupAnimations();

};

#endif // _BOSS_INCLUDE
