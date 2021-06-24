
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "SoldierPerceptionComponent.h"


void AEnemyController::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::PerceptionUpdated);

	if (auto* SoldierPerception = Cast<USoldierPerceptionComponent>(PerceptionComponent))
	{
		SoldierPerception->OnExpiredStimulus.AddDynamic(this, &AEnemyController::ExpiredStimulus);
	}
}

void AEnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bSeePlayer)
	{
		// Have a small delay before player is fully 'detected'
		if (!GetWorldTimerManager().TimerExists(PursueSeenPlayerHandle))
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("SetEnemyState"), EEnemyState::Pursuing);
			GetWorldTimerManager().SetTimer(PursueSeenPlayerHandle, TimerDelegate, DetectPlayerDelay, false);
		}
	}
	else if (GetWorldTimerManager().TimerExists(PursueSeenPlayerHandle))
	{
		// Timer is canceled if the player is no longer seen
		GetWorldTimerManager().ClearTimer(PursueSeenPlayerHandle);
	}
}

void AEnemyController::PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const FAISenseID SenseID = Stimulus.Type;

	// Sight stimulus
	if (SenseID == SightID)
	{
		// Input data to blackboard to be computed in BT
		Blackboard->SetValueAsEnum(BBEnemyState, (uint8)EEnemyState::Searching);
		Blackboard->SetValueAsVector(BBDestinationVector, Stimulus.StimulusLocation);
		Blackboard->SetValueAsVector(BBSearchVector, Stimulus.StimulusLocation);
		bSeePlayer = Stimulus.WasSuccessfullySensed();
		Blackboard->SetValueAsObject(BBPlayerTarget, Actor);
	}
}

void AEnemyController::SetEnemyState(EEnemyState EnemyState)
{
	Blackboard->SetValueAsEnum(BBEnemyState, (uint8) EnemyState);
}

void AEnemyController::ExpiredStimulus(const FAIStimulus& StimulusStore)
{
	// Lose interest if sight stimulus expires
	if (StimulusStore.Type == SightID)
	{
		SetEnemyState(EEnemyState::Idle);
	}
}

AEnemyController::AEnemyController()
{
	SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());;
	HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());;

	PrimaryActorTick.TickInterval = 0.5f;
}
