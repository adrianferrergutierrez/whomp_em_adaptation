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
    bossHealthIcon = nullptr; // Init boss health icon pointer
    showBossHealth = false;
    isFireMode = false; // Initialize fire mode flag
    final = false;
}

GUI::~GUI()
{
    // Liberar memoria de los sprites
    if (spearIcon) delete spearIcon;
    if (heartIcon) delete heartIcon;
    if (clockIcon) delete clockIcon;
    if (flintIcon) delete flintIcon;
    if (helmetIcon) delete helmetIcon;
    if (bossHealthIcon) delete bossHealthIcon; // Delete boss health icon
    if (fireIcon) delete fireIcon; // Delete fire icon
}

void GUI::init(ShaderProgram* shader, glm::mat4 projectionMat, int camWidth, int camHeight)
{
    shaderProgram = shader;
    projectionMatrix = projectionMat; // Guardamos la projection matrix ya que la GUI va separada de la modelview de toda la escena, sino se moveria con la camara
    cameraWidth = camWidth;
    cameraHeight = camHeight;
    final = false;
    // Cargamos las texturas con proteccion de errores
    if (!totemsTexture.loadFromFile("images/Totems.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando Totems.png!" << std::endl;
    }
    if (!totemsTodosTexture.loadFromFile("images/totems_todos.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando totems_todos.png!" << std::endl;
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
    // Load boss health texture
    if (!bossHealthTexture.loadFromFile("images/vidas_boss.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "Error cargando vidas_boss.png!" << std::endl;
    }

    // --- Creamos sprites--
    totemBossIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0f / 7.0f, 1.0f), &totemsTodosTexture, shaderProgram);
    totemBossIcon->setNumberAnimations(1);
    totemBossIcon->addKeyframe(0, glm::vec2(0.0f, 0.0f));
    totemBossIcon->changeAnimation(0);

    // Lanza (Totems.png: 64x16, Icono: 16x16, 3er elemento)
    spearIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.25f, 1.0f), &totemsTexture, shaderProgram);
    spearIcon->setNumberAnimations(1);
    spearIcon->addKeyframe(0, glm::vec2(0.5f, 0.0f));
    spearIcon->changeAnimation(0);

    // Fire Icon (totems_todos.png: 112x16, Icon: 16x16, 2nd sprite in the row)
    fireIcon = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(1.0f / 7.0f, 1.0f), &totemsTodosTexture, shaderProgram);
    fireIcon->setNumberAnimations(1);
    fireIcon->addKeyframe(0, glm::vec2(1.0f / 7.0f, 0.0f)); // Second sprite (index 1) texture coordinate
    fireIcon->changeAnimation(0);

    // Corazn (Corazones.png: 50x10, Icono: 10x10)
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

    // Boss Health Icon (vidas_boss.png: 40x10, Icon: 10x10, 4 frames)
    bossHealthIcon = Sprite::createSprite(glm::ivec2(10, 10), glm::vec2(0.25f, 1.0f), &bossHealthTexture, shaderProgram);
    bossHealthIcon->setNumberAnimations(4);
    bossHealthIcon->addKeyframe(0, glm::vec2(0.0f, 0.0f)); // Lleno
    bossHealthIcon->addKeyframe(1, glm::vec2(0.25f, 0.0f)); // 2/3
    bossHealthIcon->addKeyframe(2, glm::vec2(0.5f, 0.0f)); // 1/3
    bossHealthIcon->addKeyframe(3, glm::vec2(0.75f, 0.0f)); // Vacio
    bossHealthIcon->changeAnimation(0);


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

    bossHealthPositions.resize(MAX_BOSS_ORANGES);

    const float bossHealthGap = 8.f; // Espacio entre vidas player y vidas boss
    float bossHealthStartX_col1 = heartPositions[heartsPerColumn].x + heartHorizontalSpacing + bossHealthGap;

    // --- AJUSTE: Columna 1 de 6, Columna 2 de 4 ---
    const int bossOrangesPerColumn1 = 6; // Primera columna tiene 6
    const float bossOrangeVerticalSpacing = 12.f; 
    const float bossOrangeHorizontalSpacing = 12.f; 

    for (int i = 0; i < MAX_BOSS_ORANGES; ++i) {
        int columnIndex = (i < bossOrangesPerColumn1) ? 0 : 1;
        int rowIndex = (i < bossOrangesPerColumn1) ? i : (i - bossOrangesPerColumn1);

        // Calculamos X basado en la columna (relativo a bossHealthStartX_col1)
        float posX = bossHealthStartX_col1 + (columnIndex * bossOrangeHorizontalSpacing);
        // Calculamos Y basado en la fila (igual que corazones player)
        float posY = spearPos.y + heartOffsetY + (rowIndex * bossOrangeVerticalSpacing);

        bossHealthPositions[i] = glm::vec2(posX, posY);
    }

}


void GUI::update(int currentHealth, int maxHealth, int numClocks, bool hasFlint, bool hasHelmet, bool isFireModeActive, bool bossIsActive, int currentBossHealth/*, bool final*/)
{
    // Cogemos los valores del update del scene para actualizar la GUI
    currentHP = currentHealth;
    maxHP = maxHealth;
    clocks = numClocks;
    flintCollected = hasFlint;
    helmetCollected = hasHelmet;
    isFireMode = isFireModeActive; // Update fire mode status
    // Store boss info
    this->final = final;
    showBossHealth = bossIsActive;
    boss_health = currentBossHealth;
}

void GUI::render()
{
    glm::mat4 modelview = glm::mat4(1.0f);

    if (!shaderProgram) return;

    shaderProgram->use();
    shaderProgram->setUniformMatrix4f("projection", projectionMatrix);
    shaderProgram->setUniformMatrix4f("modelview", glm::mat4(1.0f));

    // Conditionally render spear or fire icon
    if (isFireMode && fireIcon) {
        fireIcon->setPosition(spearPos);
        fireIcon->render(modelview);
    }
    else if (spearIcon) {
        spearIcon->setPosition(spearPos);
        spearIcon->render(modelview);
    }
    if (final) {
        totemBossIcon->setPosition(glm::vec2(128.0f, 128.0f));
        totemBossIcon->render(modelview);

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

    // ---> Render Boss Health <--- 
    if (showBossHealth && bossHealthIcon) {
        // Calculamos cuántas naranjas mostrar en total basado en la vida máxima del boss
        int orangesToDraw = (int)ceil((float)MAX_BOSS_ORANGES);
        orangesToDraw = glm::clamp(orangesToDraw, 0, MAX_BOSS_ORANGES);

        for (int i = 0; i < orangesToDraw; ++i) {
            bossHealthIcon->setPosition(bossHealthPositions[i]);

            // Calculamos la salud en esta naranja específica
            int healthInThisOrange = boss_health - (i * BOSS_HEALTH_PER_ORANGE);

            int animIndex = 3; // Vacío por defecto

            if (healthInThisOrange >= BOSS_HEALTH_PER_ORANGE) { // Naranja completamente llena
                animIndex = 0;
            }
            else if (healthInThisOrange >= 20) { // 2/3 llena (ajustar según tus necesidades)
                animIndex = 1;
            }
            else if (healthInThisOrange > 0) { // 1/3 llena (ajustar según tus necesidades)
                animIndex = 2;
            }

            bossHealthIcon->changeAnimation(animIndex);
            bossHealthIcon->render(modelview);
        }
    }
}

