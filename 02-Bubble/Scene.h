#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "TileMap.h"
#include "Player.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();


	void init();
	void update(int deltaTime);
	void render();
	glm::vec2 getCameraPosition() { return cameraPosition; }  // Nuevo método para obtener la posición de la cámara

private:
	void initShaders();
	void updateCamera();  // Nuevo método para actualizar la cámara
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
	TileMap *map;
	Player *player;
	ShaderProgram texProgram;
	float currentTime;
	glm::mat4 projection;
	//variable para saber si estamos en el final y bloquear la camara en caso de que sea asi 
	bool final = false;




};


#endif // _SCENE_INCLUDE

