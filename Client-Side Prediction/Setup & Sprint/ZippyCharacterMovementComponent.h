// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ZippyCharacterMovementComponent.generated.h"

/*
* 클라이언트 예측 이동 시스템 파이프라인 요약 (변수 -> 저장 -> 압축 -> 전송 -> 해제 -> 적용)
* 1. 틱 실행: 매 틱마다 PerformMove()를 호출하여 이동 로직을 실행
* 2. 데이터 수집(Capture): 이동 로직은 Safe_bWantsToSprint 값을 읽고 활용
* 3. 스냅샷 저장: FSavedMove_Zippy를 생성하고 SetMoveFor()를 사용하여 
* - Safe_bWantsToSprint 값을 읽어 Saved_bWantsToSprint에 저장
* 4. 최적화: CanCombineWith()를 호출하여 방금 실행한 이동과 비슷한 서버로 전송될 예정인 Pending Move가 있는지 확인하고 필요한 경우 결합
* 5. 압축: GetCompressedFlags()를 호출하여 저장된 동작을 서버로 전송할 수 있는 작은 네트워크 패킷으로 압축
* 6. 서버 수신 및 해제: 서버가 이동을 수신하면 UpdateFromCompressedFlags()를 호출하여 방금 전송된 압축된 플래그를 사용하여 클라이언트의 Safe_bWantsToSprint 상태 변수를 업데이트, 복원한다.
* 7. 서버 실행: 복원된 상태를 가지고 클라이언트가 수행한 이동을 서버에서 그대로 실행
* - 이동 직전에 PrepMoveFor()를 호출하여 플래그에 담기지 않은 나머지 데이터나 저장된 이동의 구체적인 세팅을 적용
*/

/*
* 서버가 클라이언트가 보낸 패킷을 받았을 때 데이터 복원 순서
* 1. UpdateFromCompressedFlags() 압축된 비트 정보를 품
* 2. PrepMoveFor() FSavedMove의 데이터를 실제 컴포넌트에 적용해서 이동 준비 완료
* 3. MoveAutonomous() 실제 이동 로직 실행
*/

UCLASS()
class ZIPPY_API UZippyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UZippyCharacterMovementComponent();

	// 서버로 전송된 클라이언트의 저장된 이동 의미
	// - 매 프레임마다 동작 클래스를 저장하고 서버에서 동일한 동작을 수행할 수 있도록 복제
	class FSavedMove_Zippy : public FSavedMove_Character
	{
	public:
		// 저장된 동작 클래스 Flags
		// - 새로운 동작을 생성할 때마다 자동으로 로컬 값으로 업데이트
		// - 안전한 동작을 수행해야 할 때마다 로컬 값에 복사
		uint8 Saved_bWantsToSprint : 1;

		// 현재 동작과 새로운 동작을 비교하여 두 동작을 결합할 수 있는지 확인하는 함수
		// - 대역폭을 절약하기 위해서 저장된 동작의 모든 데이터가 거의 동일한지 여부를 판단 
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		// 저장된 동작 클래스 초기화
		virtual void Clear() override;

		// 움직임을 수행할 때마다 매 프레임마다 모든 것을 서버로 복제하는 대신 간소화된 압축된 플래그만 전송
		virtual uint8 GetCompressedFlags() const override;

		// CMC의 상태 데이터 캡처, 현재 스냅샷에 대한 저장된 이동 값 설정
		// - 캐릭터 이동 컴포넌트에 필요한 모든 변수를 확인하고 각각의 저장된 변수 설정
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		// 저장된 동작의 데이터를 가져와 캐릭터 이동 컴포넌트의 현재 상태에 적용
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	// 기본 캐릭터 동작(FSavedMove_Character) 대신 사용자 지정 동작(FSavedMove_Zippy)을 사용하겠다고 알림 
	class FNetworkPredictionData_Client_Zippy : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Zippy(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		// 새로운 지정 동작 할당
		virtual FSavedMovePtr AllocateNewMove() override;
	};

	// MaxWalkSpeed는 캐릭터 클래스 고유 변수명
	UPROPERTY(EditDefaultsOnly)
	float Sprint_MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly)
	float Walk_MaxWalkSpeed;

	// 클라이언트가 Sprint키를 누를 때마다 설정되는 플래그
	// - 이 값이 서버로 전달되고 서버는 해당 데이터를 처리
	// - 이동 안전성 속성을 갖는다.
	bool Safe_bWantsToSprint;

public:
	// 클라이언트 예측 데이터 1회 생성, 캐시 된 예측 값 반환
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	// 압축 플래그 값 업데이트, 캐릭터 이동 컴포넌트의 상태 설정
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	// 모든 동작 수행이 끝날 때 자동으로 호출되는 함수
	// - 새로운 이동 모드 설정
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

#pragma region 키 입력 토글
public:
	// 클라이언트에서만 호출 => 이동 안전성 속성이 깨지지 않는다.
	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

#pragma endregion 키 입력 토글

};
