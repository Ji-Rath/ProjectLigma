#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerPushComponent.generated.h"

class UCharacterMovementComponent;
class UPhysicsHandleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPushObject, bool, bPushingObject);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEM_API UPlayerPushComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	
	UPlayerPushComponent();

	void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector TargetOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bPushingObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FRotator TargetRotation;

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FPushObject OnPushObject;

	UFUNCTION(BlueprintCallable)
	void PushObject(UPrimitiveComponent* PushComponent);

	UFUNCTION(BlueprintCallable)
	void StopPushing();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPushingObject();

	UCharacterMovementComponent* CharacterMoveComp;

	UPhysicsHandleComponent* PhysicsHandle;
};

