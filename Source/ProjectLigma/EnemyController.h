#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GenericTeamAgentInterface.h"
#include "EnemyController.generated.h"

struct FTimerHandle;
class ULightSenseComponent;

UENUM(BlueprintType)
enum EEnemyState
{
	Idle,
	Searching,
	Pursuing
};

UCLASS()
class PROJECTLIGMA_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName BBSearchVector;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName BBDestinationVector;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName BBEnemyState;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName BBPlayerTarget;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AlertnessDetectPlayer = 75.f;

	/** How alert the enemy is 0-100 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	float Alertness = 0.f;

	/** Alertness Multiplier for when the player is 'seen', calculated every second. Multiplied against player light level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AlertMultiplier = 5.f;

	/** Minimum light level to begin sensing the player */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float MinLightLevel = 0.25f;

	UPROPERTY()
	bool bSeePlayer = false;

	/** Range to automatically detect player target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float NearsightRange = 150.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FTimerHandle LoseInterestHandle;

	/** Value to lose alertness every second after not seeing player (after short delay) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float DisinterestValue = -5.f;

	UFUNCTION(BlueprintCallable)
	void IncrementAlertness(float Amount);

	UFUNCTION(BlueprintCallable)
	bool IsTargetVisible(FVector Target);

	UPROPERTY(EditAnywhere, Category = "Teams")
	uint8 TeamNumber = FGenericTeamId::NoTeam;

	/** Returns whether the enemy is within the 'pursue' player threshold */
	UFUNCTION(BlueprintCallable)
	bool IsAlert();

protected:

	void BeginPlay() override;

	void Tick(float DeltaTime) override;

	UFUNCTION()
	void PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/**
	 * Set the state of the enemy
	 * @param EnemyState Idle, Pursuing, Searching
	 * @return void
	*/
	UFUNCTION()
	void SetEnemyState(EEnemyState EnemyState);

	UFUNCTION()
	void ExpiredStimulus(const FAIStimulus& StimulusStore);

private:

	AEnemyController();

	FAISenseID SightID;
	FAISenseID HearingID;
};

