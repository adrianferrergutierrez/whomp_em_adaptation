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
	
	// Initialize game state
	currentState = MAIN_MENU; // Comenzar directamente en el menú principal
	victoryTimer = 0;
	mouseX = 0;
	mouseY = 0;
	mousePressed = false;
	soundEnabled = true; // Por defecto, el sonido está activado
	
	// Asegurarse de obtener el tamaño del framebuffer desde el inicio
	if (windowPtr) {
		glfwGetFramebufferSize(windowPtr, &currentFramebufferWidth, &currentFramebufferHeight);
	}
	
	// Load menu textures
	newGameTexture.loadFromFile("images/NewGame.png", TEXTURE_PIXEL_FORMAT_RGBA);
	volumeOnTexture.loadFromFile("images/volume_on.png", TEXTURE_PIXEL_FORMAT_RGBA);
	volumeOffTexture.loadFromFile("images/volume_off.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	// Initialize scene (even though we start in menu)
	scene.init();
	
	//Inicializar audio
	if (!AudioManager::getInstance()->init()) {
		std::cerr << "Error al inicializar el sistema de audio" << std::endl;
	}
	else {
		// Reproducir música de fondo principal
		AudioManager::getInstance()->playMusic("sounds/supermario.mp3", true, 0.6f);
	}
}

bool Game::update(int deltaTime)
{
	// Siempre actualizar las dimensiones del framebuffer cuando sea posible
	if (windowPtr) {
		glfwGetFramebufferSize(windowPtr, &currentFramebufferWidth, &currentFramebufferHeight);
		
		// Si las dimensiones son 0, no continuar hasta que tengamos un framebuffer válido
		if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
			return bPlay;
		}
	}
	
	switch (currentState) {
		case PLAYING:
			// Update regular gameplay
			scene.update(deltaTime);
			
			// Check if player has died
			if (!scene.getPlayer()->isAlive()) {
				changeState(GAME_OVER);
			}
			
			// Check victory condition with delay
			if (scene.bossDefeated()) {
				victoryTimer += deltaTime;
				if (victoryTimer >= VICTORY_DELAY) {
					changeState(VICTORY);
				}
			}
			break;
			
		case MAIN_MENU:
		case GAME_OVER:
		case VICTORY:
		default:
			// Handle menu logic
			if (mousePressed) {
				// Convertir coordenadas de mouse a coordenadas del juego escaladas
				float centerX = SCREEN_WIDTH / 2.0f;
				float centerY = SCREEN_HEIGHT / 2.0f;
				
				// Ajustar las coordenadas del mouse según el viewport
				float targetAspectRatio = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
				float windowAspectRatio = (currentFramebufferHeight > 0) ? 
					float(currentFramebufferWidth) / float(currentFramebufferHeight) : targetAspectRatio;
				
				float scaledMouseX, scaledMouseY;
				
				if (windowAspectRatio >= targetAspectRatio) {
					// Window wider than target (Pillarboxing)
					float viewportWidth = currentFramebufferHeight * targetAspectRatio;
					float viewportX = (currentFramebufferWidth - viewportWidth) / 2.0f;
					
					// Convertir a coordenadas del juego
					scaledMouseX = ((mouseX - viewportX) / viewportWidth) * SCREEN_WIDTH;
					scaledMouseY = (mouseY / float(currentFramebufferHeight)) * SCREEN_HEIGHT;
				} else {
					// Window taller than target (Letterboxing)
					float viewportHeight = currentFramebufferWidth / targetAspectRatio;
					float viewportY = (currentFramebufferHeight - viewportHeight) / 2.0f;
					
					// Convertir a coordenadas del juego
					scaledMouseX = (mouseX / float(currentFramebufferWidth)) * SCREEN_WIDTH;
					scaledMouseY = ((mouseY - viewportY) / viewportHeight) * SCREEN_HEIGHT;
				}
				
				// Check if NewGame button was clicked
				const int newGameWidth = 96;  // Mismo valor que en renderMenu
				const int newGameHeight = 30; // Mismo valor que en renderMenu
				if (scaledMouseX >= centerX - newGameWidth/2 && 
					scaledMouseX <= centerX + newGameWidth/2 && 
					scaledMouseY >= centerY && 
					scaledMouseY <= centerY + newGameHeight) {
					
					if (currentState == GAME_OVER || currentState == VICTORY) {
						restartGame();
					}
					changeState(PLAYING);
					mousePressed = false;
				}
				
				// Volume button
				const int volumeDisplaySize = 64; // Mismo valor que en renderMenu
				if (scaledMouseX >= centerX - volumeDisplaySize/2 && 
					scaledMouseX <= centerX + volumeDisplaySize/2 && 
					scaledMouseY >= centerY + 40 && 
					scaledMouseY <= centerY + 40 + volumeDisplaySize) {
					
					// Toggle sound
					soundEnabled = !soundEnabled;
					
					if (soundEnabled) {
						AudioManager::getInstance()->resumeMusic();
						AudioManager::getInstance()->setEffectVolume(1.0f);
					} else {
						AudioManager::getInstance()->pauseMusic();
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
	
	// Asegurarse de que el framebuffer es válido
	if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
		// Si el framebuffer no es válido, usar valores predeterminados
		currentFramebufferWidth = SCREEN_WIDTH;
		currentFramebufferHeight = SCREEN_HEIGHT;
	}
	
	switch (currentState) {
		case PLAYING:
			// Render normal gameplay
			scene.render(currentFramebufferWidth, currentFramebufferHeight);
			break;
			
		case MAIN_MENU:
		case GAME_OVER:
		case VICTORY:
		default:
			renderMenu();
			break;
	}
}

// Método para renderizar la pantalla de menú
void Game::renderMenu() {
	// Set up for rendering
	ShaderProgram* shader = &scene.getShaderProgram();
	shader->use();
	
	// Asegurarse de que el framebuffer es válido
	if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
		// Si el framebuffer no es válido, usar valores predeterminados
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
	
	// Aumentar el tamaño del botón New Game (2x el tamaño original)
	const int newGameWidth = 96;  // Doble del original (48)
	const int newGameHeight = 30; // Doble del original (15)
	
	// Create temporary sprite for the New Game button with increased size
	Sprite* newGameSprite = Sprite::createSprite(glm::ivec2(newGameWidth, newGameHeight), glm::vec2(1.0f, 1.0f), &newGameTexture, shader);
	newGameSprite->setPosition(glm::vec2(centerX - newGameWidth/2, centerY));
	newGameSprite->render(glm::mat4(1.0f));
	
	// Aumentar el tamaño del botón de volumen
	const int volumeDisplaySize = 64; // Doble del tamaño anterior (32)
	Texture* currentVolumeTexture = soundEnabled ? &volumeOnTexture : &volumeOffTexture;
	Sprite* volumeSprite = Sprite::createSprite(glm::ivec2(volumeDisplaySize, volumeDisplaySize), glm::vec2(1.0f, 1.0f), currentVolumeTexture, shader);
	volumeSprite->setPosition(glm::vec2(centerX - volumeDisplaySize/2, centerY + 40)); // Ajustar la posición vertical
	volumeSprite->render(glm::mat4(1.0f));
	
	// Clean up
	delete newGameSprite;
	delete volumeSprite;
	
	// NOTA: Idealmente, aquí deberías añadir texto indicando:
	// - "Haz clic en 'New Game' para empezar"
	// - "Haz clic en el icono de volumen para activar/desactivar el sonido"
	// Necesitarías imágenes adicionales con estas instrucciones para mostrarlas en pantalla
}

void Game::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;
	keys[key] = true;
	
	// Additional key handling based on state
	switch (currentState) {
		case MAIN_MENU:
		case GAME_OVER:
		case VICTORY:
			// Restaurar la funcionalidad para iniciar el juego con Enter o Espacio
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				// Start game when pressing Enter or Space from menus
				if (currentState == GAME_OVER || currentState == VICTORY) {
					restartGame();
				}
				changeState(PLAYING);
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
	// Handle state transitions
	if (newState == PLAYING && (currentState == MAIN_MENU || currentState == GAME_OVER || currentState == VICTORY)) {
		// Transition from menu to playing
		if (currentState == GAME_OVER || currentState == VICTORY) {
			// Reset scene if coming from game over or victory
			restartGame();
		}
	}
	
	currentState = newState;
}

void Game::restartGame()
{
	// Método más seguro para reiniciar la escena
	// En lugar de destruir y recrear el objeto, solo llamamos a init() de nuevo
	scene.init();
	victoryTimer = 0;
	
	// Asegurarse de que el estado del teclado esté limpio
	for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
		keys[i] = false;
	}
	
	// Reiniciar variables de mouse
	mousePressed = false;
}





