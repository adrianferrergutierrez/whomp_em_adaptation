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
	// Load new screen textures
	instructionsTexture.loadFromFile("images/Instrucciones.png", TEXTURE_PIXEL_FORMAT_RGBA);
	creditsTexture.loadFromFile("images/Creditos.png", TEXTURE_PIXEL_FORMAT_RGBA);
	
	// Create Scene object and initialize its shaders EARLY for UI rendering
	if (scene != nullptr) delete scene; // Safety check
	scene = new Scene();
	scene->init(); // Initialize shaders needed for UI

	// Don't initialize the main scene gameplay elements here, 
	// that happens when transitioning to PLAYING

	// Initialize audio (keep existing logic)
	if (!AudioManager::getInstance()->init()) {
		std::cerr << "Error al inicializar el sistema de audio" << std::endl;
	}
	// DO NOT start music here - start it only when entering PLAYING state
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
			if (scene != nullptr) { // Check if scene exists
				scene->update(deltaTime);
			
				// Check if player has died
				if (!scene->getPlayer()->isAlive()) {
					changeState(GAME_OVER);
				}
				if (scene->isBossActive() && flag == 1) {
					if (!AudioManager::getInstance()->init()) {
						std::cerr << "Error al inicializar el sistema de audio" << std::endl;
					}
					else {
						// Reproducir música de fondo principal
						AudioManager::getInstance()->playMusic("sounds/bossfight.mp3", true, 0.6f);
					}
					flag = 0;
				}
				// Check victory condition with delay
				if (scene->bossDefeated()) {
					victoryTimer += deltaTime;
					if (victoryTimer >= VICTORY_DELAY) {
						// Go directly to Credits after victory delay
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
				const int newGameWidth = 256;  // Match renderMenu size
				const int newGameHeight = 80; // Match renderMenu size
				if (scaledMouseX >= centerX - newGameWidth/2 && 
					scaledMouseX <= centerX + newGameWidth/2 && 
					scaledMouseY >= centerY && 
					scaledMouseY <= centerY + newGameHeight) {
					
					// If clicking New Game in Game Over, we need to restart before instructions
					if (currentState == GAME_OVER) { 
						restartGame(); // Restart game logic state first
					}
					// Always transition to Instructions from menu click
					changeState(INSTRUCTIONS_SCREEN); 
					mousePressed = false;
				}
				
				// Volume button
				const int volumeDisplaySize = 64; // Match renderMenu size
				const float volumeButtonY = centerY + newGameHeight + 20; // Position below New Game button + padding
				if (scaledMouseX >= centerX - volumeDisplaySize/2 && 
					scaledMouseX <= centerX + volumeDisplaySize/2 && 
					scaledMouseY >= volumeButtonY && 
					scaledMouseY <= volumeButtonY + volumeDisplaySize) {
					
					// Toggle sound
					soundEnabled = !soundEnabled;
					
					if (soundEnabled) {
						// Restore volumes and resume music via AudioManager
						AudioManager::getInstance()->setMusicVolume(1.0f); 
						AudioManager::getInstance()->setEffectVolume(1.0f);
						AudioManager::getInstance()->resumeMusic();
					} else {
						// Set volumes to 0 and pause music via AudioManager
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
	
	// Asegurarse de que el framebuffer es válido
	if (currentFramebufferWidth <= 0 || currentFramebufferHeight <= 0) {
		// Si el framebuffer no es válido, usar valores predeterminados
		currentFramebufferWidth = SCREEN_WIDTH;
		currentFramebufferHeight = SCREEN_HEIGHT;
	}
	
	switch (currentState) {
		case PLAYING:
			// Render normal gameplay
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

// Método para renderizar la pantalla de menú
void Game::renderMenu() {
	// Set up for rendering
	ShaderProgram* shader = nullptr;
	if(scene != nullptr) { // Ensure scene exists before getting shader
		shader = &scene->getShaderProgram(); 
	} else {
		// Handle error or return if scene is unexpectedly null
		std::cerr << "Error: Scene is null in renderMenu!" << std::endl;
		return; 
	}

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
	
	// Aumentar el tamaño del botón New Game (aún más grande)
	const int newGameWidth = 256; // Larger width
	const int newGameHeight = 80; // Larger height
	
	// Create temporary sprite for the New Game button with increased size
	Sprite* newGameSprite = Sprite::createSprite(glm::ivec2(newGameWidth, newGameHeight), glm::vec2(1.0f, 1.0f), &newGameTexture, shader);
	newGameSprite->setPosition(glm::vec2(centerX - newGameWidth/2, centerY));
	newGameSprite->render(glm::mat4(1.0f));
	
	// Aumentar el tamaño del botón de volumen
	const int volumeDisplaySize = 64; // Keep this size or adjust if needed
	const float volumeButtonY = centerY + newGameHeight + 20; // Position below New Game button + padding
	Texture* currentVolumeTexture = soundEnabled ? &volumeOnTexture : &volumeOffTexture;
	Sprite* volumeSprite = Sprite::createSprite(glm::ivec2(volumeDisplaySize, volumeDisplaySize), glm::vec2(1.0f, 1.0f), currentVolumeTexture, shader);
	volumeSprite->setPosition(glm::vec2(centerX - volumeDisplaySize/2, volumeButtonY)); // Use calculated Y position
	volumeSprite->render(glm::mat4(1.0f));
	
	// Clean up
	delete newGameSprite;
	delete volumeSprite;
	
	// NOTA: Idealmente, aquí deberías añadir texto indicando:
	// - "Haz clic en 'New Game' para empezar"
	// - "Haz clic en el icono de volumen para activar/desactivar el sonido"
	// Necesitarías imágenes adicionales con estas instrucciones para mostrarlas en pantalla
}

// --- Render Instructions Screen ---
void Game::renderInstructionsScreen() {
	ShaderProgram* shader = nullptr;
	if(scene != nullptr) { 
		shader = &scene->getShaderProgram(); 
	} else {
		std::cerr << "Error: Scene is null in renderInstructionsScreen! Cannot get shader." << std::endl;
		return; 
	}
	shader->use();

	// Apply viewport scaling (same as renderMenu)
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

	// Render the instructions image full screen
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
		std::cerr << "Error: Scene is null in renderCreditsScreen! Cannot get shader." << std::endl;
		return; 
	}
	shader->use();

	// Apply viewport scaling (same as renderMenu)
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
	if (key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;
	keys[key] = true;
	
	// Additional key handling based on state
	switch (currentState) {
		case MAIN_MENU:
		case GAME_OVER:
			// Start flow when pressing Enter or Space from MAIN_MENU or GAME_OVER
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(INSTRUCTIONS_SCREEN); // Go to Instructions
			}
			break;
		case INSTRUCTIONS_SCREEN:
			// Proceed to game from Instructions
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(PLAYING); // Go to actual game
			}
			break;
		case VICTORY:
			// Proceed to credits from Victory
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(CREDITS_SCREEN);
			}
			break;
		case CREDITS_SCREEN:
			// Return to menu from Credits
			if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
				changeState(MAIN_MENU);
			}
			break;
		
		default: // Includes PLAYING state - no special Enter/Space action here
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

	// Stop sounds/music based on leaving previous state
	if (previousState == PLAYING && newState != PLAYING) {
		AudioManager::getInstance()->stopMusic();
	}

	// Handle transitions and setup for the newState
	switch (newState) {
		case MAIN_MENU:
		case INSTRUCTIONS_SCREEN:
		case CREDITS_SCREEN:
	
			if (previousState == PLAYING) {
				AudioManager::getInstance()->playSound("sounds/victory-sonic.mp3", 0.8f);
			}
			AudioManager::getInstance()->stopMusic(); // Ensure music is stopped
			break;
		case PLAYING:
			// Start/Restart logic only when entering PLAYING state
			if (previousState == INSTRUCTIONS_SCREEN) { // Check if coming from instructions
				 // Need to fully initialize the scene if starting fresh
				 restartGame(); // Calls scene->init() which loads map etc.
				
				 // Start appropriate music
				 if (scene != nullptr && scene->isBossActive()) { // Should not be active yet, but check anyway
					  AudioManager::getInstance()->playMusic("sounds/bossfight.mp3", true, 0.6f);
				 } else {
					  AudioManager::getInstance()->playMusic("sounds/supermario.mp3", true, 0.6f);
				 }
			 }
			// else if resuming from pause, different logic might apply
			break;
		case GAME_OVER:
			// Play death sound only on transition
			AudioManager::getInstance()->stopMusic(); 
			AudioManager::getInstance()->playSound("sounds/mario-bros-die.mp3", 0.8f); // Play death sound
			break;
		case VICTORY:
			// This state is now effectively skipped, keep sound stop for safety
			AudioManager::getInstance()->stopMusic();
			break;
		
		default:
			break;
	}

	currentState = newState;
}

void Game::restartGame()
{
	// Reset the music flag
	flag = 1; 
	
	// Stop any currently playing music and play the main theme
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
	
	// Ensure keyboard state is clean
	for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
		keys[i] = false;
	}
	
	// Reset mouse variables
	mousePressed = false;
}

// Add Game destructor implementation
Game::~Game() {
	if (scene != nullptr) {
		delete scene;
		scene = nullptr;
	}
	AudioManager::getInstance()->close(); // Clean up audio
}





