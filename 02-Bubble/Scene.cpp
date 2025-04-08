#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h> // Necesario para glViewport
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()
#include <algorithm> // Para std::remove_if
#include "Scene.h"
#include "Game.h"
#include "GUI.h"
#include "Item.h"        
#include "SmallHeart.h"  
#include "BigHeart.h"    // <-- INCLUIR BIGHEART
#include "Helmet.h"      // <-- INCLUIR HELMET
#include "Bisonte.h"     // <-- INCLUIR BISONTE
#include "Calabaza.h"    // <-- INCLUIR CALABAZA
#include <glm/gtc/epsilon.hpp>
#include "Boss.h" // Add Boss include


#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 5
#define TILESIZE 16

//para que se vea cuadrado necesitamos manetener la proporcion que tiene la screen en el main
#define CAMERA_WIDTH 16*TILESIZE
#define CAMERA_HEIGHT 15*TILESIZE
#define CAMERA_FOLLOW_THRESHOLD 2.0f / 3.0f


Scene::Scene()
{
	map = NULL;
	player = NULL;
	gui = NULL; // Inicializar gui a nullptr
	// Asegurarse que el vector de items esté vacío
	activeItems.clear();
	boss = nullptr; // Initialize boss pointer
	bossActive = false; // Initialize boss state
	bossBamboos.clear(); // Ensure boss bamboo vector is empty
}

Scene::~Scene()
{
	if (map != NULL)
		delete map;
	if (player != NULL)
		delete player;
	if (gui != NULL)
		delete gui;

	for (int i = 0; i < numero_snakes; ++i) {
		if (snakes[i] != nullptr) {
			delete snakes[i];
			snakes[i] = nullptr;
		}
	}

	// Liberar memoria de los bambús
	for (int i = 0; i < numero_bambus; ++i) {
		if (bambus[i] != nullptr) {
			delete bambus[i];
			bambus[i] = nullptr;
		}
	}

	// Liberar memoria de los troncos
	for (int i = 0; i < numero_troncos; ++i) {
		if (troncos[i] != nullptr) {
			delete troncos[i];
			troncos[i] = nullptr;
		}
	}

	// Liberar memoria de las ranas
	for (int i = 0; i < numero_ranas; ++i) {
		if (ranas[i] != nullptr) {
			delete ranas[i];
			ranas[i] = nullptr;
		}
	}

	// Liberar memoria de los items activos
	for (Item* item : activeItems) {
		if (item != nullptr) {
			delete item;
		}
	}
	activeItems.clear();

	// Clean up boss
	if (boss != nullptr) {
		delete boss;
		boss = nullptr;
	}
	// Clean up boss bamboos
	for (Bamboo* bamboo : bossBamboos) {
		if (bamboo != nullptr) {
			delete bamboo;
		}
	}
	bossBamboos.clear();
}

void Scene::init()
{
	srand(time(NULL)); // Inicializar generador de números aleatorios
	despawn_distance = 2.0f * CAMERA_WIDTH;
	initShaders();
	map = TileMap::createTileMap("levels/level02.txt", glm::vec2(0, 0), texProgram);
	player = new Player();
	player->init(glm::ivec2(0, 0), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);

	projection = glm::ortho(0.f, float(CAMERA_WIDTH), float(CAMERA_HEIGHT), 0.f);
	currentTime = 0.0f;
	cameraPosition = glm::vec2(0.0f);

	// <-- INICIALIZAR GUI -->
	gui = new GUI();
	gui->init(&texProgram, projection, CAMERA_WIDTH, CAMERA_HEIGHT);

	//calculamos el numero de tiles donde queremos que cambiemos
	primer_checkpoint = 144.f * TILESIZE - CAMERA_WIDTH;
	segundo_checkpoint = primer_checkpoint + CAMERA_WIDTH;
	tercer_checkpoint = segundo_checkpoint + 48.f * TILESIZE;
	cuarto_checkpoint = tercer_checkpoint + CAMERA_WIDTH;
	quinto_checkpoint = map->getMapWidth() * map->getTileSize() - CAMERA_WIDTH;
	ini_pos_snakes();
	ini_pos_osos();
	ini_pos_ranas();
	ini_pos_bambus();
	ini_pos_troncos();
}
void Scene::ini_pos_snakes() {
	posiciones_snakes.resize(numero_snakes);
	snakes.resize(numero_snakes, nullptr);
	snakes_spawned.resize(numero_snakes, false);
	snakes_spawn_point_was_visible.resize(numero_snakes, false);

	for (int i = 0; i < numero_snakes; ++i) {
		posiciones_snakes[i].y = 95 * TILESIZE;
		// snakes[i] = nullptr; // Ya se inicializa en resize
		// snakes_spawned[i] = false; // Ya se inicializa en resize
	}

	posiciones_snakes[0].x = (14 * TILESIZE) + segundo_checkpoint;
	posiciones_snakes[1].x = (20 * TILESIZE) + segundo_checkpoint;
	posiciones_snakes[2].x = (26 * TILESIZE) + segundo_checkpoint;
	posiciones_snakes[3].x = (34 * TILESIZE) + segundo_checkpoint;
}
void Scene::ini_pos_osos() {
	return;
}
void Scene::ini_pos_ranas() {
	numero_ranas = 7; // Asegurarse de que sea 7
	posiciones_ranas.resize(numero_ranas);
	ranas.resize(numero_ranas, nullptr); // Inicializar vector de punteros
	ranas_spawned.resize(numero_ranas, false); // Inicializar estado de spawn
	ranas_spawn_point_was_visible.resize(numero_ranas, false);

	// Sección 1 (Inicio -> Checkpoint 1)
	posiciones_ranas[0] = glm::vec2(18 * TILESIZE, 4 * TILESIZE);
	posiciones_ranas[1] = glm::vec2(31 * TILESIZE, 4 * TILESIZE);
	posiciones_ranas[2] = glm::vec2(46 * TILESIZE, 0 * TILESIZE);
	posiciones_ranas[3] = glm::vec2(56 * TILESIZE, 0 * TILESIZE);

	// Sección 3 (Checkpoint 3 -> Checkpoint 4)
	posiciones_ranas[4] = glm::vec2(198 * TILESIZE, 72 * TILESIZE);
	posiciones_ranas[5] = glm::vec2(203 * TILESIZE, 61 * TILESIZE);
	posiciones_ranas[6] = glm::vec2(201 * TILESIZE, 56 * TILESIZE);

}
void Scene::ini_pos_bambus() {
	posiciones_bambus.resize(numero_bambus);
	bambus.resize(numero_bambus, nullptr);
	bambus_active.resize(numero_bambus, false);
	bamboo_spawn_timers.resize(numero_bambus, 0.0f); // <-- RESTAURAR ESTA LÍNEA

	// Posiciones X específicas para el spawn de bambús
	posiciones_bambus[0].x = 70 * TILESIZE;
	posiciones_bambus[1].x = 81 * TILESIZE;
	posiciones_bambus[2].x = 83 * TILESIZE;
	posiciones_bambus[3].x = 90 * TILESIZE;
	posiciones_bambus[4].x = 95 * TILESIZE;
	posiciones_bambus[5].x = 120 * TILESIZE;
	posiciones_bambus[6].x = 123 * TILESIZE;
	posiciones_bambus[7].x = 209 * TILESIZE;
	posiciones_bambus[8].x = 212 * TILESIZE;

	// Altura inicial de spawn (y = 0 significa la parte superior del mapa)
	for (int i = 0; i < numero_bambus; ++i) {
		posiciones_bambus[i].y = 0;
		// bambus[i] = nullptr; // Ya se inicializa en resize
	}

	std::cout << "Initialized " << numero_bambus << " bamboo positions" << std::endl;
}

void Scene::ini_pos_troncos() {
	// Ensure vectors are resized according to numero_troncos (should be 8)
	posiciones_troncos.resize(numero_troncos);
	troncos.resize(numero_troncos, nullptr);

	// Define initial positions for the 8 troncos for VERTICAL ASCENT
	// Player needs to reach the first one from around Y=53*TILESIZE
	// Horizontal range approx X = 196*TILESIZE to 204*TILESIZE
	posiciones_troncos[0] = glm::vec2(198 * TILESIZE, 51 * TILESIZE); // Start
	posiciones_troncos[1] = glm::vec2(202 * TILESIZE, 48 * TILESIZE); // Up and Left
	posiciones_troncos[2] = glm::vec2(197 * TILESIZE, 45 * TILESIZE); // Up and Right
	posiciones_troncos[3] = glm::vec2(199 * TILESIZE, 43 * TILESIZE); // Up and Left
	posiciones_troncos[4] = glm::vec2(198 * TILESIZE, 30 * TILESIZE); // Up and Right
	posiciones_troncos[5] = glm::vec2(202 * TILESIZE, 41 * TILESIZE); // Up and Left
	posiciones_troncos[6] = glm::vec2(197 * TILESIZE, 36 * TILESIZE); // Up and Right
	posiciones_troncos[7] = glm::vec2(197 * TILESIZE, 40 * TILESIZE); // Up and Left (Highest)

	// Variables para el seguimiento del jugador sobre los troncos (kept for potential future use)
	player_on_tronco = false;
	current_tronco_index = -1;

	// Crear los troncos con las nuevas posiciones
	for (int i = 0; i < numero_troncos; ++i) {
		troncos[i] = new Tronco();
		troncos[i]->init(posiciones_troncos[i], texProgram);
		troncos[i]->setTileMap(map);
	}
}
void Scene::updateCamera()
{
	glm::vec2 playerPos = player->getPosition();
	if (playerPos.x == primer_checkpoint) {
		//hace falta limpiar todos los enemigos !!!!!!!!!!

		verticalPos = true;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = primer_checkpoint;
	}
	if (playerPos.x == segundo_checkpoint) {
		//hace falta limpiar todos los enemigos !!!!!!!!!!


		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = segundo_checkpoint;
		cameraPosition.y = 90 * TILESIZE;
	}
	if (playerPos.x == tercer_checkpoint) {
		//hace falta limpiar todos los enemigos !!!!!!!!!!

		verticalPos = true;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = tercer_checkpoint;

	}

	if (playerPos.x == cuarto_checkpoint) {
		//hace falta limpiar todos los enemigos !!!!!!!!!!

		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = cuarto_checkpoint;
		cameraPosition.y = 30 * TILESIZE;
	}

	if (playerPos.x == quinto_checkpoint) {
		//hace falta limpiar todos los enemigos !!!!!!!!!!

		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = quinto_checkpoint;
		final = true;
	}

	if (verticalPos) {
		if (playerPos.y > cameraPosition.y + (CAMERA_HEIGHT * 0.5f)) {
			cameraPosition.y = playerPos.y - (CAMERA_HEIGHT * 0.5f);
		}
		else if (playerPos.y < cameraPosition.y + (CAMERA_HEIGHT * 0.5f)) {
			cameraPosition.y = playerPos.y - (CAMERA_HEIGHT * 0.5f);
		}
	}
	// Comportamiento normal antes de x = 2080
	else {
		// Seguimiento horizontal normal
		if (!final) {
			if (playerPos.x > cameraPosition.x + (CAMERA_WIDTH * 0.5f)) {
				cameraPosition.x = playerPos.x - (CAMERA_WIDTH * 0.5f);
			}
			else if (playerPos.x < cameraPosition.x + (CAMERA_WIDTH * 0.5f)) {
				cameraPosition.x = playerPos.x - (CAMERA_WIDTH * 0.5f);
			}
		}
	}

	cameraPosition.x = glm::max(0.0f, cameraPosition.x);
	if (verticalPos && cameraPosition.x == tercer_checkpoint) {
		float maxYThirdCheckpoint = 30 * TILESIZE;
		cameraPosition.y = glm::max(cameraPosition.y, maxYThirdCheckpoint);
	}
	else cameraPosition.y = glm::max(0.0f, cameraPosition.y);


	float maxX = float(map->getMapWidth() * map->getTileSize() - CAMERA_WIDTH);

	float maxY = float(map->getMapHeight() * map->getTileSize() - CAMERA_HEIGHT);

	cameraPosition.x = glm::min(cameraPosition.x, maxX);

	cameraPosition.y = glm::min(cameraPosition.y, maxY);

}


void Scene::mirar_condicion_muerte(){
	if (bossActive && !boss->isAlive()) {
		player->setVictory();
		gui->updateFinal(true);
	}
}

void Scene::update(int deltaTime)
{
	 currentTime += deltaTime;

   if (bossActive && boss != nullptr) {
     updateBambusBoss(deltaTime); // Actualiza el jefe y sus bambús
 }

 mirar_condicion_muerte();
 player->update(deltaTime, troncos);
 updateCamera();
 updateTroncos(deltaTime);        
 updateSnakes(deltaTime);
 updateBambus(deltaTime); 
 updateRanas(deltaTime);
 updateItems(deltaTime);
 checkCollisions();       
 checkItemCollisions();   
 // 7. Actualizar GUI (con el estado final del jugador y jefe)
 if (player && gui) {
     gui->update(player->getCurrentHealth(),
         player->getMaxHealth(),
         player->getClockCount(),
         player->getHasFlint(),
         player->getHasHelmet(),
         player->isInFireMode(),
		 bossActive && boss != nullptr,
		 bossActive && boss != nullptr ? boss->getCurrentHealth() : 0
     );
 }

 if (!bossActive && player && player->getPosition().x >= quinto_checkpoint)
 {
     bossActive = true;
     final = true; // Lock camera
     std::cout << "Player reached boss arena! Activating boss..." << std::endl;
     glm::ivec2 bossSpawnPos = glm::ivec2(248 * TILESIZE, 39 * TILESIZE);
     if (boss == nullptr) {
         boss = new Boss();
         boss->init(bossSpawnPos, texProgram, player, map, this);
     }
 }
	}
	// --- End Boss Activation & Update ---

	//updateOsos();
	//updateRanas();
void Scene::updateBambusBoss(int deltaTime) {
	if (bossActive && boss != nullptr) {
		boss->update(deltaTime);

		// Update boss-spawned bamboos
		for (Bamboo* bamboo : bossBamboos) {
			if (bamboo != nullptr && !bamboo->shouldBeRemoved()) { // Solo actualiza si no está marcado
				bamboo->update(deltaTime);
				// Opcional: Añadir aquí lógica de despawn por visibilidad para bossBamboos
				// if (bamboo está fuera de cámara) { bamboo->markForRemoval(); }
			}
		}

		// --- AÑADIR LIMPIEZA DE BAMBÚS DEL JEFE ---
		// Eliminar bambús del jefe marcados para borrado usando remove-erase idiom
		bossBamboos.erase(
			std::remove_if(bossBamboos.begin(), bossBamboos.end(),
				[](Bamboo* bamboo) {
					if (bamboo != nullptr && bamboo->shouldBeRemoved()) {
						std::cout << "Limpiando bambú del jefe marcado." << std::endl;
						delete bamboo; // Liberar memoria
						return true;  // Indicar que debe ser eliminado del vector
					}
					return false; // Mantener el bambú
				}),
			bossBamboos.end()
		);
		// --- FIN LIMPIEZA ---

	}

}

void Scene::updateSnakes(int deltaTime) {
	glm::vec2 playerPos = player->getPosition(); // Necesario para la condición de zona

	// --- Parte 1: Actualizar y Despawnear Serpientes Existentes --- 
	for (int i = 0; i < numero_snakes; ++i) {
		if (snakes_spawned[i] && snakes[i] != nullptr) {
			// 1. Actualizar serpiente
			snakes[i]->update(deltaTime);

			// 2. Comprobar si murió ESTE frame
			if (!snakes[i]->isAlive()) {
				std::cout << "Snake " << i << " murió. Intentando dropear item..." << std::endl;
				spawnItem(snakes[i]->getPosition()); // Spawn item
				// Borrar la serpiente muerta
				delete snakes[i];
				snakes[i] = nullptr;
				snakes_spawned[i] = false;
				// No resetear snakes_spawn_point_was_visible[i] aquí.
				continue; // Pasar a la siguiente serpiente
			}

			// 3. Si sigue viva, comprobar despawn por visibilidad
			glm::vec2 snakePos = snakes[i]->getPosition();
			glm::ivec2 snakeSize = snakes[i]->getHitboxSize();
			float cameraLeft = cameraPosition.x;
			float cameraRight = cameraPosition.x + CAMERA_WIDTH;
			float cameraTop = cameraPosition.y;
			float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;
			float snakeLeft = snakePos.x;
			float snakeRight = snakePos.x + snakeSize.x;
			float snakeTop = snakePos.y;
			float snakeBottom = snakePos.y + snakeSize.y;

			bool shouldDespawnDueToVisibility = (snakeRight < cameraLeft || snakeLeft > cameraRight || snakeBottom < cameraTop || snakeTop > cameraBottom);

			if (shouldDespawnDueToVisibility) {
				// Serpiente viva pero fuera de pantalla
				std::cout << "Despawning snake " << i << " (Fuera de vista)" << std::endl;
				delete snakes[i];
				snakes[i] = nullptr;
				snakes_spawned[i] = false;
				// No resetear snakes_spawn_point_was_visible[i] aquí.
			}
		}
	}

	// --- Parte 2: Comprobar Condiciones de Spawn/Respawn --- 
	for (int i = 0; i < numero_snakes; ++i) {

		// Comprobar si el PUNTO DE SPAWN está visible actualmente
		glm::vec2 spawnPoint = posiciones_snakes[i];
		bool is_currently_visible = false;
		float cameraLeft = cameraPosition.x;
		float cameraRight = cameraPosition.x + CAMERA_WIDTH;
		float cameraTop = cameraPosition.y;
		float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;

		if (spawnPoint.x >= cameraLeft && spawnPoint.x < cameraRight &&
			spawnPoint.y >= cameraTop && spawnPoint.y < cameraBottom)
		{
			is_currently_visible = true;
		}

		// Condición de Spawn/Respawn específica para serpientes
		// Deben estar en la zona correcta Y el punto debe entrar en visión.
		bool inSpawnZone = (playerPos.x > segundo_checkpoint && playerPos.x < tercer_checkpoint);

		if (!snakes_spawned[i] && inSpawnZone && is_currently_visible && !snakes_spawn_point_was_visible[i])
		{
			std::cout << "Spawning snake " << i << " at point (" << spawnPoint.x << ", " << spawnPoint.y << ")" << std::endl;
			snakes[i] = new Snake();
			snakes[i]->init(posiciones_snakes[i], texProgram, player);
			snakes[i]->setTileMap(map);
			snakes_spawned[i] = true;
		}

		// Actualizar el estado de visibilidad del punto para el próximo frame
		snakes_spawn_point_was_visible[i] = is_currently_visible;
	}
}
void Scene::updateBambus(int deltaTime) {
	glm::vec2 playerPos = player->getPosition(); // Necesario para el spawn por distancia
	float deltaTimeSeconds = deltaTime / 1000.0f; // Necesario para el timer

	// --- Parte 1: Actualizar y Despawnear Bambús Existentes --- 
	for (int i = 0; i < numero_bambus; ++i) {
		if (bambus_active[i] && bambus[i] != nullptr) {
			bambus[i]->update(deltaTime);

			// Lógica de Despawn por Visibilidad (la que ya teníamos)
			bool shouldDespawn = false;
			if (!bambus[i]->isAlive()) {
				shouldDespawn = true;
			}
			else {
				glm::vec2 bambooPos = bambus[i]->getPosition();
				glm::ivec2 bambooSize = bambus[i]->getHitboxSize();
				float cameraLeft = cameraPosition.x;
				float cameraRight = cameraPosition.x + CAMERA_WIDTH;
				float cameraTop = cameraPosition.y;
				float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;
				float bambooLeft = bambooPos.x;
				float bambooRight = bambooPos.x + bambooSize.x;
				float bambooTop = bambooPos.y;
				float bambooBottom = bambooPos.y + bambooSize.y;
				if (bambooRight < cameraLeft || bambooLeft > cameraRight || bambooBottom < cameraTop || bambooTop > cameraBottom) {
					shouldDespawn = true;
				}
			}

			if (shouldDespawn) {
				std::cout << "Despawning bamboo " << i << " (Fuera de vista o inactivo)" << std::endl;
				delete bambus[i];
				bambus[i] = nullptr;
				bambus_active[i] = false;
				// Resetear timer cuando despawnea (como estaba antes)
				bamboo_spawn_timers[i] = 0.0f;
			}
		}
		// --- Parte 2: Lógica de Spawn Anterior (basada en timer y distancia) --- 
		else { // Si el bambú no está activo
			bamboo_spawn_timers[i] += deltaTimeSeconds;

			// Verificar si es hora de spawnear un nuevo bambú
			if (bamboo_spawn_timers[i] >= bamboo_spawn_delay) {
				// Verificar si el jugador está lo suficientemente cerca para spawnearlo
				float distanceToSpawnPoint = glm::abs(playerPos.x - posiciones_bambus[i].x);

				// Usar despawn_distance como umbral de spawn
				if (distanceToSpawnPoint <= despawn_distance) {
					// Crear nuevo bambú
					bambus[i] = new Bamboo();
					bambus[i]->init(posiciones_bambus[i], texProgram);
					bambus[i]->setTileMap(map);
					bambus_active[i] = true;
					std::cout << "Spawned bamboo " << i << " at position " << posiciones_bambus[i].x << ", " << posiciones_bambus[i].y << std::endl;
				}

				// Reiniciar el timer aunque no se haya spawneado
				bamboo_spawn_timers[i] = 0.0f;
			}
		}
	}
}
void Scene::updateTroncos(int deltaTime) {
	for (int i = 0; i < numero_troncos; ++i) {
		if (troncos[i] != nullptr) {
			troncos[i]->update(deltaTime);
		}
	}

	// Si el jugador está sobre un tronco, hay que actualizar su posición
	// Esto se maneja en checkTroncoCollisions
}

void Scene::updateRanas(int deltaTime) {

	// --- Parte 1: Actualizar y Despawnear Ranas Existentes --- 
	for (int i = 0; i < numero_ranas; ++i) {
		if (ranas_spawned[i] && ranas[i] != nullptr) {
			// 1. Actualizar rana
			ranas[i]->update(deltaTime);

			// 2. Comprobar si murió ESTE frame
			if (!ranas[i]->isAlive()) {
				std::cout << "Rana " << i << " murió. Intentando dropear item..." << std::endl;
				spawnItem(ranas[i]->getPosition()); // Spawn item
				// Borrar la rana muerta
				delete ranas[i];
				ranas[i] = nullptr;
				ranas_spawned[i] = false;
				continue; // Pasar a la siguiente rana
			}

			// 3. Si sigue viva, comprobar despawn por visibilidad
			glm::vec2 ranaPos = ranas[i]->getPosition();
			glm::ivec2 ranaSize = ranas[i]->getHitboxSize();
			float cameraLeft = cameraPosition.x;
			float cameraRight = cameraPosition.x + CAMERA_WIDTH;
			float cameraTop = cameraPosition.y;
			float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;
			float ranaLeft = ranaPos.x;
			float ranaRight = ranaPos.x + ranaSize.x;
			float ranaTop = ranaPos.y;
			float ranaBottom = ranaPos.y + ranaSize.y;

			bool shouldDespawnDueToVisibility = (ranaRight < cameraLeft || ranaLeft > cameraRight || ranaBottom < cameraTop || ranaTop > cameraBottom);

			if (shouldDespawnDueToVisibility) {
				// Rana viva pero fuera de pantalla
				std::cout << "Despawning rana " << i << " (Fuera de vista)" << std::endl;
				delete ranas[i];
				ranas[i] = nullptr;
				ranas_spawned[i] = false;
			}
		}
	}

	// --- Parte 2: Comprobar Condiciones de Spawn/Respawn --- 
	for (int i = 0; i < numero_ranas; ++i) {
		glm::vec2 spawnPoint = posiciones_ranas[i];
		bool is_currently_visible = false;
		float cameraLeft = cameraPosition.x;
		float cameraRight = cameraPosition.x + CAMERA_WIDTH;
		float cameraTop = cameraPosition.y;
		float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;

		if (spawnPoint.x >= cameraLeft && spawnPoint.x < cameraRight &&
			spawnPoint.y >= cameraTop && spawnPoint.y < cameraBottom)
		{
			is_currently_visible = true;
		}

		if (!ranas_spawned[i] && is_currently_visible && !ranas_spawn_point_was_visible[i])
		{
			std::cout << "Spawning rana " << i << " at point (" << spawnPoint.x << ", " << spawnPoint.y << ")" << std::endl;
			ranas[i] = new Rana();
			ranas[i]->init(posiciones_ranas[i], texProgram, player);
			ranas[i]->setTileMap(map);
			ranas_spawned[i] = true;
		}

		ranas_spawn_point_was_visible[i] = is_currently_visible;
	}
}

// --- NUEVOS MÉTODOS PARA ITEMS ---

void Scene::spawnItem(const glm::vec2& position) {
	const int DROP_CHANCE_PERCENT = 50;
	int chance = rand() % 100;

	if (chance < DROP_CHANCE_PERCENT) {
		// Decidir QUÉ item dropear
		// --- INICIO Restaurar Lógica Aleatoria con Nuevas Probabilidades ---

		const int CALABAZA_CHANCE = 40;  // 40%
		const int SMALL_HEART_CHANCE = 30; // 30% (acumulado 70%)
		const int BIG_HEART_CHANCE = 10; // 10% (acumulado 80%)
		const int HELMET_CHANCE = 10;    // 10% (acumulado 90%)
		// El resto (10%) será Bisonte

		int itemRoll = rand() % 100;
		Item* newItem = nullptr;

		if (itemRoll < CALABAZA_CHANCE) { // 0-39
			std::cout << "Intentando spawnear Calabaza..." << std::endl;
			newItem = new Calabaza(&texProgram);
		}
		else if (itemRoll < CALABAZA_CHANCE + SMALL_HEART_CHANCE) { // 40-69
			std::cout << "Intentando spawnear SmallHeart..." << std::endl;
			newItem = new SmallHeart(&texProgram);
		}
		else if (itemRoll < CALABAZA_CHANCE + SMALL_HEART_CHANCE + BIG_HEART_CHANCE) { // 70-79
			std::cout << "Intentando spawnear BigHeart..." << std::endl;
			newItem = new BigHeart(&texProgram);
		}
		else if (itemRoll < CALABAZA_CHANCE + SMALL_HEART_CHANCE + BIG_HEART_CHANCE + HELMET_CHANCE) { // 80-89
			std::cout << "Intentando spawnear Helmet..." << std::endl;
			newItem = new Helmet(&texProgram);
		}
		else { // 90-99
			std::cout << "Intentando spawnear Bisonte..." << std::endl;
			newItem = new Bisonte(&texProgram);
		}

		// Eliminar el código forzado que estaba aquí antes
		// std::cout << "FORZADO: Intentando spawnear Calabaza..." << std::endl;
		// Item* newItem = new Calabaza(&texProgram);
		// --- FIN Restaurar Lógica Aleatoria ---

		if (newItem) {
			newItem->init(position);
			if (!newItem->shouldBeRemoved()) {
				newItem->setTileMap(map);
				activeItems.push_back(newItem);
				std::cout << "Item añadido a activeItems (Tipo: " << int(newItem->getType()) << ")" << std::endl;
			}
			else {
				std::cout << "ERROR: Item::init() marcó el item para borrado, no se añadió." << std::endl;
				delete newItem;
			}
		}
		else {
			std::cout << "ERROR: new Item() devolvió nullptr!" << std::endl;
		}
	}
}

void Scene::updateItems(int deltaTime) {
	// Actualizar todos los items activos
	for (Item* item : activeItems) {
		if (item == nullptr) continue; // Saltar punteros nulos

		// ---> INICIO: Comprobación de Despawn por Visibilidad <---
		glm::vec2 itemPos = item->getPosition();
		glm::ivec2 itemSize = item->getHitboxSize();

		float cameraLeft = cameraPosition.x;
		float cameraRight = cameraPosition.x + CAMERA_WIDTH;
		float cameraTop = cameraPosition.y;
		float cameraBottom = cameraPosition.y + CAMERA_HEIGHT;

		float itemLeft = itemPos.x;
		float itemRight = itemPos.x + itemSize.x;
		float itemTop = itemPos.y;
		float itemBottom = itemPos.y + itemSize.y;

		// Comprobar si está COMPLETAMENTE fuera de la cámara
		if (itemRight < cameraLeft || itemLeft > cameraRight || itemBottom < cameraTop || itemTop > cameraBottom) {
			std::cout << "Item (Tipo: " << int(item->getType()) << ") fuera de cámara. Marcando para borrado." << std::endl;
			item->markForRemoval();
			// No necesitamos 'continue' aquí porque la lógica de borrado 
			// ya comprueba shouldBeRemoved() más abajo.
		}
		// ---> FIN: Comprobación de Despawn por Visibilidad <---

		// Solo actualizar si no está marcado para borrar (aunque el chequeo anterior ya lo haría)
		if (!item->shouldBeRemoved()) {
			item->update(deltaTime);
		}
	}

	// Eliminar items marcados para borrado usando remove-erase idiom
	activeItems.erase(
		std::remove_if(activeItems.begin(), activeItems.end(),
			[](Item* item) {
				if (item != nullptr && item->shouldBeRemoved()) {
					delete item; // Liberar memoria
					return true; // Indicar que debe ser eliminado del vector
				}
				return false; // Mantener el item
			}),
		activeItems.end()
	);
}

void Scene::checkItemCollisions() {
	if (!player || !player->isAlive()) return; // No comprobar si no hay jugador o está muerto

	glm::vec2 playerPos = player->getPosition();
	glm::ivec2 playerSize(32, 32); // Usar la hitbox correcta del jugador

	// Usamos un índice para poder modificar el vector si un item se recoge
	for (size_t i = 0; i < activeItems.size(); ++i) {
		Item* item = activeItems[i];
		// Saltar items nulos, ya marcados, O QUE AÚN NO ESTÉN ACTIVOS
		if (item == nullptr || item->shouldBeRemoved() || !item->isActive()) continue;

		glm::vec2 itemPos = item->getHitboxPosition();
		glm::ivec2 itemSize = item->getHitboxSize();

		if (checkCollisionAABB(playerPos, playerSize, itemPos, itemSize)) {
			item->applyEffect(player); // Aplicar efecto (marcará el item para borrado)
			// No necesitamos borrarlo aquí, updateItems se encargará
		}
	}
}

void Scene::render(int framebufferWidth, int framebufferHeight)
{
	glm::mat4 modelview;

	// --- Cálculo y aplicación del Viewport para Pillarboxing --- 
	// Use the dynamically obtained framebuffer dimensions

	float targetAspectRatio = float(CAMERA_WIDTH) / float(CAMERA_HEIGHT); // 16:15
	// Calculate aspect ratio of the current framebuffer
	float windowAspectRatio = (framebufferHeight > 0) ? float(framebufferWidth) / float(framebufferHeight) : targetAspectRatio;

	int vp_x, vp_y, vp_width, vp_height;

	if (windowAspectRatio >= targetAspectRatio) {
		// Window wider than target (Pillarboxing)
		vp_height = framebufferHeight;
		vp_width = int(vp_height * targetAspectRatio);
		vp_x = (framebufferWidth - vp_width) / 2;
		vp_y = 0;
	}
	else {
		// Window taller than target (Letterboxing)
		vp_width = framebufferWidth;
		vp_height = int(vp_width / targetAspectRatio);
		vp_x = 0;
		vp_y = (framebufferHeight - vp_height) / 2;
	}

	// Check for zero dimensions to avoid issues with glViewport
	if (vp_width <= 0 || vp_height <= 0) {
		// Fallback or handle error, maybe use default if possible
		// For now, just use the full framebuffer to avoid glError
		vp_x = 0; vp_y = 0; vp_width = framebufferWidth; vp_height = framebufferHeight;
		if (vp_width <= 0) vp_width = 1; // Ensure non-zero
		if (vp_height <= 0) vp_height = 1; // Ensure non-zero
	}

	// Aplicar el viewport calculado
	glViewport(vp_x, vp_y, vp_width, vp_height);
	// -----------------------------------------------------------

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// Aplicamos la transformación de la cámara
	modelview = glm::mat4(1.0f);
	modelview = glm::translate(modelview, glm::vec3(-cameraPosition.x, -cameraPosition.y, 0.f));
	texProgram.setUniformMatrix4f("modelview", modelview);

	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();

	// Renderizar troncos antes del jugador para que el jugador aparezca encima
	renderTroncos(modelview);
	renderBambus(modelview);
	renderSnakes(modelview);
	renderRanas(modelview);
	renderItems(modelview); // <-- RENDERIZAR ITEMS

	player->render(modelview);

	// Render Boss if active
	if (bossActive && boss != nullptr) {
		boss->render(modelview);
		// Render boss bamboos
		for (Bamboo* bamboo : bossBamboos) {
			if (bamboo != nullptr) {
				bamboo->render(modelview);
			}
		}
	}

	// <-- RENDERIZAR GUI AL FINAL -->
	if (gui) {
		gui->render();
	}
}

void Scene::renderSnakes(glm::mat4 modelview) {
	for (int i = 0; i < numero_snakes; ++i) {
		if (snakes_spawned[i] && snakes[i] != nullptr) {
			snakes[i]->render(modelview);
		}
	}
}
void Scene::renderBambus(glm::mat4 modelview) {
	for (int i = 0; i < numero_bambus; ++i) {
		if (bambus_active[i] && bambus[i] != nullptr) {
			bambus[i]->render(modelview);
		}
	}
}
void Scene::renderTroncos(glm::mat4 modelview) {
	for (int i = 0; i < numero_troncos; ++i) {
		if (troncos[i] != nullptr) {
			troncos[i]->render(modelview);
		}
	}
}
void Scene::renderRanas(glm::mat4 modelview) {
	for (int i = 0; i < numero_ranas; ++i) {
		if (ranas_spawned[i] && ranas[i] != nullptr) {
			ranas[i]->render(modelview);
		}
	}
}
void Scene::renderItems(glm::mat4& modelview) {
	for (Item* item : activeItems) {
		if (item != nullptr) {
			item->render(modelview);
		}
	}
}
void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if (!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if (!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if (!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::checkCollisions()
{
	// Solo comprobar colisiones si el jugador no está muerto
if (!player->isAlive()) {
    return;
}

glm::vec2 playerPos = player->getPosition();
glm::vec2 lanzaPos = player->getPositionLanza();
// Obtener los proyectiles del jugador
vector<FireStickProjectile*>& projectiles = player->getProjectiles();

// ¡¡IMPORTANTE!! Usar el tamaño correcto de la hitbox del jugador si es diferente a 32x32
glm::ivec2 playerSize(32, 32);

// Detección de colisiones con serpientes
for (int i = 0; i < numero_snakes; ++i) {
    if (snakes_spawned[i] && snakes[i] != nullptr && snakes[i]->isAlive()) { // Comprobar también si la serpiente está viva
        glm::vec2 snakePos = snakes[i]->getHitboxPosition();
        glm::ivec2 snakeSize = snakes[i]->getHitboxSize();

        // Colisión jugador - serpiente
        if (checkCollisionAABB(playerPos, playerSize, snakePos, snakeSize)) {
            player->takeDamage(snakes[i]->getDamage());
            if (!snakes[i]->isAlive()) {
                delete snakes[i];
                snakes[i] = nullptr; // Eliminar la serpiente
            }
            break; // Salimos del bucle de serpientes después de la colisión con el jugador
        }

        // Colisión ataque del jugador - serpiente
        if (player->estaAttacking()) {
            if (checkCollisionAABB(lanzaPos, playerSize, snakePos, snakeSize)) {
                snakes[i]->takeDamage(player->getDamage());
                if (!snakes[i]->isAlive()) {
                    delete snakes[i];
                    snakes[i] = nullptr; // Eliminar la serpiente
                    break; // Salimos del bucle de serpientes después de matar a una
                }
            }
        }

        // Colisión proyectil - serpiente
        for (auto it = projectiles.begin(); it != projectiles.end(); ) {
            FireStickProjectile* proj = *it;
            if (proj->isActive()) {
                glm::vec2 projPos = proj->getPosition();
                glm::ivec2 projSize = proj->getSize();

                if (checkCollisionAABB(projPos, projSize, snakePos, snakeSize)) {
                    // El proyectil impactó con la serpiente
                    std::cout << "Serpiente eliminada por proyectil de fuego." << std::endl;
                    delete snakes[i];
                    snakes[i] = nullptr;
                    snakes_spawned[i] = false; // Marcar como no spawn
                    proj->deactivate(); // Desactivar el proyectil
                }
            }
            ++it;
        }
    }
}
	// Detección de colisiones con bambús
	if (player->isAlive()) { // Verificamos de nuevo por si murió por una serpiente
		for (int i = 0; i < numero_bambus; ++i) {
			if (bambus_active[i] && bambus[i] != nullptr && bambus[i]->isAlive()) { // Comprobar si bambú está vivo
				glm::vec2 bambooPos = bambus[i]->getHitboxPosition();
				glm::ivec2 bambooSize = bambus[i]->getHitboxSize();

				if (checkCollisionAABB(playerPos, playerSize, bambooPos, bambooSize)) {
					if (!player->getProteccionSuperior()) {
						player->takeDamage(bambus[i]->getDamage());
						}
					delete bambus[i];
					bambus[i] = nullptr;
					bambus_active[i] = false;
					break; // Salimos del bucle de bambús
				}
			}
		}
	}

	// ---> INICIO: Detección de colisiones con Tiles Dañinas (23 y 42) por DEBAJO <--- 
	if (player->isAlive()) { // Comprobar de nuevo por si murió antes

		// Coordenada Y justo debajo de los pies (sin cambios)
		int playerFeetY = int(playerPos.y + playerSize.y);
		int tileY_UnderFeet = playerFeetY / map->getTileSize();

		// --- Introducir Tolerancia Horizontal ---
		float toleranceX = 10.0f; // Píxeles de margen desde los bordes. Puedes ajustar este valor.
		// Un valor mayor hace la detección menos sensible en los bordes.

// Calcular coordenadas X *interiores* para la comprobación
// Sumamos tolerancia al borde izquierdo y restamos al borde derecho
		int checkX_Left = int(playerPos.x + toleranceX);
		int checkX_Right = int(playerPos.x + playerSize.x - 1.0f - toleranceX);

		int tileX_Left_Check = checkX_Left / map->getTileSize();
		int tileX_Right_Check = checkX_Right / map->getTileSize();


		// Comprobar tile bajo el punto izquierdo interior
		int tileID_Left = map->getTile(tileX_Left_Check, tileY_UnderFeet);
		if (tileID_Left == 23 || tileID_Left == 42) {
			player->takeDamage(10);
			return;
		}

		// Comprobar tile bajo el punto derecho interior (solo si es diferente de la columna izquierda)
		// Es importante comparar las columnas de *comprobación* (tileX_Left_Check != tileX_Right_Check)
		if (tileX_Left_Check != tileX_Right_Check) {
			int tileID_Right = map->getTile(tileX_Right_Check, tileY_UnderFeet);
			if (tileID_Right == 23 || tileID_Right == 42) {
				player->takeDamage(10);
				return;
			}
		}
	}

	// ---> INICIO: Detección de colisiones con Ranas <---
	if (player->isAlive()) { // Comprobar si sigue vivo
		for (int i = 0; i < numero_ranas; ++i) {
			if (ranas_spawned[i] && ranas[i] != nullptr && ranas[i]->isAlive()) {
				glm::vec2 ranaPos = ranas[i]->getHitboxPosition();
				glm::ivec2 ranaSize = ranas[i]->getHitboxSize();

				if (checkCollisionAABB(playerPos, playerSize, ranaPos, ranaSize)) {
					player->takeDamage(ranas[i]->getDamage()); // Usar getDamage()
					break; // Salir solo del bucle de ranas si recibe daño
				}
				if (player->estaAttacking()) {
					if (checkCollisionAABB(lanzaPos, playerSize, ranaPos, ranaSize)) {
						ranas[i]->takeDamage(player->getDamage());
						if (!ranas[i]->isAlive()){
							delete ranas[i];
							ranas[i] = nullptr;
							ranas_spawned[i] = false; // Marcar como no spawn
						}
						break; // Salir del bucle de ranas
					}
				}
				// Colisión proyectil-rana
				for (auto it = projectiles.begin(); it != projectiles.end(); ) {
					FireStickProjectile* proj = *it;
					if (proj->isActive()) {
						glm::vec2 projPos = proj->getPosition();
						glm::ivec2 projSize = proj->getSize();

						if (checkCollisionAABB(projPos, projSize, ranaPos, ranaSize)) {
							ranas[i]->takeDamage(player->getFireDamage());
							if (!ranas[i]->isAlive()){
							delete ranas[i];
							ranas[i] = nullptr;

							}
							proj->deactivate();

							break; // Salir del bucle de proyectiles
						}
					}
					++it;
				}
			}
		}
	}
	// ---> FIN: Detección de colisiones con Ranas <---

	// ---> INICIO: Detección de colisiones con BOSS <---
	if (bossActive && boss != nullptr && boss->isAlive() && player->isAlive()) {
		// 1. Player vs Boss Body Collision
		glm::vec2 bossPos = boss->getHitboxPosition(); // Use hitbox pos/size
		glm::ivec2 bossSize = boss->getHitboxSize();
		if (checkCollisionAABB(playerPos, playerSize, bossPos, bossSize)) {
			player->takeDamage(boss->getDamage());
			// Note: Player takes damage, boss does not necessarily change state here
		}

		// 2. Lanza vs Boss Collision
		if (player->estaAttacking()) {
			if (checkCollisionAABB(lanzaPos, playerSize, bossPos, bossSize)) {
				boss->takeDamage(player->getDamage());

				// Opcional: Efectos visuales o sonoros al impactar
				// AudioManager::getInstance()->playSound("boss_hit");
			}
		}

		// 3. Proyectiles vs Boss Collision
		vector<FireStickProjectile*>& projectiles = player->getProjectiles();
		for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
			FireStickProjectile* proj = *it;
			if (proj->isActive()) {
				glm::vec2 projPos = proj->getPosition();
				glm::ivec2 projSize = proj->getSize();

				if (checkCollisionAABB(projPos, projSize, bossPos, bossSize)) {
					int fireDamage = player->getFireDamage(); // Más daño por ser fuego
					boss->takeDamage(fireDamage);
					proj->deactivate();

					// Opcional: Efectos visuales o sonoros al impactar
					// AudioManager::getInstance()->playSound("fire_hit");
				}
			}
		}

		// 4. Comprobar si el jefe ha sido derrotado
		if (!boss->isAlive()) {
			std::cout << "¡Jefe derrotado!" << std::endl;
			// Aquí puedes añadir lógica adicional para cuando el jefe es derrotado
			// Por ejemplo, cambiar a la siguiente escena, dar recompensas, etc.
		}

		// Leaf collision is handled inside Boss::update
	}
	// ---> FIN: Detección de colisiones con BOSS <---

	if (player->isAlive() && bossActive) { 
		for (Bamboo* bamboo : bossBamboos) { // Iterar sobre los bambús del jefe
			// Comprobar si el bambú es válido, está "vivo" y no está ya marcado
			if (bamboo != nullptr && bamboo->isAlive() && !bamboo->shouldBeRemoved()) {
				glm::vec2 bambooPos = bamboo->getHitboxPosition();
				glm::ivec2 bambooSize = bamboo->getHitboxSize();

				if (checkCollisionAABB(playerPos, playerSize, bambooPos, bambooSize)) {
					std::cout << "Colisión detectada con bambú del jefe!" << std::endl;
					// Comprobar protección del jugador
					if (!player->getProteccionSuperior()) {
						std::cout << "Jugador sin protección, recibe daño." << std::endl;
						player->takeDamage(bamboo->getDamage());
						// Podrías añadir un pequeño retroceso al jugador aquí si quieres
					}
					else {
						std::cout << "Jugador protegido, no recibe daño." << std::endl;
						// Podrías añadir un sonido de "bloqueo" aquí
					}

					// Marcar el bambú para eliminarlo, independientemente de la protección
					bamboo->markForRemoval();
					std::cout << "Bambú del jefe marcado para eliminación." << std::endl;

				}
			}
		}
	}
}

bool Scene::checkCollisionAABB(const glm::vec2& pos1, const glm::ivec2& size1,
	const glm::vec2& pos2, const glm::ivec2& size2) const
{
	bool collisionX = pos1.x + size1.x >= pos2.x && pos2.x + size2.x >= pos1.x;
	bool collisionY = pos1.y + size1.y >= pos2.y && pos2.y + size2.y >= pos1.y;

	return collisionX && collisionY;
}


// --- Boss Helper Functions ---

void Scene::spawnSingleBamboo(const glm::vec2& spawnPos) {
	// Spawn a bamboo at the given position (e.g., boss feet)
	if (bossActive) { // Only spawn if boss is active
		std::cout << "Scene spawning single bamboo at (" << spawnPos.x << ", " << spawnPos.y << ")" << std::endl;
		Bamboo* newBamboo = new Bamboo();
		// Use the provided spawnPos directly
		glm::vec2 correctedSpawnPos = glm::vec2(spawnPos.x - 8, spawnPos.y); // Center the 16px bamboo
		newBamboo->init(correctedSpawnPos, texProgram);
		newBamboo->setTileMap(map);
		bossBamboos.push_back(newBamboo);
	}
}

void Scene::spawnBambooRain(float leftBound, float rightBound, float spawnY) {
	// Spawn multiple bamboos across the specified range at the given Y
	if (bossActive) {
		std::cout << "Scene spawning bamboo rain between " << leftBound << " and " << rightBound << " at Y=" << spawnY << std::endl;
		const int rainCount = 5; // Number of bamboos in the rain, adjust as needed
		float spacing = (rightBound - leftBound) / (rainCount + 1);

		for (int i = 0; i < rainCount; ++i) {
			Bamboo* newBamboo = new Bamboo();
			float spawnX = leftBound + (i + 1) * spacing - 8; // Center the bamboo
			glm::vec2 spawnPos = glm::vec2(spawnX, spawnY); // Use the provided Y
			newBamboo->init(spawnPos, texProgram);
			newBamboo->setTileMap(map);
			bossBamboos.push_back(newBamboo);
		}
	}
}

// --- End Boss Helper Functions ---

