#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include <vector>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"
#include "Snake.h"
#include "Bamboo.h"
#include "Tronco.h"
#include "Rana.h"
#include "GUI.h"
#include "Item.h"
#include "SmallHeart.h"
#include "Boss.h"
#include <glm/gtc/epsilon.hpp>


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();


	void init();
	void update(int deltaTime);
	void render(int framebufferWidth, int framebufferHeight);
	glm::vec2 getCameraPosition() { return cameraPosition; }  // Nuevo método para obtener la posición de la cámara

	// Public functions for Boss to call
	void spawnSingleBamboo(const glm::vec2& spawnPos);
	void spawnBambooRain(float leftBound, float rightBound, float spawnY);
	
	// State check methods
	bool bossDefeated() const { return bossActive && boss != nullptr && !boss->isAlive(); }
	Player* getPlayer() const { return player; }
	ShaderProgram& getShaderProgram(){ return texProgram; }

private:
	void updateBambusBoss(int deltaTime);
	void initShaders();
	void updateCamera();  // Nuevo método para actualizar la cámara
	void ini_pos_snakes();
	void ini_pos_osos();
	void ini_pos_ranas();
	void ini_pos_bambus();
	void ini_pos_troncos();
	void updateSnakes(int deltaTime);
	void updateOsos();
	void updateRanas(int deltaTime);
	void updateBambus(int deltaTime);
	void updateTroncos(int deltaTime);
	void updateItems(int deltaTime);
	void renderSnakes(glm::mat4 modelview);
	void renderOsos(glm::mat4 modelview);
	void renderRanas(glm::mat4 modelview);
	void renderBambus(glm::mat4 modelview);
	void renderTroncos(glm::mat4 modelview);
	void renderItems(glm::mat4& modelview);
	bool checkCollisionAABB(const glm::vec2& pos1, const glm::ivec2& size1, const glm::vec2& pos2, const glm::ivec2& size2) const;
	void checkCollisions();
	void checkTroncoCollisions();
	void checkItemCollisions();
	void spawnItem(const glm::vec2& position);
	void mirar_condicion_muerte();
	void checkGameEndConditions();

	glm::vec2 cameraPosition;  // Nueva variable para la posición de la cámara

	//dividiremos el mapa en sectores para que se vaya mostrando a trozos
	int sector_horizontal = 1;
	int sector_vertical = 1;

	//los puntos de checkpoint sera cuando cambiemos de sector vertical a sector horizontal
	float primer_checkpoint;
	float segundo_checkpoint;
	float tercer_checkpoint;
	float cuarto_checkpoint;
	float quinto_checkpoint;

	bool verticalPos = false;
	Player* player;
	TileMap* map;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	//variable para saber si estamos en el final y bloquear la camara en caso de que sea asi 
	bool final = false;

	//ENEMIGOS***********************************************************************************************************

	vector <glm::vec2> posiciones_snakes;
	//vector que marca que snakes ya han spawneado, si es true han spawneado y ya estan en pantalla o estan muertas (y el player no ha pasado por el trigger point otra vez)
	vector <bool> snakes_spawned;
	vector <bool> snakes_spawn_point_was_visible;
	vector <Snake*> snakes;
	int numero_snakes = 4;
	float despawn_distance;

	// Bambús
	vector<glm::vec2> posiciones_bambus;
	vector<Bamboo*> bambus;
	vector<bool> bambus_active;
	vector<float> bamboo_spawn_timers;
	float bamboo_spawn_delay = 2.0f; // Segundos entre spawn de bambús
	int numero_bambus = 9;

	vector <glm::vec2> posiciones_ranas;
	vector <bool> ranas_spawned;
	vector <bool> ranas_spawn_point_was_visible;
	vector <Rana*> ranas;
	int numero_ranas = 7;

	vector <glm::vec2> posiciones_osos;
	vector <bool> osos_spawned;
	int numero_osos = 0;

	// Troncos (plataformas móviles)
	vector<glm::vec2> posiciones_troncos;
	vector<Tronco*> troncos;
	int numero_troncos = 8;
	bool player_on_tronco;
	int current_tronco_index;

	GUI* gui;

	// Items
	std::vector<Item*> activeItems;

	// Boss related members
	Boss* boss = nullptr;
	vector<Bamboo*> bossBamboos; // Separate vector for boss-spawned bamboos
	bool bossActive = false; // Flag to know if boss fight has started
	
public:
	// Make bossActive available to Game for checking
	bool isBossActive() const { return bossActive; }
};


#endif // _SCENE_INCLUDE

