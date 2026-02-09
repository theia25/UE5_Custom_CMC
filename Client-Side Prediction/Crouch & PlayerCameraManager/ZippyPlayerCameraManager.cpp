// Fill out your copyright notice in the Description page of Project Settings.

#include "ZippyPlayerCameraManager.h"

#include "ZippyCharacter.h"
#include "ZippyCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AZippyPlayerCameraManager::AZippyPlayerCameraManager()
{
}

void AZippyPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// 로직과 비주얼 분리(앉기 판정은 즉시, 카메라는 부드럼게 이동)
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (AZippyCharacter* ZippyCharacter = Cast<AZippyCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UZippyCharacterMovementComponent* ZMC = ZippyCharacter->GetZippyCharacterMovement();

		// 앉았을 때 카메라가 내려가야 하는 높이(음수 값) 계산
		FVector TargetCrouchOffset = FVector(
			0, 
			0, 
			// 캐릭터 무브먼트의 웅크린 절반 높이(60) - 캡슐 컴포넌트의 캡슐 절반 높이(96) = -36만큼 아래로 내려가야 한다.
			ZMC->GetCrouchedHalfHeight() - ZippyCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);

		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (ZMC->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);

			// 핵심: 카메라 순간이동 상쇄
			// - 엔진은 이미 카메라를 내렸기 때문에 코드로 다시 올려줌 (0 - (-36) = +36)
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		// 앉았다가 일어날 때는 -36높이에서 0으로 카메라 위치 이동
		// - 엔진은 즉시 캡슐 높이가 커지고 캐릭터 위치와 카메라도 +36만큼 튀어오름
		OutVT.POV.Location += Offset;
		
	}
}
