#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAIPerceptionComponent;

UCLASS()
class PROJECTLIGMA_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UseComponentPicker, AllowedClasses = "SplineComponent", AllowAnyActor))
	FComponentReference PatrolSpline;

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* PerceptionComponent;

	AEnemyCharacter();

protected:
	void BeginPlay() override;

	void Tick(float DeltaSeconds) override;
};