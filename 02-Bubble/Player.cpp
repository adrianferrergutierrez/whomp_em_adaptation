#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"
#include "FireStickProjectile.h"
#include "AudioManager.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 16*3 // 3 tiles
#define FALL_STEP 3

enum PlayerAnims
{
    STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, JUMP_LEFT,
    ATTACK_LEFT, ATTACK_RIGHT, ATTACK_STAND_LEFT, ATTACK_STAND_RIGHT,
    ATTACK_UP_LEFT, ATTACK_UP_RIGHT, ATTACK_DOWN, DAMAGE, VICTORY
};

enum LanzaAnims {
    LEFT, RIGHT, VACIA
};

void Player::init(const glm::ivec2& tileMapPos, ShaderProgram& shaderProgram)
{
    this->shaderProgram = &shaderProgram;



    // Inicialización del sistema de salud, items y daño
    maxHealth = 120;
    health = Health(maxHealth);

    pumpinkcount = 0;
    clockscount = 2;

    damage = 10;

    isBlinking = false;
    blinkTime = 0;
    visible = true;

    // Inicialización del sistema de ataque
    isAttacking = false;
    izq = false;
    attackTimer = 0;
    fireusages = 3;
    firetimer = 0;

    // Inicialización del jugador
    isGOD = false;
    bJumping = false;
    spritesheet.loadFromFile("images/indio6.png", TEXTURE_PIXEL_FORMAT_RGBA);

    sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);
    spriteLanza = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);

    spriteLanza->setNumberAnimations(3);
    sprite->setNumberAnimations(15);

    // Configuración de animaciones para la lanza
    spriteLanza->setAnimationSpeed(VACIA, 8);
    spriteLanza->addKeyframe(VACIA, glm::vec2(0.5, 0.875));

    spriteLanza->setAnimationSpeed(RIGHT, 8);
    spriteLanza->addKeyframe(RIGHT, glm::vec2(0.75, 0.625));
    spriteLanza->addKeyframe(RIGHT, glm::vec2(0.5, 0.625));
    spriteLanza->addKeyframe(RIGHT, glm::vec2(0.25, 0.625));

    spriteLanza->setAnimationSpeed(LEFT, 8);
    spriteLanza->addKeyframe(LEFT, glm::vec2(0.5, 0.750));
    spriteLanza->addKeyframe(LEFT, glm::vec2(0.25, 0.750));
    spriteLanza->addKeyframe(LEFT, glm::vec2(0.0, 0.750));

    // Configuración de animaciones del personaje
    // Ataques
    sprite->setAnimationSpeed(ATTACK_DOWN, 8);
    sprite->addKeyframe(ATTACK_DOWN, glm::vec2(0.0, 0.875));

    sprite->setAnimationSpeed(ATTACK_UP_LEFT, 8);
    sprite->addKeyframe(ATTACK_UP_LEFT, glm::vec2(0.75, 0.250));

    sprite->setAnimationSpeed(ATTACK_UP_RIGHT, 8);
    sprite->addKeyframe(ATTACK_UP_RIGHT, glm::vec2(0.25, 0.875));

    sprite->setAnimationSpeed(ATTACK_LEFT, 8);
    sprite->addKeyframe(ATTACK_LEFT, glm::vec2(0.5, 0.5));
    sprite->addKeyframe(ATTACK_LEFT, glm::vec2(0.25, 0.5));
    sprite->addKeyframe(ATTACK_LEFT, glm::vec2(0.0, 0.5));

    sprite->setAnimationSpeed(ATTACK_RIGHT, 8);
    sprite->addKeyframe(ATTACK_RIGHT, glm::vec2(0.25, 0.375));
    sprite->addKeyframe(ATTACK_RIGHT, glm::vec2(0.5, 0.375));
    sprite->addKeyframe(ATTACK_RIGHT, glm::vec2(0.75, 0.375));

    sprite->setAnimationSpeed(ATTACK_STAND_LEFT, 8);
    sprite->addKeyframe(ATTACK_STAND_LEFT, glm::vec2(0.75, 0.5));

    sprite->setAnimationSpeed(ATTACK_STAND_RIGHT, 8);
    sprite->addKeyframe(ATTACK_STAND_RIGHT, glm::vec2(0.0, 0.375));

    // Salto
    sprite->setAnimationSpeed(JUMP_RIGHT, 8);
    sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.25, 0.254));

    sprite->setAnimationSpeed(JUMP_LEFT, 8);
    sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.5, 0.254));

    // Animación de estar quieto
    sprite->setAnimationSpeed(STAND_LEFT, 8);
    sprite->addKeyframe(STAND_LEFT, glm::vec2(0.75, 0.125));

    sprite->setAnimationSpeed(STAND_RIGHT, 8);
    sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0, 0.0));

    // Animación de movimiento
    sprite->setAnimationSpeed(MOVE_LEFT, 8);
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.5, 0.125));
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.25, 0.125));
    sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.0, 0.125));

    sprite->setAnimationSpeed(MOVE_RIGHT, 8);
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.0));
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5, 0.0));
    sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75, 0.0));

	sprite->setAnimationSpeed(DAMAGE, 8);
	sprite->addKeyframe(DAMAGE, glm::vec2(0.0, 0.125 * 5));

	sprite->setAnimationSpeed(VICTORY, 8);
	sprite->addKeyframe(VICTORY, glm::vec2(0.75, 0.125 * 6));

    // Inicialización de animaciones
    sprite->changeAnimation(0);
    spriteLanza->changeAnimation(0);
    tileMapDispl = tileMapPos;
    sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));

    // Variables para el casco
    hasHelmet = false;
    helmet_usages = 0;
}


void Player::update(int deltaTime, vector<Tronco*> troncos)
{
    // Actualizar el sistema de salud
    float deltaTimeSeconds = deltaTime / 1000.0f;
    health.update(deltaTimeSeconds);

 
    //por defecto no se esta protegiendo de arriba
    proteccion_superior = false;

    if (Game::instance().getKey(GLFW_KEY_T) && !victory_state) {
        if (fireusages > 0) {
            firemode = true;
            firetimer = 10000;
        }
    }

    if (firemode) {
        firetimer -= deltaTime;
        if (firetimer <= 0) {
            firemode = false;
            fireusages--;
        }
    }

    bool gKeyCurrentlyPressed = Game::instance().getKey(GLFW_KEY_G);
    if (gKeyCurrentlyPressed && !gKeyPressedLastFrame && !victory_state) {
        if (isGOD) {
            becomeHuman();
        }
        else {
            becomeGOD();
        }
    }
    gKeyPressedLastFrame = gKeyCurrentlyPressed;

    bool hkeyCurrentlyPressed = Game::instance().getKey(GLFW_KEY_H);
    if (hkeyCurrentlyPressed && !hKeyPressedLastFrame && !victory_state) {
        clockscount = 2;
        health.reset();
    }
    hKeyPressedLastFrame = hkeyCurrentlyPressed;

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

    sprite->update(deltaTime);

    // Sistema de ataque - Actualizar temporizador y estado
    if (isAttacking) {
        attackTimer -= deltaTime;
        if (attackTimer <= 0) {
            isAttacking = false;
            // Restaurar la animación correcta después del ataque
            if (Game::instance().getKey(GLFW_KEY_LEFT) && !victory_state) {
                sprite->changeAnimation(MOVE_LEFT);
                izq = true;
            }
            else if (Game::instance().getKey(GLFW_KEY_RIGHT) && !victory_state) {
                sprite->changeAnimation(MOVE_RIGHT);
                izq = false;
            }
            else if (izq) sprite->changeAnimation(STAND_LEFT);
            else sprite->changeAnimation(STAND_RIGHT);
        }
    }

    bool movingHorizontally = false; // Variable para saber si hay input de movimiento horizontal

    // ----Gestión de movimiento---
    if (Game::instance().getKey(GLFW_KEY_LEFT) && !victory_state)
    {
        if (!bJumping && !isAttacking) {
            if (sprite->animation() != MOVE_LEFT)
                sprite->changeAnimation(MOVE_LEFT);
            izq = true;
        }

        posPlayer.x -= 2;
        //Comprobamos colisiones con el mapa para la izquierda
        if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || posPlayer.x <= 0 || posPlayer.x == 2048 || posPlayer.x == 2310 || posPlayer.x == 3072 || posPlayer.x == 3328 || posPlayer.x == 3840)
        {
            //Cancelamos el movimiento
            posPlayer.x += 2;
            if (!bJumping && !isAttacking) sprite->changeAnimation(STAND_LEFT);
        }
        else {
            //Confirmamos el movimiento para la animación
            movingHorizontally = true;
        }
    }
    else if (Game::instance().getKey(GLFW_KEY_RIGHT) && !victory_state)
    {
        if (!bJumping && !isAttacking) {
            if (sprite->animation() != MOVE_RIGHT)
                sprite->changeAnimation(MOVE_RIGHT);
            izq = false;
        }

        posPlayer.x += 2;

        if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
        {
            //Rectificamos el movimiento, ha colisionado
            posPlayer.x -= 2;
            if (!bJumping && !isAttacking) sprite->changeAnimation(STAND_RIGHT);
        }
        else {
            //Confirmamos el movimiento a la derecha para la animación
            movingHorizontally = true;
        }
    }
    else if (Game::instance().getKey(GLFW_KEY_UP) && !isAttacking && !bJumping && !victory_state)
    {
        proteccion_superior = true;
        if (izq) sprite->changeAnimation(ATTACK_UP_LEFT);
        else sprite->changeAnimation(ATTACK_UP_RIGHT);
    }
    else if (Game::instance().getKey(GLFW_KEY_DOWN) && !bJumping && !isAttacking && !victory_state)
    {
        if (izq) sprite->changeAnimation(JUMP_LEFT);
        else sprite->changeAnimation(JUMP_RIGHT);
    }
    else
    {
        if (!bJumping && !isAttacking && !victory_state) {
            if (izq) sprite->changeAnimation(STAND_LEFT);
            else sprite->changeAnimation(STAND_RIGHT);
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
            // Al aterrizar, cambia a STAND_LEFT o STAND_RIGHT según la última animación de salto
            if (sprite->animation() == JUMP_LEFT)
                sprite->changeAnimation(STAND_LEFT);
            else
                sprite->changeAnimation(STAND_RIGHT);
        }
        else
        {
            posPlayer.y = int(startY - JUMP_HEIGHT * sin(3.14159f * jumpAngle / 180.f));

            // Si estamos en la fase de descenso y pulsamos DOWN, hacer ataque hacia abajo
            if (jumpAngle > 90 && Game::instance().getKey(GLFW_KEY_DOWN) && !isAttacking && !victory_state)
            {
                sprite->changeAnimation(ATTACK_DOWN);
            }

            // Comprobación de aterrizaje (solo cuando va hacia abajo)
            if (jumpAngle > 90)
            {
                // ---> INICIO: Comprobar aterrizaje en Tronco durante el salto <---
                bool landedOnTronco = false;
                float landingTroncoY = 0.f;
                for (Tronco* tronco : troncos) {
                    // Usamos una copia de posPlayer para la predicción, +1 para asegurar que esté "debajo"
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

            // Permitir iniciar salto si se presiona Z (key de salto)
            if (Game::instance().getKey(GLFW_KEY_Z) && !victory_state)
            {
                bJumping = true;
                jumpAngle = 0;
                startY = posPlayer.y;
                // Reproducir sonido de salto
                if (!AudioManager::getInstance()->init()) {
                    std::cerr << "Error al inicializar el sistema de audio" << std::endl;
          
                }
                else AudioManager::getInstance()->playSound("sounds/SE_marioland_jump.wav", 0.8f);
                if (izq)
                    sprite->changeAnimation(JUMP_LEFT);
                else
                    sprite->changeAnimation(JUMP_RIGHT);
                onGround = false;
            }
        }
        else
        {
            onGround = false;
        }
    }

    // Gestión de ataque con la tecla X
    if (Game::instance().getKey(GLFW_KEY_X) && !isAttacking && !bJumping && !victory_state)
    {
        isAttacking = true;
        attackTimer = 300;
        if (!AudioManager::getInstance()->init()) {
            std::cerr << "Error al inicializar el sistema de audio" << std::endl;

        }
        else AudioManager::getInstance()->playSound("sounds/hit.mp3", 0.8f);
        glm::vec2 projPos;
        int projDirection;

        if (izq) {
            if (sprite->animation() == MOVE_LEFT) {
                sprite->changeAnimation(ATTACK_LEFT);
            }
            else {
                sprite->changeAnimation(ATTACK_STAND_LEFT);
            }
            spriteLanza->changeAnimation(LEFT);
            lanzaOffset = glm::vec2(-30, 0);

            projPos = glm::vec2(float(posPlayer.x + lanzaOffset.x + 5), float(posPlayer.y + lanzaOffset.y + 10));
            projDirection = -1;
        }
        else {
            if (sprite->animation() == MOVE_RIGHT) {
                sprite->changeAnimation(ATTACK_RIGHT);
            }
            else {
                sprite->changeAnimation(ATTACK_STAND_RIGHT);
            }
            spriteLanza->changeAnimation(RIGHT);
            lanzaOffset = glm::vec2(29, 1);

            projPos = glm::vec2(float(posPlayer.x + lanzaOffset.x + 5), float(posPlayer.y + lanzaOffset.y + 10));
            projDirection = 1;
        }
        if (firemode) {
            if (!AudioManager::getInstance()->init()) {
                std::cerr << "Error al inicializar el sistema de audio" << std::endl;

            }
            else AudioManager::getInstance()->playSound("sounds/SE_LinkGuardGB.wav", 0.8f);
            // ⚠️ Crear el proyectil y añadirlo al vector
            FireStickProjectile* projectile = new FireStickProjectile();
            projectile->init(projPos, *shaderProgram, projDirection);
            projectile->setTileMap(map);
            projectiles.push_back(projectile);
        }
    }

    // Actualizar proyectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); )
    {
        FireStickProjectile* proj = *it;
        proj->update(deltaTime);
        if (!proj->isActive()) {
            delete proj;
            it = projectiles.erase(it);
        }
        else {
            ++it;
        }
    }


    // Actualizar posición de la lanza si está atacando
    if (isAttacking)
    {
        spriteLanza->update(deltaTime);
        lanzaPos = glm::vec2(float(posPlayer.x + lanzaOffset.x), float(posPlayer.y + lanzaOffset.y));
        spriteLanza->setPosition(lanzaPos);
        spriteLanza->setCameraPosition(cameraPos);
    }
    if (victory_state && onGround)sprite->changeAnimation(VICTORY);
    else if (tiempoDamageTemporal - deltaTimeSeconds > 0) {
        sprite->changeAnimation(DAMAGE);
		tiempoDamageTemporal -= deltaTimeSeconds;
    }
    
    sprite->setCameraPosition(cameraPos);
    sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));

    // Debug info
    std::cout << "Player position: " << posPlayer.x << " " << posPlayer.y << ", Health: "
        << health.getCurrentHealth() << " Invulnerable: " << health.getIsInvulnerable() << std::endl;
}

void Player::render(glm::mat4 modelview)
{
    // Solo renderizamos el sprite si está visible (para efecto de parpadeo)
    if (!isBlinking || visible) {
        sprite->render(modelview);
        if (isAttacking) spriteLanza->render(modelview);
    }
    for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
        (*it)->render(modelview);
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

// ----- Sistema de salud y daño -----

void Player::takeDamage(int dmg)
{
    if (hasHelmet) {
        if (helmet_usages > 0) {
            if (!health.getIsInvulnerable()) --helmet_usages;
            dmg = dmg / 2;
        }
        else {
            hasHelmet = false;
            std::cout << "El casco se ha roto." << std::endl;
        }
    }
    if (!health.getIsInvulnerable()) {
        sprite->changeAnimation(DAMAGE);
        tiempoDamageTemporal = tiempoDamage;

    }

    health.takeDamage(dmg,false);
   

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

void Player::becomeGOD() {
    isGOD = true;
    health.beInvulnerable(1000000.0f);
}
void Player::becomeHuman() {
    isGOD = false;
    health.beInvulnerable(0.0f);
}

bool Player::isInFireMode() const {
    return firemode;
}

Player::~Player() {
    // Limpiar todos los proyectiles
    for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
        delete (*it);
    }
    projectiles.clear();
    // Otros recursos a liberar...
}


void Player::setVictory() {
    victory_state = true;
}
