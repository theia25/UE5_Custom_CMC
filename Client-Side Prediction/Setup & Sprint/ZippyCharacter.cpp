###생성자 변경
// ACharacter 생성자를 호출하기 전에 FObjectInitializer를 통해 설정 조작
// - 엔진이 제공하는 CMC 대신 UZippyCharacterMovementComponent로 클래스 생성 요청
AZippyCharacter::AZippyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZippyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 엔진이 제공하는 CharaccterMovementComponent를 내가 만든 Zippy CMC로 교체
	ZippyCharacterMovementComponent = Cast<UZippyCharacterMovementComponent>(GetCharacterMovement());
}
