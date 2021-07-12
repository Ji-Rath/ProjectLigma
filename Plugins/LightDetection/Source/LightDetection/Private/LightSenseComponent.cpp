#include "LightSenseComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ShapeComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/LocalLightComponent.h"
#include "EngineUtils.h"

float ULightSenseComponent::CalculateLightLevel(UObject* WorldContextObject, const FVector& SurfacePos, const TArray<TSubclassOf<AActor>> ActorLights, TArray<FHitResult>& OutHits, const float TraceDistance /*= 2000.f*/)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	float LightVisibility = 0.f;

	// Loop through ALL light components in the level
	// TODO: Make this scalable
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;

		for (TSubclassOf<AActor> ActorClass : ActorLights)
		{
			// Make sure the actor we are checking for is one we want
			if (ActorClass && !Actor->IsA(ActorClass)) { continue; }

			// Get all light components from the actor to calculate
			TArray<ULightComponent*> Lights;
			Actor->GetComponents(Lights);

			FHitResult Hit;
			for (ULightComponent* Light : Lights)
			{
				LightVisibility += GetSingleLightLevel(WorldContextObject, Light, SurfacePos, {}, Hit, TraceDistance);
				OutHits.Add(Hit);
			}
		}
	}

	return LightVisibility;
}

float ULightSenseComponent::GetSingleLightLevel(UObject* WorldContextObject, const ULightComponent* Light, const FVector& SurfacePos, TArray<AActor*> ActorsToIgnore, FHitResult& OutHit, const float TraceDistance /*= 2000.f*/, const ECollisionChannel Channel /*= ECC_Visibility*/)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	float LightStrength = 0.f;
	// Ensure Light component is valid
	if (Light)
	{
		// Make sure light component is emitting light
		bool bIsOn = !FMath::IsNearlyEqual(Light->Intensity, 0.f);
		if (bIsOn)
		{
			// Attempt to loop through only relevant lights based on type and distance
			if (auto* LocalLight = Cast<ULocalLightComponent>(Light))
			{
				bool bWithinDistance = FVector::Distance(SurfacePos, Light->GetComponentLocation()) < LocalLight->AttenuationRadius;
				if (bWithinDistance)
				{
					if (auto* SpotLight = Cast<USpotLightComponent>(Light))
					{
						LightStrength = GetSpotLightLevel(World, SpotLight, SurfacePos, OutHit, Channel);
					}
					else if (auto* PointLight = Cast<UPointLightComponent>(Light))
					{
						LightStrength = GetPointLightLevel(World, PointLight, SurfacePos, ActorsToIgnore, OutHit, Channel);
					}
				}
			}
			else if (auto* DirectionalLight = Cast<UDirectionalLightComponent>(Light))
			{
				LightStrength = GetDirectionalLightLevel(World, DirectionalLight, SurfacePos, OutHit, TraceDistance);
			}
		}
	}
	return LightStrength;
}

float ULightSenseComponent::GetCurrentLightLevel() const
{
	return LightLevel;
}

ULightSenseComponent::ULightSenseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	LightActors.Add(AActor::StaticClass());
}

void ULightSenseComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<FHitResult> Hit;
	LightLevel = CalculateLightLevel(GetWorld(), GetOwner()->GetActorLocation(), LightActors, Hit, LightTraceDistance);
}

void ULightSenseComponent::BeginPlay()
{
	Super::BeginPlay();
}

float ULightSenseComponent::GetSpotLightLevel(const UWorld* World, const USpotLightComponent* Light, const FVector& SurfacePos, FHitResult& OutHit, const ECollisionChannel Channel /*= ECC_Visibility*/) 
{
	float LightVisibility = 0.f;
	if (ensure(Light))
	{
		float LightRange = Light->AttenuationRadius;
		float LightAngle = Light->OuterConeAngle;
	
		float Distance = FVector::Distance(Light->GetComponentLocation(), SurfacePos);

		// Only determine visibility if within range
		if (Distance < LightRange)
		{
			// Determine angle from light to surface
			FVector LightDirection = Light->GetOwner()->GetActorForwardVector();
			FVector Point = SurfacePos - Light->GetComponentLocation();
			Point.Normalize();

			float Dot = FVector::DotProduct(Point, LightDirection);
			float AngleToSurface = UKismetMathLibrary::DegAcos(Dot);

			// Determine whether the surface is in light based on line trace
			if (AngleToSurface < LightAngle)
				LightVisibility = GetPointLightLevel(World, Light, SurfacePos, {}, OutHit, Channel);
		}
	}
	return LightVisibility;
}

float ULightSenseComponent::GetPointLightLevel(const UWorld* World, const UPointLightComponent* Light, const FVector& SurfacePos, TArray<AActor*> ActorsToIgnore, FHitResult& OutHit, const ECollisionChannel Channel /*= ECC_Visibility*/)
{
	float LightVisibility = 0.f;
	
	// Reduce visible range based on intensity of light
	float UnitFactor = Light->GetUnitsConversionFactor(Light->IntensityUnits, ELightUnits::Candelas);
	float IntensityFactor = (Light->Intensity * UnitFactor) / 20.f;

	float VisibleRange = Light->AttenuationRadius * IntensityFactor;
	float ClearVisibleRange = VisibleRange / 3;

	if (ensure(Light))
	{
		FVector LightDirection = UKismetMathLibrary::GetForwardVector(Light->GetComponentRotation());
		FCollisionQueryParams CollisionParams = FCollisionQueryParams::DefaultQueryParam;
		CollisionParams.AddIgnoredActors(ActorsToIgnore);
		bool bTraced = World->LineTraceSingleByChannel(OutHit, Light->GetComponentLocation(), SurfacePos, Channel, CollisionParams);
		
		// Determine light level based on distance to light
		if (!bTraced)
		{
			float Distance = FVector::Distance(SurfacePos, Light->GetComponentLocation());
			LightVisibility = 1-UKismetMathLibrary::NormalizeToRange(Distance, ClearVisibleRange, VisibleRange);
			//DrawDebugLine(World, SurfacePos, Light->GetComponentLocation(), FColor::Red, false, 1.f, 0, 1.f);
		}
		else
		{
			//DrawDebugLine(World, SurfacePos, Light->GetComponentLocation(), FColor::Green, false, 1.f, 0, 1.f);
			//DrawDebugPoint(World, OutHit.ImpactPoint, 5.f, FColor::Red, false, 1.f);
		}
	}
	return FMath::Clamp(LightVisibility, 0.f, 1.f);
}

float ULightSenseComponent::GetDirectionalLightLevel(const UWorld* World, const UDirectionalLightComponent* Light, const FVector& SurfacePos, FHitResult& OutHit, const float TraceDistance /*= 2000.f*/)
{
	bool bTraced = false;
	if (ensure(Light))
	{
		FVector LightDirection = UKismetMathLibrary::GetForwardVector(Light->GetComponentRotation()) * -1;
		FVector EndPos = SurfacePos + (LightDirection * TraceDistance);

		bTraced = World->LineTraceSingleByChannel(OutHit, SurfacePos, EndPos, ECC_Visibility);
		if (bTraced)
		{
			//DrawDebugLine(GetWorld(), SurfacePos, EndPos, FColor::Green, false, 1.f, 0, 1.f);
		}
		else
		{
			//DrawDebugLine(GetWorld(), SurfacePos, EndPos, FColor::Red, false, 1.f, 0, 1.f);
		}
	}
	return bTraced ? 0.f : 1.f;
}