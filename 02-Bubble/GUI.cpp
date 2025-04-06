#include "GUI.h"
#include "Game.h" // Para acceso a constantes como CAMERA_WIDTH/HEIGHT si es necesario
#include <glm/gtc/matrix_transform.hpp> /
#include <iostream> // Para std::cout temporal

GUI::GUI()
{
    shaderProgram = nullptr;
    spearIcon = nullptr;
    heartIcon = nullptr;
    clockIcon = nullptr;
    flintIcon = nullptr;
    helmetIcon = nullptr;
}

GUI::~GUI()
{
    // Liberar memoria de los sprites
    if (spearIcon) delete spearIcon;
    if (heartIcon) delete heartIcon;
    if (clockIcon) delete clockIcon;
    if (flintIcon) delete flintIcon;
    if (helmetIcon) delete helmetIcon;
}

void GUI::init(ShaderProgram* shader, glm::mat4 projectionMat, int camWidth, int camHeight)
{
    shaderProgram = shader;
    projectionMatrix = projectionMat; // Guardamos la projection matrix ya que la GUI va separada de la modelview de toda la escena, sino se moveria con la camara
    cameraWidth = camWidth;
    cameraHeight = camHeight;

    // Cargamos las texturas con proteccion de errores
    if (!totemsTexture.loadFromFile("images/Totems.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando Totems.png!" << std::endl;
    }
    if (!corazonesTexture.loadFromFile("images/Corazones.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando Corazones.png!" << std::endl;
    }
    if (!corazones2Texture.loadFromFile("images/Corazones2.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando Corazones2.png!" << std::endl;
    }
    if (!items1Texture.loadFromFile("images/Items1.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando Items1.png!" << std::endl;
    }

    // --- Creamos sprites--

    // Lanza (Totems.png: 64x16, Icono: 16x16, 3er elemento)
    spearIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.25f, 1.0f), &totemsTexture, shaderProgram);
    spearIcon->setNumberAnimations(1);
    spearIcon->addKeyframe(0, glm::vec2(0.5f, 0.0f));
    spearIcon->changeAnimation(0);

    // Corazón (Corazones.png: 50x10, Icono: 10x10)
    heartIcon = Sprite::createSprite(glm::ivec2(10, 10), glm::vec2(0.2f, 1.0f), &corazonesTexture, shaderProgram);
    heartIcon->setNumberAnimations(4);
    heartIcon->addKeyframe(0, glm::vec2(0.0f, 0.0f)); // Lleno
    heartIcon->addKeyframe(1, glm::vec2(0.2f, 0.0f)); // 2/3 lleno
    heartIcon->addKeyframe(2, glm::vec2(0.4f, 0.0f)); // 1/3 lleno
    heartIcon->addKeyframe(3, glm::vec2(0.6f, 0.0f)); // Vacio
    heartIcon->changeAnimation(0); // Empezamos con el corazon lleno

    // Reloj (Corazones2.png: 48x16, Icono: 16x16, 2ndo elemento)
    clockIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0f / 3.0f, 1.0f), &corazones2Texture, shaderProgram);
    clockIcon->setNumberAnimations(1);
    clockIcon->addKeyframe(0, glm::vec2(1.0f / 3.0f, 0.0f));
    clockIcon->changeAnimation(0);

    // Casco (Items1.png: 32x16, Icono: 16x16, 2ndo elemento)
    helmetIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.5f, 1.0f), &items1Texture, shaderProgram);
    helmetIcon->setNumberAnimations(1);
    helmetIcon->addKeyframe(0, glm::vec2(0.5f, 0.0f));
    helmetIcon->changeAnimation(0);


    // --- Calculamos posiciones en la pantalla ----

    //Posicion del cuadrado con la lanza
    spearPos = glm::vec2(16.f, 12.f);

    //Como maximo tendremos 12 corazones (como en el juego original)
    heartPositions.resize(MAX_DISPLAYABLE_HEARTS);

    // Constantes cortesia de la ia
    const float heartOffsetX1 = 3.f;
    const float heartOffsetY = 16.f;
    const float heartVerticalSpacing = 12.f;
    const float heartHorizontalSpacing = 12.f;
    const int heartsPerColumn = 6;

    for (int i = 0; i < MAX_DISPLAYABLE_HEARTS; ++i) {
        int columnIndex = i / heartsPerColumn;
        int rowIndex = i % heartsPerColumn;

        // Calcular X basado en la columna
        float posX = spearPos.x + heartOffsetX1 + (columnIndex * heartHorizontalSpacing);
        // Calcular Y basado en la fila
        float posY = spearPos.y + heartOffsetY + (rowIndex * heartVerticalSpacing);

        heartPositions[i] = glm::vec2(posX, posY);
    }

    clockPositions.resize(MAX_DISPLAYABLE_CLOCKS);
    for (int i = 0; i < MAX_DISPLAYABLE_CLOCKS; ++i) {
        clockPositions[i] = glm::vec2(spearPos.x + 16.f + 8.f + i * 18.f, spearPos.y);
    }

    itemPositions.resize(5);
    for (int i = 0; i < itemPositions.size(); ++i) {
        itemPositions[i] = glm::vec2(16.f + i * 18.f, cameraHeight - 16.f - 16.f);
    }

    std::cout << "GUI Initialized" << std::endl;
}

void GUI::update(int currentHealth, int maxHealth, int numClocks, bool hasFlint, bool hasHelmet)
{
    // Cogemos los valores del update del scene para actualizar la GUI
    currentHP = currentHealth;
    maxHP = maxHealth;
    clocks = numClocks;
    flintCollected = hasFlint;
    helmetCollected = hasHelmet;
}

void GUI::render()
{
    glm::mat4 modelview = glm::mat4(1.0f);

    if (!shaderProgram) return;

    shaderProgram->use();
    shaderProgram->setUniformMatrix4f("projection", projectionMatrix);
    shaderProgram->setUniformMatrix4f("modelview", glm::mat4(1.0f));

    // Lanza
    if (spearIcon) {
        spearIcon->setPosition(spearPos);
        spearIcon->render(modelview);
    }

    // Corazones
    if (heartIcon) {
        int heartsToDraw = (int)ceil((float)maxHP / HEALTH_PER_HEART);
        heartsToDraw = glm::clamp(heartsToDraw, 0, MAX_DISPLAYABLE_HEARTS);

        for (int i = 0; i < heartsToDraw; ++i) {
            heartIcon->setPosition(heartPositions[i]);

            int healthInThisHeart = currentHP - (i * HEALTH_PER_HEART);

            int animIndex = 3;

            if (healthInThisHeart >= HEALTH_PER_HEART) { // HP: 30+ (Lleno)
                animIndex = 0;
            }
            else if (healthInThisHeart >= 20) { // HP: [20, 30) (2/3 Lleno)
                animIndex = 1;
            }
            else if (healthInThisHeart >= 10) { // HP: [10, 20) (1/3 Lleno)
                animIndex = 2;
            }
            heartIcon->changeAnimation(animIndex);
            heartIcon->render(modelview);
        }
    }

    // Relojes
    if (clockIcon) {
        int clocksToDraw = glm::clamp(clocks, 0, MAX_DISPLAYABLE_CLOCKS);
        for (int i = 0; i < clocksToDraw; ++i) {
            clockIcon->setPosition(clockPositions[i]);
            clockIcon->render(modelview);
        }
    }

    // Items recolectados
    int currentItemIndex = 0;

    if (helmetCollected && helmetIcon && currentItemIndex < itemPositions.size()) {
        helmetIcon->setPosition(itemPositions[currentItemIndex]);
        helmetIcon->render(modelview);
        currentItemIndex++;
    }
}

