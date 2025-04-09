#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "AudioManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h> // Necesario para glViewport


void Game::init(GLFWwindow* window)
{
	windowPtr = window;
	bPlay = true;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	flag = 1;
	currentState = MAIN_MENU; // Comenzamos aqui
	victoryTimer = 0;
	mouseX = 0;
	mouseY = 0;
	mousePressed = false;
	soundEnabled = true; // Por defecto sonido encendido
	
	if (windowPtr) {
		glfwGetFramebufferSize(windowPtr, &currentFramebufferWidth, &currentFramebufferHeight);
	}
	
	newGameTexture.loadFromFile("images/NewGame.png", TEXTURE_PIXEL_FORMAT_RGBA);
	volumeOnTexture.loadFromFile("images/volume_on.png", TEXTURE_PIXEL_FORMAT_RGBA);
	volumeOffTexture.loadFromFile("images/volume_off.png", TEXTURE_PIXEL_FORMAT_RGBA);
	instructionsTexture.loadFromFile("images/Instrucciones.png", TEXTURE_PIXEL_FORMAT_RGBA);
	creditsTexture.loadFromFile("images/Creditos.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	if (scene != nullptr) delete scene; 
	scene = new Scene();
	scene->init(); 


	if (!AudioManager::getInstance()->init()) {
	}
}

bool Game::update(int deltaTime)
{
	if (windowPtr) {
		glfwGetFramebufferSize(windowPtr, &currentFramebufferWidth, &currentFramebufferHeight);
		
		if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
			return bPlay;
		}
	}
	
	switch (currentState) {
		case PLAYING:
			if (scene != nullptr) { 
				scene->update(deltaTime);
			
				if (!scene->getPlayer()->isAlive()) {
					changeState(GAME_OVER);
				}
				if (scene->isBossActive() && flag == 1) {
					if (!AudioManager::getInstance()->init()) {
					}
					else {
						// Reproducimos musica
						AudioManager::getInstance()->playMusic("sounds/bossfight.mp3", true, 0.6f);
					}
					flag = 0;
				}
				// MIramos la victoria
				if (scene->bossDefeated()) {
					victoryTimer += deltaTime;
					if (victoryTimer >= VICTORY_DELAY) {
						// Vamos a creditos despues del tiempo
						changeState(CREDITS_SCREEN);
					}
				}
			}
			break;
			
		case MAIN_MENU:
		case GAME_OVER:
		case VICTORY:
		case CREDITS_SCREEN:
		default:
			if (mousePressed) {
				//coordenadas el mouse
				float centerX = SCREEN_WIDTH / 2.0f;
				float centerY = SCREEN_HEIGHT / 2.0f;
				
				float targetAspectRatio = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
				float windowAspectRatio = (currentFramebufferHeight > 0) ? 
					float(currentFramebufferWidth) / float(currentFramebufferHeight) : targetAspectRatio;
				
				float scaledMouseX, scaledMouseY;
				
				if (windowAspectRatio >= targetAspectRatio) {
					float viewportWidth = currentFramebufferHeight * targetAspectRatio;
					float viewportX = (currentFramebufferWidth - viewportWidth) / 2.0f;
					
					scaledMouseX = ((mouseX - viewportX) / viewportWidth) * SCREEN_WIDTH;
					scaledMouseY = (mouseY / float(currentFramebufferHeight)) * SCREEN_HEIGHT;
				} else {
					float viewportHeight = currentFramebufferWidth / targetAspectRatio;
					float viewportY = (currentFramebufferHeight - viewportHeight) / 2.0f;
					
					scaledMouseX = (mouseX / float(currentFramebufferWidth)) * SCREEN_WIDTH;
					scaledMouseY = ((mouseY - viewportY) / viewportHeight) * SCREEN_HEIGHT;
				}
				
				const int newGameWidth = 256;  
				const int newGameHeight = 80; 
				if (scaledMouseX >= centerX - newGameWidth/2 && 
					scaledMouseX <= centerX + newGameWidth/2 && 
					scaledMouseY >= centerY && 
					scaledMouseY <= centerY + newGameHeight) {
					
				
					if (currentState == GAME_OVER) { 
						restartGame(); // Restart game logic state first
					}
					changeState(INSTRUCTIONS_SCREEN); 
					mousePressed = false;
				}
				
				// Boton de volumen
				const int volumeDisplaySize = 64; 
				const float volumeButtonY = centerY + newGameHeight + 20; 
				if (scaledMouseX >= centerX - volumeDisplaySize/2 && 
					scaledMouseX <= centerX + volumeDisplaySize/2 && 
					scaledMouseY >= volumeButtonY && 
					scaledMouseY <= volumeButtonY + volumeDisplaySize) {
					
					// Toggle sound
					soundEnabled = !soundEnabled;
					
					if (soundEnabled) {
						// Recuperamos volumen
						AudioManager::getInstance()->setMusicVolume(1.0f); 
						AudioManager::getInstance()->setEffectVolume(1.0f);
						AudioManager::getInstance()->resumeMusic();
					} else {
						// Volumen a 0
						AudioManager::getInstance()->pauseMusic();
						AudioManager::getInstance()->setMusicVolume(0.0f);
						AudioManager::getInstance()->setEffectVolume(0.0f);
					}
					
					mousePressed = false;
				}
			}
			break;
	}

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
		currentFramebufferWidth = SCREEN_WIDTH;
		currentFramebufferHeight = SCREEN_HEIGHT;
	}
	
	switch (currentState) {
		case PLAYING:
			if (scene != nullptr) { // Check if scene exists
				scene->render(currentFramebufferWidth, currentFramebufferHeight);
			}
			break;
		case INSTRUCTIONS_SCREEN:
			renderInstructionsScreen();
			break;
		case CREDITS_SCREEN:
			renderCreditsScreen();
			break;
		case MAIN_MENU:
		case GAME_OVER:
		case VICTORY:
		default:
			renderMenu();
			break;
	}
}

void Game::renderMenu() {
	ShaderProgram* shader = nullptr;
	if(scene != nullptr) { // Ensure scene exists before getting shader
		shader = &scene->getShaderProgram(); 
	} else {
		return; 
	}

	shader->use();
	
	if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
		currentFramebufferWidth = SCREEN_WIDTH;
		currentFramebufferHeight = SCREEN_HEIGHT;
	}
	
	// Aplicar la misma lógica de escalado que se usa en el juego (pillarboxing/letterboxing)
	float targetAspectRatio = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
	float windowAspectRatio = (currentFramebufferHeight > 0) ? float(currentFramebufferWidth) / float(currentFramebufferHeight) : targetAspectRatio;
	
	int vp_x, vp_y, vp_width, vp_height;
	
	if (windowAspectRatio >= targetAspectRatio) {
		// Window wider than target (Pillarboxing)
		vp_height = currentFramebufferHeight;
		vp_width = int(vp_height * targetAspectRatio);
		vp_x = (currentFramebufferWidth - vp_width) / 2;
		vp_y = 0;
	} else {
		// Window taller than target (Letterboxing)
		vp_width = currentFramebufferWidth;
		vp_height = int(vp_width / targetAspectRatio);
		vp_x = 0;
		vp_y = (currentFramebufferHeight - vp_height) / 2;
	}
	
	// Aplicar el viewport calculado
	glViewport(vp_x, vp_y, vp_width, vp_height);
	
	// Configurar la proyección
	shader->setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	shader->setUniformMatrix4f("modelview", glm::mat4(1.0f));
	shader->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	shader->setUniform2f("texCoordDispl", 0.f, 0.f);
	
	// Escalado de botones para adaptarlos a la resolución actual
	float scale = float(vp_height) / float(SCREEN_HEIGHT);
	
	// Calcular posiciones centradas independientemente de la resolución
	float centerX = SCREEN_WIDTH / 2.0f;
	float centerY = SCREEN_HEIGHT / 2.0f;
	
	// Aumentar el tamaño del botón New Game (aún más grande)
	const int newGameWidth = 256; // Larger width
	const int newGameHeight = 80; // Larger height
	
	
	Sprite* newGameSprite = Sprite::createSprite(glm::ivec2(newGameWidth, newGameHeight), glm::vec2(1.0f, 1.0f), &newGameTexture, shader);
	newGameSprite->setPosition(glm::vec2(centerX - newGameWidth/2, centerY));
	newGameSprite->render(glm::mat4(1.0f));
	
	// Aumentar el tamaño del botón de volumen
	const int volumeDisplaySize = 64; 
	const float volumeButtonY = centerY + newGameHeight + 20; 
	Texture* currentVolumeTexture = soundEnabled ? &volumeOnTexture : &volumeOffTexture;
	Sprite* volumeSprite = Sprite::createSprite(glm::ivec2(volumeDisplaySize, volumeDisplaySize), glm::vec2(1.0f, 1.0f), currentVolumeTexture, shader);
	volumeSprite->setPosition(glm::vec2(centerX - volumeDisplaySize/2, volumeButtonY));
	volumeSprite->render(glm::mat4(1.0f));
	
	delete newGameSprite;
	delete volumeSprite;
	
}

// --- Render Instructions Screen ---
void Game::renderInstructionsScreen() {
	ShaderProgram* shader = nullptr;
	if(scene != nullptr) { 
		shader = &scene->getShaderProgram(); 
	} else {
		return; 
	}
	shader->use();

	float targetAspectRatio = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
	float windowAspectRatio = (currentFramebufferHeight > 0) ? float(currentFramebufferWidth) / float(currentFramebufferHeight) : targetAspectRatio;
	int vp_x, vp_y, vp_width, vp_height;
	if (windowAspectRatio >= targetAspectRatio) {
		vp_height = currentFramebufferHeight;
		vp_width = int(vp_height * targetAspectRatio);
		vp_x = (currentFramebufferWidth - vp_width) / 2;
		vp_y = 0;
	} else {
		vp_width = currentFramebufferWidth;
		vp_height = int(vp_width / targetAspectRatio);
		vp_x = 0;
		vp_y = (currentFramebufferHeight - vp_height) / 2;
	}
	glViewport(vp_x, vp_y, vp_width, vp_height);

	shader->setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	shader->setUniformMatrix4f("modelview", glm::mat4(1.0f));
	shader->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	shader->setUniform2f("texCoordDispl", 0.f, 0.f);

	Sprite* instructionsSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.0f, 1.0f), &instructionsTexture, shader);
	instructionsSprite->setPosition(glm::vec2(0, 0));
	instructionsSprite->render(glm::mat4(1.0f));
	delete instructionsSprite;
}

// --- Render Credits Screen ---
void Game::renderCreditsScreen() {
	ShaderProgram* shader = nullptr;
	if(scene != nullptr) { 
		shader = &scene->getShaderProgram(); 
	} else {
		return; 
	}
	shader->use();

	float targetAspectRatio = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
	float windowAspectRatio = (currentFramebufferHeight > 0) ? float(currentFramebufferWidth) / float(currentFramebufferHeight) : targetAspectRatio;
	int vp_x, vp_y, vp_width, vp_height;
	if (windowAspectRatio >= targetAspectRatio) {
		vp_height = currentFramebufferHeight;
		vp_width = int(vp_height * targetAspectRatio);
		vp_x = (currentFramebufferWidth - vp_width) / 2;
		vp_y = 0;
	} else {
		vp_width = currentFramebufferWidth;
		vp_height = int(vp_width / targetAspectRatio);
		vp_x = 0;
		vp_y = (currentFramebufferHeight - vp_height) / 2;
	}
	glViewport(vp_x, vp_y, vp_width, vp_height);

	// Setup projection and modelview
	shader->setUniformMatrix4f("projection", glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f));
	shader->setUniformMatrix4f("modelview", glm::mat4(1.0f));
	shader->setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
	shader->setUniform2f("texCoordDispl", 0.f, 0.f);

	// Render the credits image full screen
	Sprite* creditsSprite = Sprite::createSprite(glm::ivec2(SCREEN_WIDTH, SCREEN_HEIGHT), glm::vec2(1.0f, 1.0f), &creditsTexture, shader);
	creditsSprite->setPosition(glm::vec2(0, 0));
	creditsSprite->render(glm::mat4(1.0f));
	delete creditsSprite;
}

void Game::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE) 
		bPlay = false;
	keys[key] = true;
	
	switch (currentState) {
		case MAIN_MENU:
		case GAME_OVER:
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(INSTRUCTIONS_SCREEN); 
			}
			break;
		case INSTRUCTIONS_SCREEN:
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(PLAYING); 
			}
			break;
		case VICTORY:
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(CREDITS_SCREEN);
			}
			break;
		case CREDITS_SCREEN:
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(MAIN_MENU);
			}
			break;
		
		default: 
			break;
	}
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
	mouseX = x;
	mouseY = y;
}

void Game::mousePress(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		mousePressed = true;
	}
}

void Game::mouseRelease(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		mousePressed = false;
	}
}

bool Game::getKey(int key) const
{
	return keys[key];
}

void Game::changeState(GameState newState)
{
	GameState previousState = currentState;

	// Parem la musica si salimos de estado
	if (previousState == PLAYING && newState != PLAYING) {
		AudioManager::getInstance()->stopMusic();
	}

	switch (newState) {
		case MAIN_MENU:
		case INSTRUCTIONS_SCREEN:
		case CREDITS_SCREEN:
	
			if (previousState == PLAYING) {
				AudioManager::getInstance()->playSound("sounds/victory-sonic.mp3", 0.8f);
			}
			AudioManager::getInstance()->stopMusic(); 
			break;
		case PLAYING:
			if (previousState == INSTRUCTIONS_SCREEN) { 
				 restartGame(); 
				
				 if (scene != nullptr && scene->isBossActive()) { 
					  AudioManager::getInstance()->playMusic("sounds/bossfight.mp3", true, 0.6f);
				 } else {
					  AudioManager::getInstance()->playMusic("sounds/supermario.mp3", true, 0.6f);
				 }
			 }
			break;
		case GAME_OVER:
			AudioManager::getInstance()->stopMusic(); 
			AudioManager::getInstance()->playSound("sounds/mario-bros-die.mp3", 0.8f); 
			break;
		case VICTORY:
			AudioManager::getInstance()->stopMusic();
			break;
		
		default:
			break;
	}

	currentState = newState;
}

void Game::restartGame()
{
	// Reseteamos el flag de musica
	flag = 1; 
	
	// Hacemos stop
	AudioManager::getInstance()->stopMusic();
	AudioManager::getInstance()->playMusic("sounds/supermario.mp3", true, 0.6f);

	// Reset the scene by deleting the old one and creating a new one
	if (scene != nullptr) {
		delete scene;
		scene = nullptr;
	}
	scene = new Scene();
	scene->init(); 

	victoryTimer = 0;
	
	for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
		keys[i] = false;
	}
	
	mousePressed = false;
}

Game::~Game() {
	if (scene != nullptr) {
		delete scene;
		scene = nullptr;
	}
	AudioManager::getInstance()->close(); 
}





