#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 16*3 //3 tiles
#define FALL_STEP 3


enum PlayerAnims
{
    STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, JUMP_LEFT
};


void Player::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    // Inicialización del sistema de salud,items y daño
    maxHealth = 120;
    health = Health(maxHealth);

    pumpinkcount = 0;
    clockscount = 2;

    damage = 10;

    isBlinking = false;
    blinkTime = 0;
    visible = true;

    // Inicialización del jugador
    bJumping = false;
    spritesheet.loadFromFile("images/indio.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);

    sprite->setNumberAnimations(6);

    sprite->setAnimationSpeed(JUMP_RIGHT, 8);
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.25, 0.254));

    sprite->setAnimationSpeed(JUMP_LEFT, 8);
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.5, 0.254));

    // Animación de estar quieto mirando a la izquierda
    sprite->setAnimationSpeed(STAND_LEFT, 8);
    sprite->addKeyframe(STAND_LEFT, glm::vec2(0.75, 0.125));

    // Animación de estar quieto mirando a la derecha
    sprite->setAnimationSpeed(STAND_RIGHT, 8);
    sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0, 0.0));

    // Animación de moverse a la izquierda
    sprite->setAnimationSpeed(MOVE_LEFT, 8);
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.5, 0.125));
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.25, 0.125));
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.0, 0.125));

    // Animación de moverse a la derecha
    sprite->setAnimationSpeed(MOVE_RIGHT, 8);
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.0));
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5, 0.0));
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75, 0.0));

    // Animación inicial
    sprite->changeAnimation(0);
    tileMapDispl = tileMapPos;
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}


void Player::update(int deltaTime, vector<Tronco*> troncos)
{

    // Actualizar el sistema de salud
    float deltaTimeSeconds = deltaTime / 1000.0f;
    health.update(deltaTimeSeconds);

    // Gestionar el parpadeo cuando el jugador es invulnerable
    if (health.getIsInvulnerable()) {
        isBlinking = true;
        blinkTime += deltaTime;

        // Alternar visibilidad cada 100ms para que se vea el parpadeo
        if (blinkTime >= 100) {
            visible = !visible;
            blinkTime = 0;
        }
    }
    else {
        isBlinking = false;
        visible = true;
        blinkTime = 0;
    }

    //cout para debug
    cout << "Health: " << health.getCurrentHealth() << " Invulnerable: " << health.getIsInvulnerable() << endl;
    sprite->update(deltaTime);

    bool movingHorizontally = false; // Variable para saber si hay input de movimiento horizontal

    // ----Gestion de movimiento---
    if (Game::instance().getKey(GLFW_KEY_LEFT))
    {
        posPlayer.x -= 2;
        //Comprovamos colisiones con el mapa para la izquierda. Los valores que parecen RANDOM no lo son, son valores para que el player no pueda volver para atras una vez hemos cruzado un CHECKPOINT
        if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || posPlayer.x <= 0 || posPlayer.x == 2048 || posPlayer.x == 2310)
        {
            //Cancelamos el movimiento
            posPlayer.x += 2;
        }
        else {
            //Confirmamos el movimiento par ala animacion
            movingHorizontally = true;
        }
    }
    else if (Game::instance().getKey(GLFW_KEY_RIGHT))
    {
        posPlayer.x += 2;

        if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32))) // Quitado límite derecho por simplicidad ahora
        {
            //Rectificamos el movimiento, ha colisionado
            posPlayer.x -= 2;
        }
        else {
            //Confirmamos el movimiento a la derecha para la animacion
            movingHorizontally = true;
        }
    }

    // Variable para saber si el jugador está en el suelo (mapa o tronco)
    bool onGround = false;
    float groundY = 0.f; // Posición Y del suelo donde está parado

    // Salto
    if (bJumping)
    {
        jumpAngle += JUMP_ANGLE_STEP;
        if (jumpAngle >= 180) // Usar >= por si acaso
        {
            bJumping = false;
        }
        else
        {
            posPlayer.y = int(startY - JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

            // Comprobación de aterrizaje (solo cuando va hacia abajo)
            if (jumpAngle > 90)
            {
                // ---> INICIO: Comprobar aterrizaje en Tronco durante el salto <---
                bool landedOnTronco = false;
                float landingTroncoY = 0.f;
                for (Tronco* tronco : troncos) {
                    // Usamos una copia de posPlayer para la predicción, +1 para asegurar que esté "debajo", cortesia de la ia para arreglar los bugs de colisiones
                    glm::vec2 predictedPos = posPlayer;
                    predictedPos.y += 1;
                    if (tronco != nullptr && tronco->isPlayerOnTop(predictedPos, glm::ivec2(32, 32))) {
                        // Aterriza en un tronco
                        landedOnTronco = true;
                        landingTroncoY = tronco->getPosition().y;
                        break;
                    }
                }

                if (landedOnTronco) {
                    bJumping = false;
                    posPlayer.y = landingTroncoY - 32.f;
                    onGround = true;
                    groundY = posPlayer.y;

                }
                else {

                    // Si no aterrizó en tronco, comprobar colisión con el mapa
                    if (map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y)) {
                        bJumping = false; // Aterrizó en el mapa
                        onGround = true; // Marcamos que está en el suelo
                        groundY = posPlayer.y; // Guardamos la Y del suelo
                    }
                }
            }
        }
    }

    // Caída (si no está saltando y no acaba de aterrizar en este frame) / Comprobación de suelo
    if (!bJumping)
    {
        //aplicamos la gravedad
        posPlayer.y += FALL_STEP;

        // Comprovamos si hay colision con el suelo
        bool groundDetected = false;
        float surfaceY = 0.f; // La Y donde debería estar el jugador si toca suelo

        //miramos el movimiento junto a los troncos
        for (Tronco* tronco : troncos)
        {
            if (tronco != nullptr) {
                glm::vec2 playerCurrentPos = posPlayer; // Posición después de aplicar gravedad
                glm::ivec2 playerSize = glm::ivec2(32, 32);
                float playerHeight = 32.f;

                glm::vec2 troncoPos = tronco->getPosition(); // Esquina sup. izq. de hitbox 16x16
                glm::ivec2 troncoSize = tronco->getSize();   // Tamaño hitbox 16x16
                float verticalDisp = tronco->getVerticalDisplacement(); // Desplazamiento del tronco este frame

                // Comprobación horizontal (usando hitbox 16x16 del tronco)
                bool horizontalAlign = (playerCurrentPos.x + playerSize.x > troncoPos.x) &&
                    (playerCurrentPos.x < troncoPos.x + troncoSize.x);

                if (horizontalAlign) {
                    // Comprobación vertical
                    float troncoTopY = troncoPos.y;
                    float playerFeetY = playerCurrentPos.y + playerHeight;
                    float tolerance = 4.0f; // Margen de detección base

                    // Rango vertical base para detección
                    float minDetectY = troncoTopY - tolerance;
                    float maxDetectY = troncoTopY + tolerance;

                    //miramos con una tolerancia/margen
                    if (verticalDisp < 0) {
                        maxDetectY = troncoTopY + tolerance + FALL_STEP + 2.0f;

                    }

                    // Comprobar si los pies del jugador están en el rango vertical (ajustado o no)
                    bool verticalAlign = (playerFeetY >= minDetectY && playerFeetY <= maxDetectY);

                    if (verticalAlign) {
                        groundDetected = true;
                        surfaceY = troncoTopY - playerHeight; // Posición Y correcta sobre el tronco
                        break;
                    }
                }
            }
        }

        //comprovacion con el mapa
        if (!groundDetected)
        {
            glm::ivec2 posInt = glm::ivec2(posPlayer.x, posPlayer.y);
            int mapSurfaceY_int = posInt.y; // Pasamos la Y actual para que collisionMoveDown la corrija si es necesario

            //miramos colision con el suelo del mapa
            if (map->collisionMoveDown(posInt, glm::ivec2(32, 32), &mapSurfaceY_int))
            {
                // Hubo colisión con el mapa. La superficie Y ajustada está en mapSurfaceY_int
                // collisionMoveDown detectó colisión y ajustó mapSurfaceY_int
                groundDetected = true;
                surfaceY = float(mapSurfaceY_int); // Usamos el valor int ajustado
                // La superficie correcta es la que devuelve la función
            }
        }


        if (groundDetected)
        {
            onGround = true; // Marcar estado global para animación
            posPlayer.y = surfaceY; // Ajustar la posición Y real a la superficie detectada
            groundY = surfaceY;

            // Permitir iniciar salto si se presiona UP
            if (Game::instance().getKey(GLFW_KEY_UP))
            {
                bJumping = true;
                jumpAngle = 0;
                startY = posPlayer.y;
                onGround = false;
            }
        }
        else
        {

            onGround = false;
        }
    }


    // ---> INICIO: Lógica final de Animación <---
    int currentAnim = sprite->animation();

    if (bJumping) { // Si está saltando (o iniciando salto)
        if (Game::instance().getKey(GLFW_KEY_LEFT) || currentAnim == STAND_LEFT || currentAnim == MOVE_LEFT || currentAnim == JUMP_LEFT) {
            if (currentAnim != JUMP_LEFT) sprite->changeAnimation(JUMP_LEFT);
        }
        else {
            if (currentAnim != JUMP_RIGHT) sprite->changeAnimation(JUMP_RIGHT);
        }
    }
    else if (onGround) { // Si está en el suelo (mapa o tronco)
        if (movingHorizontally) { // Y se está moviendo horizontalmente
            if (Game::instance().getKey(GLFW_KEY_LEFT)) {
                if (currentAnim != MOVE_LEFT) sprite->changeAnimation(MOVE_LEFT);
            }
            else { // Asumimos derecha si movingHorizontally es true y no es izquierda
                if (currentAnim != MOVE_RIGHT) sprite->changeAnimation(MOVE_RIGHT);
            }
        }
        else { // Si está en el suelo y NO se mueve horizontalmente
            if (currentAnim == MOVE_LEFT || currentAnim == JUMP_LEFT || currentAnim == STAND_LEFT) {
                if (currentAnim != STAND_LEFT) sprite->changeAnimation(STAND_LEFT);
            }
            else {
                if (currentAnim != STAND_RIGHT) sprite->changeAnimation(STAND_RIGHT);
            }
        }
    }
    else { // Si no está saltando y no está en el suelo -> está cayendo
        // Forzar animación de salto/caída (similar a bJumping)
        if (Game::instance().getKey(GLFW_KEY_LEFT) || currentAnim == STAND_LEFT || currentAnim == MOVE_LEFT || currentAnim == JUMP_LEFT) {
            if (currentAnim != JUMP_LEFT) sprite->changeAnimation(JUMP_LEFT); // O una animación específica de caída
        }
        else {
            if (currentAnim != JUMP_RIGHT) sprite->changeAnimation(JUMP_RIGHT); // O una animación específica de caída
        }
    }
    // ---> FIN: Lógica final de Animación <---


    sprite->setCameraPosition(cameraPos);
    sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));
    cout << "Player position: " << posPlayer.x << " " << posPlayer.y << endl;
}

void Player::render(glm::mat4 modelview)
{
    // Solo renderizamos el sprite si está visible (para efecto de parpadeo)
    if (!isBlinking || visible) {
        sprite->render(modelview);
    }
}

void Player::setTileMap(TileMap* tileMap)
{
    map = tileMap;
}

void Player::setPosition(const glm::vec2& pos)
{
    posPlayer = pos;
    sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));
}

void Player::setCameraPosition(const glm::vec2& pos)
{
    cameraPos = pos;
}

void Player::takeDamage(int damage)
{
    if (hasHelmet) {
        if (helmet_usages > 0) {

            if (!health.getIsInvulnerable())--helmet_usages;
            damage = damage / 2;
        }
        else {
            hasHelmet = false;
            std::cout << "El casco se ha roto." << std::endl;
        }
    }
    health.takeDamage(damage);

    // AÑADIR ANIMACIONES/EFECTOS DE SONIDO AL RECIBIR DAÑO?? 

    // Comprobar si el jugador ha muerto
    if (!health.isAlive()) {
        if (clockscount > 0) {
            --clockscount;
            health.reset();
        }
    }
}


void Player::collectPumpkin() {
    pumpinkcount++;
    calculatAndUpdateMaxHealth();
}

void Player::calculatAndUpdateMaxHealth() {
    if (pumpinkcount == 9 || pumpinkcount == 12 || pumpinkcount == 16 || pumpinkcount == 22 || pumpinkcount == 30
        || pumpinkcount == 42 || pumpinkcount == 62 || pumpinkcount == 99) maxHealth = maxHealth + 20;
    health.setMaxHealth(maxHealth);
    //actualizamos la vida maxima si tenemos muchas calabazas
}

void Player::addHealth(int amount) {
    health.heal(amount);
}

void Player::healToMax() {
    health.reset();
}

void Player::equipHelmet() {
    //aunque ya tenga el casco, le reiniciamos los usos
    hasHelmet = true;
    helmet_usages = 4;
}

void Player::becomeInvulnerable(float duration) {
    health.beInvulnerable(duration);
    isBlinking = true;
    blinkTime = 5;
}



