#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 5
#define TILESIZE 16



Scene::Scene()
{
    map = NULL;
    player = NULL;
}

Scene::~Scene()
{
    if (map != NULL)
        delete map;
    if (player != NULL)
        delete player;
}

void Scene::init()
{
	initShaders();
	map = TileMap::createTileMap("levels/level02.txt", glm::vec2(SCREEN_X, SCREEN_Y), texProgram);
	player = new Player();
	player->init(glm::ivec2(SCREEN_X, SCREEN_Y), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);
	posPlayer = player->getPosition();
	projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f );
	currentTime = 0.0f;
}

    // Limitar la cámara a los bordes del mapa
	//if sector 11
    cameraPosition.x = glm::max(0.0f, cameraPosition.x);
    cameraPosition.y = glm::max(0.0f, cameraPosition.y);

	//if sector 2 

	//if sector 3
    float maxX = float(map->getMapWidth() * map->getTileSize() - CAMERA_WIDTH);
    float maxY = float(map->getMapHeight() * map->getTileSize() - CAMERA_HEIGHT);

    cameraPosition.x = glm::min(cameraPosition.x, maxX);
    cameraPosition.y = glm::min(cameraPosition.y, maxY);
}




void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
	player->update(deltaTime);
}
void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// Posición del personaje
	posPlayer = player->getPosition();

	// Definir los límites del mapa para evitar que la cámara se salga
	int mapWidth = map->getMapSize().x * map->getTileSize();
	int mapHeight = map->getMapSize().y * map->getTileSize();

	float playerSpeed = 8.0f;
	float currentSpeed;
	
	// Centrar la cámara en el personaje
	float camX = posPlayer.x - SCREEN_WIDTH / 2;
	float camY = posPlayer.y - SCREEN_HEIGHT / 2;

	// Evitar que la cámara se salga de los bordes del mapa
	camX = glm::clamp(camX, 0.0f, float(mapWidth - SCREEN_WIDTH));
	camY = glm::clamp(camY, 0.0f, float(mapHeight - SCREEN_HEIGHT));
	

	// Aplicar la transformación de la cámara
	modelview = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, 0.0f));

	texProgram.setUniformMatrix4f("modelview", modelview);
	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

	// Renderizar mapa y personaje
	map->render();
	player->render();
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