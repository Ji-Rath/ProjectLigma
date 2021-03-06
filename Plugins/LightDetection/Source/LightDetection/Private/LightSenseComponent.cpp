#include "LightSenseComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ShapeComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/LocalLightComponent.h"
#include "EngineUtils.h"

float ULightSenseComponent::CalculateLightLevel(const FVector& SurfacePos) const
{
	float LightVisibility = 0.f;

	// Loop through ALL light components in the level
	// TODO: Make this scalable
	const UWorld* MyWorld = GetWorld();
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;

		for (TSubclassOf<AActor> ActorClass : LightActors)
		{
			// Make sure the actor we are checking for is one we want
			if (ActorClass && !Actor->IsA(ActorClass)) { continue; }

			// Get all light components from the actor to calculate
			TArray<ULightComponent*> Lights;
			Actor->GetComponents(Lights);

			for (ULightComponent* Light : Lights)
			{
				LightVisibility += GetSingleLightLevel(Light, SurfacePos);
			}
		}
	}

	return LightVisibility;
}

float ULightSenseComponent::GetSingleLightLevel(ULightComponent* Light, const FVector& SurfacePos) const
{
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
						return GetSpotLightLevel(SpotLight, SurfacePos);
					}
					else if (auto* PointLight = Cast<UPointLightComponent>(Light))
					{
						return GetPointLightLevel(PointLight, SurfacePos);
					}
				}
			}
			else if (auto* DirectionalLight = Cast<UDirectionalLightComponent>(Light))
			{
				return GetDirectionalLightLevel(DirectionalLight, SurfacePos);
			}
		}
	}
	return 0.f;
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

	LightLevel = CalculateLightLevel(GetOwner()->GetActorLocation());
}

void ULightSenseComponent::BeginPlay()
{
	Super::BeginPlay();
}

float ULightSenseComponent::GetSpotLightLevel(USpotLightComponent* Light, const FVector& SurfacePos) const 
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
				LightVisibility = GetPointLightLevel(Light, SurfacePos);
		}
	}
	return LightVisibility;
}

float ULightSenseComponent::GetPointLightLevel(UPointLightComponent* Light, const FVector& SurfacePos) const
{
	float LightVisibility = 0.f;
	
	// Reduce visible range based on intensity of light
	float UnitFactor = Light->GetUnitsConversionFactor(Light->IntensityUnits, ELightUnits::Candelas);
	float IntensityFactor = (Light->Intensity * UnitFactor) / 20.f;

	float VisibleRange = Light->AttenuationRadius * IntensityFactor;
	float ClearVisibleRange = VisibleRange / 3;

	if (ensure(Light))
	{
		FHitResult Hit;
		FVector LightDirection = UKismetMathLibrary::GetForwardVector(Light->GetComponentRotation());
		bool bTraced = GetWorld()->LineTraceSingleByChannel(Hit, SurfacePos, Light->GetComponentLocation(), ECC_Visibility);
		
		// Determine light level based on distance to light
		if (!bTraced)
		{
			float Distance = FVector::Distance(SurfacePos, Light->GetComponentLocation());
			LightVisibility = 1-UKismetMathLibrary::NormalizeToRange(Distance, ClearVisibleRange, VisibleRange);
			//DrawDebugLine(GetWorld(), SurfacePos, Light->GetComponentLocation(), FColor::Red, false, 1.f, 0, 1.f);
		}
		else
		{
			//DrawDebugLine(GetWorld(), SurfacePos, Light->GetComponentLocation(), FColor::Green, false, 1.f, 0, 1.f);
		}
	}
	return FMath::Clamp(LightVisibility, 0.f, 1.f);
}

float ULightSenseComponent::GetDirectionalLightLevel(UDirectionalLightComponent* Light, const FVector& SurfacePos) const
{
	bool bTraced = false;
	if (ensure(Light))
	{
		FHitResult Hit;
		FVector LightDirection = UKismetMathLibrary::GetForwardVector(Light->GetComponentRotation()) * -1;
		FVector EndPos = SurfacePos + (LightDirection * LightTraceDistance);

		bTraced = GetWorld()->LineTraceSingleByChannel(Hit, SurfacePos, EndPos, ECC_Visibility);
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