#include "pch.h"

#include "MoveComponent.h"
#include "UserData.h"

MoveComponent::MoveComponent()
{
	moveFunctions[0] = &MoveLeft;
	moveFunctions[1] = &MoveUp;
	moveFunctions[2] = &MoveRight;
	moveFunctions[3] = &MoveDown;
}