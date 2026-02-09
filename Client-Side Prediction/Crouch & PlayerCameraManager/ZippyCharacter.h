#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ZippyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AZippyCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	class UZippyCharacterMovementComponent* ZippyCharacterMovementComponent;

  // 추가된 부분
pulic:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UZippyCharacterMovementComponent* GetZippyCharacterMovement() const { return ZippyCharacterMovementComponent; }

	FCollisionQueryParams GetIgnoreCharacterParams() const;
};
