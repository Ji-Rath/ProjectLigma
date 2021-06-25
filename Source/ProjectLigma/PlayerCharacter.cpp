#include "PlayerCharacter.h"


APlayerCharacter::APlayerCharacter()
{
	SetGenericTeamId(FGenericTeamId(2));
}

void APlayerCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (TeamID != NewTeamID)
	{
		TeamID = NewTeamID;
	}
}

FGenericTeamId APlayerCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

}
