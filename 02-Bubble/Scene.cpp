#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Scene.h"
#include "Game.h"

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 10

// FACTOR DE ZOOM: Reduce la vista para acercar la cámara al personaje
#define CAMERA_ZOOM 4.0f  // Aumenta este valor para acercar más la cámara

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
    posP = player->getPosition();

    // Aplicar el zoom en la proyección ortográfica
    projection = glm::ortho(0.f, float(SCREEN_WIDTH) / CAMERA_ZOOM, float(SCREEN_HEIGHT) / CAMERA_ZOOM, 0.f);

    currentTime = 0.0f;
}

void Scene::update(int deltaTime)
{
    currentTime += deltaTime;
    player->update(deltaTime);
    
}

void Scene::render()
{
    texProgram.use();
    texProgram.setUniformMatrix4f("projection", projection);
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
    glm::mat4 modelview;
    // Posición del personaje
    posP = player->getPosition();
    //cout << "posP: " << posP.x << " " << posP.y << endl;    
    // Obtener el tamaño del mapa en píxeles
    int mapWidth = map->getMapSize().x * map->getTileSize();
    int mapHeight = map->getMapSize().y * map->getTileSize();

    // Centrar la cámara en el personaje (ajustado con zoom)
    float camX = (posP.x - (SCREEN_WIDTH / (2 * CAMERA_ZOOM)));
    float camY = (posP.y - (SCREEN_HEIGHT / (2 * CAMERA_ZOOM)));
    //cout << "camX: " << camX << " camY: " << camY << endl;  

    // Limitar la cámara dentro de los bordes del mapa
    camX = glm::clamp(camX, 0.0f, float(mapWidth - SCREEN_WIDTH / CAMERA_ZOOM));
    camY = glm::clamp(camY, 0.0f, float(mapHeight - SCREEN_HEIGHT / CAMERA_ZOOM));

    // Aplicar la transformación de la cámara
    modelview = glm::translate(glm::mat4(1.0f), glm::vec3(-camX, -camY, 0.0f));
    texProgram.setUniformMatrix4f("modelview", modelview);
    texProgram.setUniform2f("texCoordDispl", 0.f, 0.f);

    // Renderizar mapa y personaje
    map->render();
    player->render(modelview);
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