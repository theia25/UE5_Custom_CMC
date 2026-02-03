// Fill out your copyright notice in the Description page of Project Settings.


#include "ZippyCharacterMovementComponent.h"

#include "GameFramework/Character.h"

UZippyCharacterMovementComponent::UZippyCharacterMovementComponent()
{
}

bool UZippyCharacterMovementComponent::FSavedMove_Zippy::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Zippy* NewZippyMove = static_cast<FSavedMove_Zippy*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewZippyMove->Saved_bWantsToSprint)
	{
		return false;
	}

	// 기존 압축 플래그와 비교해서 방금 실행한 이동과 서버로 전송될 예정인 Pending Move를 확인, 필요한 경우 결합
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

// 상태 초기화
void UZippyCharacterMovementComponent::FSavedMove_Zippy::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
}

// 클라이언트가 이동 데이터를 복제하는 방식
uint8 UZippyCharacterMovementComponent::FSavedMove_Zippy::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	// 매 프레임마다 전송해야 하는 필수 데이터만 사용 (벡터나 큰 데이터는 RPC 사용)
	if (Saved_bWantsToSprint)
		Result |= FLAG_Custom_0;

	// 서버로 전송할 수 있는 작은 네트워크 패킷으로 압축
	// - 서버 수신 시 UpdateFromCompressedFlags()호출
	return Result;
}

void UZippyCharacterMovementComponent::FSavedMove_Zippy::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UZippyCharacterMovementComponent* CharacterMovement = Cast<UZippyCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

void UZippyCharacterMovementComponent::FSavedMove_Zippy::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UZippyCharacterMovementComponent* CharacterMovement = Cast<UZippyCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

FNetworkPredictionData_Client* UZippyCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UZippyCharacterMovementComponent* MutableThis = const_cast<UZippyCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Zippy(*this);

		// 서버-클라이언트 위치 부드럽게 보간 가능한 최대 거리
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;

		// NoSmoothNetUpdateDist: 서버-클라이언트의 위치 차이가 140.f보다 크면 즉시 텔레포트
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UZippyCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	// 압축된 플래그를 사용하여 클라이언트의 상태 변수 업데이트
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UZippyCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking)
	{
		if (Safe_bWantsToSprint)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}

void UZippyCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UZippyCharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
}

UZippyCharacterMovementComponent::FNetworkPredictionData_Client_Zippy::FNetworkPredictionData_Client_Zippy(const UCharacterMovementComponent& ClientMovement) 
	: Super(ClientMovement)
{
}

FSavedMovePtr UZippyCharacterMovementComponent::FNetworkPredictionData_Client_Zippy::AllocateNewMove()
{
	// new 사용 이유: FSavedMove_Character구조체가 UObject를 상속받지 않는 일반 C++ 클래스이기 때문이다.
	// GC 대신 별도 클래스를 추가하는 방식으로 해결하는 이유
	// - 언리얼 GC는 무겁기 때문에 매 프레임 생성되는 이동 데이터까지 관리하면 오버헤드가 너무 크다.
	// - 대신 FSavedMovePtr(스마트 포인터, TSharedPtr)를 사용하여 참조 카운팅 방식으로 메모리를 스스로 관리함
	return FSavedMovePtr(new FSavedMove_Zippy());
}
