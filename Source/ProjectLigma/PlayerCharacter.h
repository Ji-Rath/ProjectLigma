#pragma once

#include "GameFramework/Character.h"
#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class PROJECTLIGMA_API APlayerCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

private:
	APlayerCharacter();

	FGenericTeamId TeamID;
	
public:
	void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	FGenericTeamId GetGenericTeamId() const override;

	void Tick(float DeltaSeconds) override;

	void BeginPlay() override;

};

