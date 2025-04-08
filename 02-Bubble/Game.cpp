#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"
#include "AudioManager.h"


void Game::init(GLFWwindow* window)
{
	windowPtr = window;
	bPlay = true;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
	if (windowPtr) {
		glfwGetFramebufferSize(windowPtr, &currentFramebufferWidth, &currentFramebufferHeight);
	}
	scene.update(deltaTime);

	return bPlay;
}

void Game::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.render(currentFramebufferWidth, currentFramebufferHeight);
}

void Game::keyPressed(int key)
{
	if (key == GLFW_KEY_ESCAPE) // Escape code
		bPlay = false;
	keys[key] = true;
}

void Game::keyReleased(int key)
{
	keys[key] = false;
}

void Game::mouseMove(int x, int y)
{
}

void Game::mousePress(int button)
{
}

void Game::mouseRelease(int button)
{
}

bool Game::getKey(int key) const
{
	return keys[key];
}



