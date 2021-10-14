#include "PlayerPushComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Interaction/ToggleInteractable.h"


UPlayerPushComponent::UPlayerPushComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
		// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UPlayerPushComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterMoveComp = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	ensureMsgf(CharacterMoveComp, TEXT("No Character Movement Component found for %s"), *(GetOwner()->GetName()));

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	ensureMsgf(CharacterMoveComp, TEXT("No Physics Handle Component found for %s"), *(GetOwner()->GetName()));
}

void UPlayerPushComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bPushingObject && PhysicsHandle->GetGrabbedComponent())
	{
		FVector TargetLocation = TargetOffset + GetOwner()->GetActorLocation();
		PhysicsHandle->SetTargetLocation(TargetLocation);

		if (FVector::Distance(TargetLocation, GetOwner()->GetActorLocation()) > (TargetOffset.Size() + 25.f))
		{
			AToggleInteractable* PushBox = Cast<AToggleInteractable>(PhysicsHandle->GetGrabbedComponent()->GetOwner());
			PushBox->SetState(GetOwner(), false);
		}
	}
}

void UPlayerPushComponent::PushObject(UPrimitiveComponent* PushComponent)
{
	if (!CharacterMoveComp || !PhysicsHandle) { return; }
	
	bPushingObject = true;

	CharacterMoveComp->MaxWalkSpeed = 200.f;
	CharacterMoveComp->bOrientRotationToMovement = false;

	FHitResult OutHit;
	GetWorld()->LineTraceSingleByChannel(OUT OutHit, GetOwner()->GetActorLocation(), PushComponent->GetCenterOfMass(), ECC_Visibility);

	TargetRotation = UKismetMathLibrary::MakeRotFromX(OutHit.ImpactNormal * -1.f);
	TargetOffset = OutHit.ImpactPoint - GetOwner()->GetActorLocation();

	PhysicsHandle->GrabComponentAtLocation(PushComponent, FName(), OutHit.ImpactPoint);

	OnPushObject.Broadcast(true);
}

void UPlayerPushComponent::StopPushing()
{
	if (!CharacterMoveComp || !PhysicsHandle) { return; }

	bPushingObject = false;

	CharacterMoveComp->MaxWalkSpeed = 400.f;
	CharacterMoveComp->bOrientRotationToMovement = true;

	PhysicsHandle->ReleaseComponent();

	OnPushObject.Broadcast(false);
}

bool UPlayerPushComponent::IsPushingObject()
{
	return bPushingObject;
}
