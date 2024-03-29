#include "HeroStateManager.h"

AttackingState* HeroStateManager::attacking = new AttackingState();
DyingState* HeroStateManager::dying = new DyingState();
FallingState* HeroStateManager::falling = new FallingState();
GrapplingState* HeroStateManager::grappling = new GrapplingState();
IdleState* HeroStateManager::idle = new IdleState();
JumpingState* HeroStateManager::jumping = new JumpingState();
RunningState* HeroStateManager::running = new RunningState();
GrappleJumpState* HeroStateManager::grappleJumping = new GrappleJumpState();
HoldingPlatformState* HeroStateManager::holdingPlatform = new HoldingPlatformState();
ShootingGrappleState* HeroStateManager::shootingGrapple = new ShootingGrappleState();

HeroStateBase* HeroStateManager::currentState = idle; //initialize to idle state


HeroStateManager::HeroStateManager()
{
}


HeroStateManager::~HeroStateManager()
{
}