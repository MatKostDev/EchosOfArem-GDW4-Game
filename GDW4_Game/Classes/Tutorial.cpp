#include "Tutorial.h"
#include "Gameplay.h"
#include <iostream>
#include "HeroStateManager.h"

cocos2d::Scene* Tutorial::createScene()
{
	Scene* scene = Tutorial::create();
	return scene;
}

bool Tutorial::init()
{
	if (!Scene::init())
		return false;

	srand(time(NULL)); //seed rng
	director = Director::getInstance();
	//Setting the default animation rate for the director
	director->setAnimationInterval(1.0f / 60.0f);
	director->setDisplayStats(1); //Remove this after debugging

	initGameObjects();
	initSprites();
	initListeners();

	scheduleUpdate();

	return true;
}

//initializes the user interface
void Tutorial::initUI()
{

}

void Tutorial::initGameObjects()
{
	//set bounds for the scene
	GameObject::MAX_X = 1920.0f;
	GameObject::MAX_Y = 1080.0f;
	
	Hero::hero->sprite->setPosition(Vec2(20.0f, 400.0f)); //set initial position
}

void Tutorial::initSprites()
{
	//add background
	background = Sprite::create("Backgrounds/greyBackground.png");
	background->setAnchorPoint(Vec2(0.0f, 0.0f));
	this->addChild(background, 1);

	cocos2d::TMXTiledMap* testTileMap = TMXTiledMap::create("Tilemaps/tutTest.tmx"); //ayy it works
	addChild(testTileMap, 1);

	cocos2d::TMXLayer* groundLayer = testTileMap->getLayer("Ground");
	cocos2d::TMXLayer* platformLayer = testTileMap->getLayer("Platform");

	unsigned int tileMapWidth = testTileMap->getMapSize().width;   //map width
	unsigned int tileMapHeight = testTileMap->getMapSize().height; //map height
	for (unsigned int x = 0; x < tileMapWidth; x++)       //width of map grid
	{
		for (unsigned int y = 0; y < tileMapHeight; y++)  //height of map grid
		{
			cocos2d::Sprite* currentTile = groundLayer->getTileAt(Vec2(x, y));
			if (currentTile != NULL)
			{
				GroundTile* newGroundTile = new GroundTile(currentTile->getPosition(), 128);

				//set collision flags if there are adjacent ground tiles
				//we have to do our own x and y validation because cocos sucks and crashes otherwise
				if (x != 0)
				{
					if (groundLayer->getTileAt(Vec2(x - 1, y)) != NULL)
						newGroundTile->ignoreLeftCollision = true;
				}
				if (x != tileMapWidth - 1)
				{
					if (groundLayer->getTileAt(Vec2(x + 1, y)) != NULL)
						newGroundTile->ignoreRightCollision = true;
				}
				if (y != 0)
				{
					if (groundLayer->getTileAt(Vec2(x, y - 1)) != NULL)
						newGroundTile->ignoreTopCollision = true;
				}
				if (y != tileMapHeight - 1)
				{
					if (groundLayer->getTileAt(Vec2(x, y + 1)) != NULL)
						newGroundTile->ignoreBottomCollision = true;
				}
			}

			currentTile = platformLayer->getTileAt(Vec2(x, y));
			if (currentTile != NULL)
			{
				PlatformTile* newPlatformTile = new PlatformTile(currentTile->getPosition(), 128);
			}
		}
	}

	//add hero (singleton class)
	this->addChild(Hero::hero->sprite, 20);
	this->addChild(Hero::hero->arm, 21); //add hero arm
	Hero::hero->arm->setVisible(0); //make arm invisible to begin with

	//add hero hurtbox FOR TESTING PURPOSES
	testHurtbox = DrawNode::create();
	this->addChild(testHurtbox, 30);
	//add fire melee attack hixbox FOR TESTING PURPOSES
	testMeleeAttack = DrawNode::create();
	this->addChild(testMeleeAttack, 40);

	//add grapple (singleton class)
	this->addChild(Grapple::grapple, 5);
}

void Tutorial::initListeners()
{
	//Init the mouse listener
	initMouseListener();

	//Init the keyboard listener
	initKeyboardListener();
}

void Tutorial::initMouseListener()
{
	//Init the mouse listener
	mouseListener = EventListenerMouse::create();

	//On Mouse Down
	mouseListener->onMouseDown = CC_CALLBACK_1(Tutorial::mouseDownCallback, this);

	//On Mouse Up
	mouseListener->onMouseUp = CC_CALLBACK_1(Tutorial::mouseUpCallback, this);

	//On Mouse Move
	mouseListener->onMouseMove = CC_CALLBACK_1(Tutorial::mouseMoveCallback, this);

	//On Mouse Scroll
	mouseListener->onMouseScroll = CC_CALLBACK_1(Tutorial::mouseScrollCallback, this);

	//Add the mouse listener to the dispatcher
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void Tutorial::initKeyboardListener()
{
	//Create the keyboard listener
	keyboardListener = EventListenerKeyboard::create();

	//Setting up callbacks
	keyboardListener->onKeyPressed = CC_CALLBACK_2(Tutorial::keyDownCallback, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(Tutorial::keyUpCallback, this);

	//Add the keyboard listener to the dispatcher
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

//UPDATE
void Tutorial::update(float dt)
{
	Grapple::grapple->update(dt, this); //update grapple
	Hero::hero->update(dt); //update our hero
	//if (hero->invincibilityTimer > 0)
	//	flickerSprite(); //flicker sprite if it's invincible

	testHurtbox->clear();
	//DRAW HURTBOX FOR TESTING
	testHurtbox->drawSolidRect(Vec2(Hero::hero->hurtBox.origin.x, Hero::hero->hurtBox.origin.y),
		Vec2(Hero::hero->hurtBox.origin.x + Hero::hero->hurtBox.size.width,
			Hero::hero->hurtBox.origin.y + Hero::hero->hurtBox.size.height),
		Color4F(1.0f, 0.0f, 0.0f, 0.f));
	//DRAW MOVEBOX FOR TESTING
	testHurtbox->drawSolidRect(Vec2(Hero::hero->moveBox.origin.x, Hero::hero->moveBox.origin.y),
		Vec2(Hero::hero->moveBox.origin.x + Hero::hero->moveBox.size.width,
			Hero::hero->moveBox.origin.y + Hero::hero->moveBox.size.height),
		Color4F(0.0f, 1.0f, 0.0f, .0f));

	testMeleeAttack->clear();
	//DRAW MELEE ATTACK HITBOX FOR TESTING
	testMeleeAttack->drawSolidRect(HeroAttackManager::currentAttack->hitbox.origin,
		Vec2(HeroAttackManager::currentAttack->hitbox.getMaxX(), HeroAttackManager::currentAttack->hitbox.getMaxY()),
		Color4F(1.0f, 0.7f, 0.8f, 0.3f));

	spawnEnemies();     //spawn enemies if needed 
	updateObjects(dt);  //update objects
	updateEnemies(dt);  //update enemies

	//check if we should move to the next scene
	if (Hero::hero->moveBox.getMaxX() >= 1915)
	{
		this->removeAllChildrenWithCleanup(true);
		TileBase::deleteAllTiles();
		director->replaceScene(Gameplay::createScene());
		//director->replaceScene(TransitionFade::create(0.5, Gameplay::createScene(), Color3B(255, 255, 255)));
	}
}

void Tutorial::spawnEnemies()
{
	//spawns all enemies to keep a certain amount of each in the map

}

void Tutorial::updateObjects(float dt)
{
	//update all platforms
	unsigned int numPlatforms = Platform::platformList.size();
	for (unsigned int i = 0; i < numPlatforms; i++)
		Platform::platformList[i]->update();

	//update all ice projectiles
	for (unsigned int i = 0; i < IceProjectile::iceProjectileList.size(); i++)
		IceProjectile::iceProjectileList[i]->update(dt);
}

void Tutorial::updateEnemies(float dt)
{

}

//removes all game objects from the world
void Tutorial::removeAllObjects()
{

}

//flickers sprite every 1/10th of a second (typically to display invincibility)
void Tutorial::flickerSprite()
{
	//if (((int)(ship->invincibilityTimer * 10)) % 2 == 1)
	//	ship->sprite->setZOrder(0); //flicker the ship (hide it behind background)
	//else
	//	ship->sprite->setZOrder(10); //show the ship again
}

//--- Callbacks ---//

void Tutorial::mouseDownCallback(Event* event)
{
	//Cast the event as a mouse event
	EventMouse* mouseEvent = dynamic_cast<EventMouse*>(event);
	auto mouseButton = mouseEvent->getMouseButton();

	if (mouseButton == cocos2d::EventMouse::MouseButton::BUTTON_LEFT)
	{
		HeroAttackManager::setCurrentAttack(HeroAttackTypes::meleeFireA, nullptr); //can pass a nullptr because we dont need to add anything to the scene for melee attacks
	}
	if (mouseButton == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
	{
		//Get the position of the mouse button press
		auto mouseClickPosition = mouseEvent->getLocationInView();
		mouseClickPosition.y += 1080;

		auto mouseGameViewPosition = mouseClickPosition;
		//do some simple math to convert mouse click position on screen to in-game world position
		//mouseGameViewPosition -= Vec2(1920 / 2, 1080 / 2); //update if screen size changes
		//mouseGameViewPosition += Hero::hero->sprite->getPosition();

		Grapple::grapple->shoot(Vect2(mouseGameViewPosition)); //shoot the grapple
		HeroStateManager::shootingGrapple->onEnter(); //put hero in grapple state
	}
}

void Tutorial::mouseUpCallback(Event* event)
{
}

void Tutorial::mouseMoveCallback(Event* event)
{
}

void Tutorial::mouseScrollCallback(Event* event)
{
}

void Tutorial::keyDownCallback(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_A:
		HeroStateManager::currentState->handleInput(InputType::p_a);
		Hero::hero->lookState = Hero::LookDirection::lookingLeft;
		Hero::hero->moveState = Hero::MoveDirection::movingLeft;
		break;

	case EventKeyboard::KeyCode::KEY_D:
		HeroStateManager::currentState->handleInput(InputType::p_d);
		Hero::hero->lookState = Hero::LookDirection::lookingRight;
		Hero::hero->moveState = Hero::MoveDirection::movingRight;
		break;

	case EventKeyboard::KeyCode::KEY_S:
		HeroStateManager::currentState->handleInput(InputType::p_s);
		HeroAttackBase::isSKeyHeld = true;
		break;

	case EventKeyboard::KeyCode::KEY_W:
		HeroAttackBase::isWKeyHeld = true;
		break;

	case EventKeyboard::KeyCode::KEY_SPACE:
		HeroStateManager::currentState->handleInput(InputType::p_space);
		break;

		//ATTACKS FOR TESTING. TODO: remove later and set to proper keybinds (numbers to swap between attacks?)
	case EventKeyboard::KeyCode::KEY_Q:
		HeroAttackManager::setCurrentAttack(HeroAttackTypes::meleeFireA, nullptr); //scene can be nullptr since we dont actually add anything to the scene in melee attacks
		break;

	case EventKeyboard::KeyCode::KEY_E:
		HeroAttackManager::setCurrentAttack(HeroAttackTypes::projectileIceA, this);
		break;

		//FOR TESTING
	case EventKeyboard::KeyCode::KEY_I:
		Grapple::grapple->testCase = !Grapple::grapple->testCase;
		break;
	}
}

void Tutorial::keyUpCallback(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_A:
		//make sure the hero is moving in this direction before we decide if they are now idle
		if (Hero::hero->moveState == Hero::MoveDirection::movingLeft)
			Hero::hero->moveState = Hero::MoveDirection::idle;
		break;

	case EventKeyboard::KeyCode::KEY_D:
		//make sure the hero is moving in this direction before we decide if they are now idle
		if (Hero::hero->moveState == Hero::MoveDirection::movingRight)
			Hero::hero->moveState = Hero::MoveDirection::idle;
		break;

	case EventKeyboard::KeyCode::KEY_S:
		HeroAttackBase::isSKeyHeld = false;
		break;

	case EventKeyboard::KeyCode::KEY_W:
		HeroAttackBase::isWKeyHeld = false;
		break;

	case EventKeyboard::KeyCode::KEY_SPACE:
		HeroStateManager::currentState->handleInput(InputType::r_space);
		break;
	}
}