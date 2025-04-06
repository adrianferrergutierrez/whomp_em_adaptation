#pragma once

#ifndef _GUI_INCLUDE
#define _GUI_INCLUDE

#include <glm/glm.hpp>
#include <vector>
#include "ShaderProgram.h"
#include "Sprite.h"
#include "Texture.h"

class GUI
{
public:
    GUI();
    ~GUI();

    void init(ShaderProgram* shader, glm::mat4 projectionMat, int camWidth, int camHeight);
    void update(int currentHealth, int maxHealth, int numClocks, bool hasFlint, bool hasHelmet);
    void render();

private:
    ShaderProgram* shaderProgram;
    glm::mat4 projectionMatrix;
    int cameraWidth;
    int cameraHeight;

    // Añadimos texturas individuales
    Texture totemsTexture;
    Texture corazonesTexture;
    Texture corazones2Texture;
    Texture items1Texture;

    // Sprites para los elementos
    Sprite* spearIcon;
    Sprite* heartIcon;
    Sprite* clockIcon;
    Sprite* flintIcon;
    Sprite* helmetIcon;

    // Estado actual de los items
    int currentHP;
    int maxHP;
    int clocks;
    bool flintCollected;
    bool helmetCollected;

    // Posiciones fijas en pantalla para los elementos
    glm::vec2 spearPos;
    std::vector<glm::vec2> heartPositions; // Vector para posiciones verticales
    std::vector<glm::vec2> clockPositions; // Vector para posiciones horizontales
    std::vector<glm::vec2> itemPositions;  // Vector para posiciones de items abajo-izquierda

    // Constantes como la vida que representa un corazon o el numero de maximo de elementos
    const int HEALTH_PER_HEART = 30;
    const int MAX_DISPLAYABLE_HEARTS = 12; // O el máximo que quepa
    const int MAX_DISPLAYABLE_CLOCKS = 10; // O el máximo que quepa

};

#endif 
