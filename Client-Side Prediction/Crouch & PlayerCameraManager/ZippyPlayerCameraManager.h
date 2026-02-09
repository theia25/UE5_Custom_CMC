// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "ZippyPlayerCameraManager.generated.h"

UCLASS()
class ZIPPY_API AZippyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

private:
	// EditDefaultsOnly: 디테일 패널 수정 제어 (배치된 액터는 수정 불가)
	// BlueprintReadOnly: 블루프린트 그래프 접근 제어 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float CrouchBlendDuration = 0.5f;

	float CrouchBlendTime;

public:
	AZippyPlayerCameraManager();

	// FTViewTarget: Transition 정보 포함
	// 카메라 위치, 회전, FOV 등을 계산해서 최종 카메라 정보(POV)를 매 프레임 갱신
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

};
