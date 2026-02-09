#include "ZippyCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UZippyCharacterMovementComponent::UZippyCharacterMovementComponent()
{
	// FNavAgentProperties를 통해 Crouch 기능을 지원한다.
	NavAgentProps.bCanCrouch = true;
} 

void UZippyCharacterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = ~bWantsToCrouch;
}
