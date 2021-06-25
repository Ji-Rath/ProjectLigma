
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
#include "DrawDebugHelpers.h"


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
	bool bVisible = false;

	if (bSeePlayer)
	{
		float LightLevel = LightSense->CalculateLightLevel(Target);
		float Distance = FVector::Distance(GetPawn()->GetActorLocation(), Target);
		float AlertRange = UKismetMathLibrary::NormalizeToRange(Alertness, 0.f, 100.f) * (1000.f - NearsightRange);
		bVisible = (LightLevel > MinLightLevel || Distance < (AlertRange + NearsightRange));
		
	}
	
	return bVisible;
}

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();

	SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());;
	HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());;

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyController::PerceptionUpdated);

	if (auto* SoldierPerception = Cast<USoldierPerceptionComponent>(PerceptionComponent))
	{
		SoldierPerception->OnExpiredStimulus.AddDynamic(this, &AEnemyController::ExpiredStimulus);
	}
}

void AEnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float AlertRange = UKismetMathLibrary::NormalizeToRange(Alertness, 0.f, 100.f) * (1000.f-NearsightRange);
	DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation(), AlertRange + NearsightRange, 10, FColor::Red, false, 0.5f);

	AActor* Player = Cast<AActor>(Blackboard->GetValueAsObject(BBPlayerTarget));
	if (Player)
	{
		if (IsTargetVisible(Player->GetActorLocation()))
		{
			// Calculate alertness based on target distance and light level
			float Distance = FVector::Distance(Player->GetActorLocation(), GetPawn()->GetActorLocation());
			float DistanceMultiplier = (1 - UKismetMathLibrary::NormalizeToRange(Distance, 0.f, 2000.f)) * 5.f;
			float LightLevel = LightSense->CalculateLightLevel(Player->GetActorLocation());
			float AlertIncrement = (AlertMultiplier * DistanceMultiplier) * DeltaTime;

			IncrementAlertness(AlertIncrement);

			if (GetWorldTimerManager().TimerExists(LoseInterestHandle))
			{
				// Lose Interest timer is canceled if the player is visible
				GetWorldTimerManager().ClearTimer(LoseInterestHandle);
			}
		}
		else if (!GetWorldTimerManager().TimerExists(LoseInterestHandle))
		{
			// Have a small delay before alertness begins to subtract
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("IncrementAlertness"), DisinterestValue);
			GetWorldTimerManager().SetTimer(LoseInterestHandle, TimerDelegate, 1.f, true, 10.f);
			IncrementAlertness(-5.f);

			// Input data to blackboard to be computed in BT
			Blackboard->SetValueAsVector(BBDestinationVector, Player->GetActorLocation());
			Blackboard->SetValueAsVector(BBSearchVector, Player->GetActorLocation());
		}
	}
}

void AEnemyController::PerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const FAISenseID SenseID = Stimulus.Type;

	// Sight stimulus
	if (SenseID == SightID)
	{
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
		if (!GetWorldTimerManager().TimerExists(LoseInterestHandle))
		{
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("IncrementAlertness"), DisinterestValue);
			GetWorldTimerManager().SetTimer(LoseInterestHandle, TimerDelegate, 1.f, true);
		}
	}
}

AEnemyController::AEnemyController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f;

	LightSense = CreateDefaultSubobject<ULightSenseComponent>(TEXT("Light Sense"));

	SetGenericTeamId(FGenericTeamId(TeamNumber));
}
