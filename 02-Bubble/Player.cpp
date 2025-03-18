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
	STAND_LEFT, STAND_RIGHT, MOVE_LEFT, MOVE_RIGHT, JUMP_RIGHT, JUMP_LEFT
};


void Player::init(const glm::ivec2 &tileMapPos, ShaderProgram &shaderProgram)
{
	bJumping = false;
	spritesheet.loadFromFile("images/indio.png", TEXTURE_PIXEL_FORMAT_RGBA);
	sprite = Sprite::createSprite(glm::ivec2(32, 32), glm::vec2(0.25, 0.125), &spritesheet, &shaderProgram);

	sprite->setNumberAnimations(6);

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

	//així cambiem la animació on cal que estigui, si ens movem a la dreta cambiarem l'animació...
	sprite->changeAnimation(0);
	tileMapDispl = tileMapPos;
	sprite->setPosition(glm::vec2(float(tileMapDispl.x + posPlayer.x), float(tileMapDispl.y + posPlayer.y)));
	
}


//en l'update mira que si s'ha apretat la key mirnat la classe game, que es static global (singleton), mira si hem premut la tecla.
//es podria fer una manera de inputs per a distribuirlos
void Player::update(int deltaTime)
{   

	sprite->update(deltaTime);
	if (Game::instance().getKey(GLFW_KEY_LEFT))
	{
		if (!bJumping) // Solo cambia la animación si NO está saltando
		{
			if (sprite->animation() != MOVE_LEFT)
				sprite->changeAnimation(MOVE_LEFT);
		}
		posPlayer.x -= 2;

		if (map->collisionMoveLeft(posPlayer, glm::ivec2(32, 32)) || posPlayer.x == 0 || posPlayer.x == 2048 || posPlayer.x == 2310)
		{
			posPlayer.x += 2;
			if (!bJumping) sprite->changeAnimation(STAND_LEFT); // Evita cambiar la animación si está en el aire
		}
	}
	else if (Game::instance().getKey(GLFW_KEY_RIGHT))
	{
		if (!bJumping) // Solo cambia la animación si NO está saltando
		{
			if (sprite->animation() != MOVE_RIGHT)
				sprite->changeAnimation(MOVE_RIGHT);
		}
		posPlayer.x += 2;

		if (map->collisionMoveRight(posPlayer, glm::ivec2(32, 32)))
		{
			posPlayer.x -= 2;
			if (!bJumping) sprite->changeAnimation(STAND_RIGHT);
		}
	}
	else
	{
		if (!bJumping) // Si está en el aire, no cambia a STAND
		{
			if (sprite->animation() == MOVE_LEFT)
				sprite->changeAnimation(STAND_LEFT);
			else if (sprite->animation() == MOVE_RIGHT)
				sprite->changeAnimation(STAND_RIGHT);
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
		}
	}
	else
	{
		posPlayer.y += FALL_STEP;
		if(map->collisionMoveDown(posPlayer, glm::ivec2(32, 32), &posPlayer.y))
		{
			if (Game::instance().getKey(GLFW_KEY_UP))
			{
				bJumping = true;
				jumpAngle = 0;
				startY = posPlayer.y;
				if (sprite->animation() == MOVE_LEFT || sprite->animation() == STAND_LEFT)
					sprite->changeAnimation(JUMP_LEFT);
				else
					sprite->changeAnimation(JUMP_RIGHT);
			}

		}
	}
	sprite->setCameraPosition(cameraPos);
	sprite->setPosition(glm::vec2(float(posPlayer.x), float(posPlayer.y)));
	cout << "Player position: " << posPlayer.x << " " << posPlayer.y << endl;
}

void Player::render(glm::mat4 modelview)
{
	sprite->render(modelview);
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




