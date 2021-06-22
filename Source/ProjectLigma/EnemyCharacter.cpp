#include "EnemyCharacter.h"



AEnemyCharacter::AEnemyCharacter()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
