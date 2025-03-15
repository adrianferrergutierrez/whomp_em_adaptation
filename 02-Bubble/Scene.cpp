#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"


#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 5
#define TILESIZE 16

#define CAMERA_WIDTH 16*TILESIZE
#define CAMERA_HEIGHT 15*TILESIZE
#define CAMERA_FOLLOW_THRESHOLD 2.0f / 3.0f


Scene::Scene()
{
	map = NULL;
	player = NULL;
}

Scene::~Scene()
{
	if(map != NULL)
		delete map;
	if(player != NULL)
		delete player;
}

void Scene::init()
{

	initShaders();
	map = TileMap::createTileMap("levels/level02.txt", glm::vec2(0, 0), texProgram);
	player = new Player();
	player->init(glm::ivec2(0, 0), texProgram);
	player->setPosition(glm::vec2(INIT_PLAYER_X_TILES * map->getTileSize(), INIT_PLAYER_Y_TILES * map->getTileSize()));
	player->setTileMap(map);

	projection = glm::ortho(0.f, float(CAMERA_WIDTH), float(CAMERA_HEIGHT), 0.f);
	currentTime = 0.0f;
	cameraPosition = glm::vec2(0.0f);

	//calculamos el numero de tiles donde queremos que cambiemos
	primer_checkpoint = 144.f * TILESIZE - CAMERA_WIDTH;
	segundo_checkpoint = primer_checkpoint + CAMERA_WIDTH;
	tercer_checkpoint = segundo_checkpoint + 48.f * TILESIZE;
	cuarto_checkpoint = tercer_checkpoint + CAMERA_WIDTH;
	quinto_checkpoint = map->getMapWidth() * map->getTileSize() - CAMERA_WIDTH;
}
void Scene::updateCamera()
{
    glm::vec2 playerPos = player->getPosition();
	if (playerPos.x == primer_checkpoint) {
		verticalPos = true;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = primer_checkpoint;
	}
	if (playerPos.x == segundo_checkpoint) {
		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = segundo_checkpoint;
		cameraPosition.y = 90*TILESIZE;
	}
	if (playerPos.x == tercer_checkpoint) {
		verticalPos = true;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = tercer_checkpoint;
	}

	if (playerPos.x == cuarto_checkpoint) {
		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = cuarto_checkpoint;
		cameraPosition.y = 30* TILESIZE;
	}

	if (playerPos.x == quinto_checkpoint) {
		verticalPos = false;
		// Mantener la posición X fija donde está el jugador
		cameraPosition.x = quinto_checkpoint;
		final = true;
	}

	if (verticalPos) {
        if (playerPos.y > cameraPosition.y + (CAMERA_HEIGHT * 0.7f)) {
            cameraPosition.y = playerPos.y - (CAMERA_HEIGHT * 0.7f);
        }
        else if (playerPos.y < cameraPosition.y + (CAMERA_HEIGHT * 0.3f)) {
            cameraPosition.y = playerPos.y - (CAMERA_HEIGHT * 0.3f);
        }
    }
    // Comportamiento normal antes de x = 2080
    else {
        // Seguimiento horizontal normal
		if (!final) {
			if (playerPos.x > cameraPosition.x + (CAMERA_WIDTH * 0.7f)) {
				cameraPosition.x = playerPos.x - (CAMERA_WIDTH * 0.7f);
			}
			else if (playerPos.x < cameraPosition.x + (CAMERA_WIDTH * 0.3f)) {
				cameraPosition.x = playerPos.x - (CAMERA_WIDTH * 0.3f);
			}
		}
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
	updateCamera();  
}
void Scene::render()
{
	glm::mat4 modelview;

	texProgram.use();
	texProgram.setUniformMatrix4f("projection", projection);
	texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

	// Aplicamos la transformación de la cámara
	modelview = glm::mat4(1.0f);
	modelview = glm::translate(modelview, glm::vec3(-cameraPosition.x, -cameraPosition.y, 0.f));
	texProgram.setUniformMatrix4f("modelview", modelview);

	texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);
	map->render();
	player->render(modelview);
}
void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/texture.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/texture.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	texProgram.init();
	texProgram.addShader(vShader);
	texProgram.addShader(fShader);
	texProgram.link();
	if(!texProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << texProgram.log() << endl << endl;
	}
	texProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}



