
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
		if (!GetWorldTimerManager().TimerExists(PursueSeenPlayerHandle))
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("SetEnemyState"), EEnemyState::Pursuing);
			GetWorldTimerManager().SetTimer(PursueSeenPlayerHandle, TimerDelegate, DetectPlayerDelay, false);
		}
	}
	else if (GetWorldTimerManager().TimerExists(PursueSeenPlayerHandle))
	{
		GetWorldTimerManager().ClearTimer(PursueSeenPlayerHandle);
	}
}

void AEnemyController::PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const FAISenseID SenseID = Stimulus.Type;

	if (SenseID == SightID)
	{
		Blackboard->SetValueAsEnum(BBEnemyState, (uint8)EEnemyState::Searching);
		Blackboard->SetValueAsVector(BBSearchVector, Stimulus.StimulusLocation);
		bSeePlayer = Stimulus.WasSuccessfullySensed();
		Blackboard->SetValueAsObject(BBPlayerTarget, Actor);

		/*
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindStatic(&AEnemyController::SetEnemyState, EEnemyState::Idle);
		GetWorldTimerManager().SetTimer(SearchLocationHandle, TimerDelegate, 15.f, false);
		*/
		
	}
}

void AEnemyController::SetEnemyState(EEnemyState EnemyState)
{
	Blackboard->SetValueAsEnum(BBEnemyState, (uint8) EnemyState);
}

void AEnemyController::ExpiredStimulus(const FAIStimulus& StimulusStore)
{
	if (StimulusStore.Type == SightID)
	{
		SetEnemyState(EEnemyState::Idle);
	}
}

AEnemyController::AEnemyController()
{
	SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());;
	HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());;
}
