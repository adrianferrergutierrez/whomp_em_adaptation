#ifndef _GAME_INCLUDE
#define _GAME_INCLUDE


#include <GLFW/glfw3.h>
#include "Scene.h"
#include "Sprite.h"
#include "Texture.h"


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PROPORTION SCREEN_WIDTH/SCREEN_HEIGHT


// Game states
enum GameState {
	CREDITS,       // No se usa, pero se mantiene para no cambiar los otros valores
	MAIN_MENU,     // Menú principal
	PLAYING,       // Jugando
	GAME_OVER,     // Juego terminado (derrota)
	VICTORY        // Juego terminado (victoria)
};

// Game is a singleton (a class	 with a single instance) that represents our whole application
class Game
{

private:
	Game() {}
	GLFWwindow* windowPtr = nullptr; // Store window pointer
	int currentFramebufferWidth = 0;
	int currentFramebufferHeight = 0;

public:
	static Game& instance()
	{
		static Game G;

		return G;
	}

	void init(GLFWwindow* window);
	bool update(int deltaTime);
	void render();

	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);

	bool getKey(int key) const;
	
	// Game state methods
	void changeState(GameState newState);
	GameState getState() const { return currentState; }
	void restartGame();

private:
	bool bPlay; // Continue to play game?
	bool keys[GLFW_KEY_LAST + 1]; // Store key states so that 
	// we can have access at any time
	Scene scene;
	
	// Game state
	GameState currentState;
	
	// Victory delay timer
	int victoryTimer;
	const int VICTORY_DELAY = 5000; // 5 seconds in milliseconds
	
	// Menu textures
	Texture newGameTexture;
	Texture volumeOnTexture;
	Texture volumeOffTexture;
	
	// Sound state
	bool soundEnabled;
	
	// Mouse position
	int mouseX, mouseY;
	bool mousePressed;
	
	// Helper methods for rendering different screens
	void renderMenu();
};


#endif // _GAME_INCLUDE


