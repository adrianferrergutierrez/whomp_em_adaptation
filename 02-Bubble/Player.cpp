#include <cmath>
#include <iostream>
#include <GL/glew.h>
#include "Player.h"
#include "Game.h"


#define JUMP_ANGLE_STEP 4
#define JUMP_HEIGHT 96
#define FALL_STEP 4


enum PlayerAnims
{
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, JUMP_LEFT, ATTACK_LEFT, ATTACK_RIGHT, ATTACK_STAND_LEFT, ATTACK_STAND_RIGHT, ATTACK_UP_LEFT, ATTACK_UP_RIGHT, ATTACK_DOWN
};
enum LanzaAnims {
	LEFT, RIGHT, VACIA
};

void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	isAttacking = false;
	izq = false;
	attackTimer = 0;

	bJumping = false;
	spritesheet.loadFromFile("images/indio6.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);
	spriteLanza = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);
	spriteLanza->setNumberAnimations(3);
	sprite->setNumberAnimations(13);

	sprite->setAnimationSpeed(ATTACK_DOWN, 8);
	sprite->addKeyframe(ATTACK_DOWN, glm::vec2(0.0, 0.875));


	sprite->setAnimationSpeed(ATTACK_UP_LEFT, 8); 
	sprite->addKeyframe(ATTACK_UP_LEFT, glm::vec2(0.75, 0.250));

	sprite->setAnimationSpeed(ATTACK_UP_RIGHT, 8);
	sprite->addKeyframe(ATTACK_UP_RIGHT, glm::vec2(0.25, 0.875));

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
	
	sprite->setAnimationSpeed(JUMP_RIGHT, 8);
	sprite->addKeyframe(JUMP_RIGHT, glm::vec2(0.25, 0.254));

	sprite->setAnimationSpeed(JUMP_LEFT, 8);
	sprite->addKeyframe(JUMP_LEFT, glm::vec2(0.5, 0.254));
	//l'animació va a 8 fps
		sprite->setAnimationSpeed(STAND_LEFT, 8);

	//quin cuadrat de la textura cal que agafis en l'animació.
		sprite->addKeyframe(STAND_LEFT, glm::vec2(0.75, 0.125));
		
		sprite->setAnimationSpeed(STAND_RIGHT, 8);
		sprite->addKeyframe(STAND_RIGHT, glm::vec2(0.0, 0.0));
		
		sprite->setAnimationSpeed(MOVE_LEFT, 8);
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.5, 0.125));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.25, 0.125));
		sprite->addKeyframe(MOVE_LEFT, glm::vec2(0.0, 0.125));
	//definim 8 fps y 3 keyframes.
		sprite->setAnimationSpeed(MOVE_RIGHT, 8);
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.25, 0.0));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.5, 0.0));
		sprite->addKeyframe(MOVE_RIGHT, glm::vec2(0.75, 0.0));

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



	//així cambiem la animació on cal que estigui, si ens movem a la dreta cambiarem l'animació...
	sprite->changeAnimation(0);
	spriteLanza->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
}


//en l'update mira que si s'ha apretat la key mirnat la classe game, que es static global (singleton), mira si hem premut la tecla.
//es podria fer una manera de inputs per a distribuirlos
void Player::update(int deltaTime)
{   

	sprite->update(deltaTime);
	if (isAttacking)
	{
		attackTimer -= deltaTime;
		if (attackTimer <= 0)
		{
			isAttacking = false;
			// Restaurar la animación correcta después del ataque
			if (Game::instance().getKey(GLFW_KEY_LEFT)) {
				sprite->changeAnimation(MOVE_LEFT);
				izq = true;
			}
			else if (Game::instance().getKey(GLFW_KEY_RIGHT)){
				sprite->changeAnimation(MOVE_RIGHT);
				izq = false;
			}
			else if (izq) sprite->changeAnimation(STAND_LEFT);
			else sprite->changeAnimation(STAND_RIGHT);
		} 
	}
	if (Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if (!bJumping&&!isAttacking) // Solo cambia la animación si NO está saltando
		{
			if (sprite->animation() != MOVE_LEFT)
				sprite->changeAnimation(MOVE_LEFT);
			izq = true;
		}
		posPlayer.x -= 2;

		if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || posPlayer.x == 0 || posPlayer.x == 2048 || posPlayer.x == 2310)
		{
			posPlayer.x += 2;
			if (!bJumping && !isAttacking) sprite->changeAnimation(STAND_LEFT); // Evita cambiar la animación si está en el aire
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if (!bJumping && !isAttacking) // Solo cambia la animación si NO está saltando
		{
			if (sprite->animation() != MOVE_RIGHT)
				sprite->changeAnimation(MOVE_RIGHT);
			izq = false;
		}
		posPlayer.x += 2;

		if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			if (!bJumping && !isAttacking) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_UP) && !isAttacking &&!bJumping)
	{
		if (izq) sprite->changeAnimation(ATTACK_UP_LEFT);
		else sprite->changeAnimation(ATTACK_UP_RIGHT);

	}
	else if (Game::instance().getKey(GLFW_KEY_DOWN) && !bJumping && !isAttacking)
	{

		if (izq) sprite->changeAnimation(JUMP_LEFT);
		else sprite->changeAnimation(JUMP_RIGHT);

	}

	else
	{
		if (!bJumping && !isAttacking) // Si está en el aire, no cambia a STAND
		{
			if (izq) sprite->changeAnimation(STAND_LEFT);

			else sprite->changeAnimation(STAND_RIGHT);
		}
	}

	
	if(bJumping)
	{
		jumpAngle += JUMP_ANGLE_STEP;
		if(jumpAngle == 180)
		{
			bJumping = false;
			posPlayer.y = startY;
			// Al aterrizar, cambia a STAND_LEFT o STAND_RIGHT según la última animación de salto
			if (sprite->animation() == JUMP_LEFT)
				sprite->changeAnimation(STAND_LEFT);
			else
				sprite->changeAnimation(STAND_RIGHT);
		}
		else
		{
			posPlayer.y = int(startY - 96 * sin(3.14159f * jumpAngle / 180.f));
			if(jumpAngle > 90)
				bJumping = !map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y);

			if (Game::instance().getKey(GLFW_KEY_DOWN) && !isAttacking)
			{
				sprite->changeAnimation(ATTACK_DOWN);
			}
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			if (Game::instance().getKey(GLFW_KEY_Z))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
				if (izq)
					sprite->changeAnimation(JUMP_LEFT);

				else
					sprite->changeAnimation(JUMP_RIGHT);
			}

		}
	}
	if (Game::instance().getKey(GLFW_KEY_X) && !isAttacking && !bJumping)
	{
		isAttacking = true;
		attackTimer = 100;
		if (izq) {
			if (sprite->animation() == MOVE_LEFT) {
				sprite->changeAnimation(ATTACK_LEFT);
				spriteLanza->changeAnimation(LEFT);
				lanzaOffset = glm::vec2(-29, 0);
			}
			else {
				sprite->changeAnimation(ATTACK_STAND_LEFT);
				spriteLanza->changeAnimation(LEFT);
				lanzaOffset = glm::vec2(-30, 0);
			}
		}
		else {
			if (sprite->animation() == MOVE_RIGHT) {
				sprite->changeAnimation(ATTACK_RIGHT);
				spriteLanza->changeAnimation(RIGHT);
				lanzaOffset = glm::vec2(28, 1);
			}
			else {
				sprite->changeAnimation(ATTACK_STAND_RIGHT);
				spriteLanza->changeAnimation(RIGHT);
				lanzaOffset = glm::vec2(29, 1);
			}
		}
	}

	if (isAttacking)
	{
		spriteLanza->setPosition(glm::vec2(float(posPlayer.x + lanzaOffset.x), float(posPlayer.y + lanzaOffset.y)));
		spriteLanza->setCameraPosition(cameraPos);
	}

	sprite->setCameraPosition(cameraPos);
	sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));
}

void Player::render(glm::mat4 modelview)
{
	sprite->render(modelview);
	if(isAttacking) spriteLanza->render(modelview);
}

void Player::setTileMap(TileMap *tileMap)
{
	map = tileMap;
}

void Player::setPosition(const glm::vec2 &pos)
{
	posPlayer = pos;
	sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));
}
void Player::setCameraPosition(const glm::vec2& pos)
{
	cameraPos = pos;
}




