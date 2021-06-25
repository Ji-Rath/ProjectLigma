
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "SoldierPerceptionComponent.h"
#include "LightSenseComponent.h"
#include "Kismet/KismetMathLibrary.h"


void AEnemyController::IncrementAlertness(float Amount)
{
	Alertness = FMath::Clamp(Alertness + Amount, 0.f, 100.f);

	if (Alertness <= 0.f)
	{
		SetEnemyState(EEnemyState::Idle);
	}

	if (Alertness > 0.f)
	{
		//Pursue player if enemy is within alert range
		if (Alertness >= AlertnessDetectPlayer && Amount > 0.f)
		{
			SetEnemyState(EEnemyState::Pursuing);
		}
		else
		{
			SetEnemyState(EEnemyState::Searching);
		}
	}
	
}

bool AEnemyController::IsTargetVisible(FVector Target)
{
	float LightLevel = LightSense->CalculateLightLevel(Target);
	float Distance = FVector::Distance(GetPawn()->GetActorLocation(), Target);
	bool bVisible = LightLevel > MinLightLevel || Distance < NearsightRange;

	return bVisible;
}

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
		AActor* Player = Cast<AActor>(Blackboard->GetValueAsObject(BBPlayerTarget));
		if (IsTargetVisible(Player->GetActorLocation()))
		{
			float Distance = FVector::Distance(Player->GetActorLocation(), GetPawn()->GetActorLocation());
			float DistanceMultiplier = (1 - UKismetMathLibrary::NormalizeToRange(Distance, 0.f, 2000.f)) * 5.f;
			float LightLevel = LightSense->CalculateLightLevel(Player->GetActorLocation());
			float AlertIncrement = (LightLevel * AlertMultiplier * DistanceMultiplier) * DeltaTime;

			IncrementAlertness(AlertIncrement);

			if (GetWorldTimerManager().TimerExists(LoseInterestHandle))
			{
				// Timer is canceled if the player is no longer seen
				GetWorldTimerManager().ClearTimer(LoseInterestHandle);
			}
		}
		else
		{
			// Have a small delay before alertness begins to subtract
			if (!GetWorldTimerManager().TimerExists(LoseInterestHandle) && Alertness >= 0.f && Alertness <= AlertnessDetectPlayer)
			{
				FTimerDelegate TimerDelegate;
				TimerDelegate.BindUFunction(this, FName("IncrementAlertness"), DisinterestValue);
				GetWorldTimerManager().SetTimer(LoseInterestHandle, TimerDelegate, 1.f, true, 10.f);
			}
		}
	}
	else 
	{
		// Have a small delay before alertness begins to subtract
		if (!GetWorldTimerManager().TimerExists(LoseInterestHandle))
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("IncrementAlertness"), DisinterestValue);
			GetWorldTimerManager().SetTimer(LoseInterestHandle, TimerDelegate, 1.f, true, 10.f);
			IncrementAlertness(-5.f);
		}
	}
}

void AEnemyController::PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const FAISenseID SenseID = Stimulus.Type;

	// Sight stimulus
	if (SenseID == SightID)
	{
		// Only update stimulus position if player is truly 'visible'
		if (IsTargetVisible(Actor->GetActorLocation()))
		{
			// Input data to blackboard to be computed in BT
			Blackboard->SetValueAsVector(BBDestinationVector, Stimulus.StimulusLocation);
			Blackboard->SetValueAsVector(BBSearchVector, Stimulus.StimulusLocation);
		}
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

	PrimaryActorTick.TickInterval = 0.25f;

	LightSense = CreateDefaultSubobject<ULightSenseComponent>(TEXT("Light Sense"));

	SetGenericTeamId(FGenericTeamId(1));
}
