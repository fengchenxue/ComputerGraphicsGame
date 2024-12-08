#include "Object.h"

Animation NPC::animation;
Skeleton NPC::skeleton;

Player::Player()
{
	animationInstance.animation = &animation;
}

NPC::NPC()
{
	animationInstance.animation = &animation;
}
