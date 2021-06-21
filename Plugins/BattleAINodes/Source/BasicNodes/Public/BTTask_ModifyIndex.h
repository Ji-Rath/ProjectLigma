#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ModifyIndex.generated.h"

UENUM()
enum ModifyType
{
	Reset,
	Flipflop,
	Random
};

UCLASS()
class BASICNODES_API UBTTask_ModifyIndex : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** How to modify the index. Reset - reset to 0 after reaching the last point. Flipflop - reverse direction after reaching last point. Random - go to a random index */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ModifyType> ModifyIndex = Reset;

	/** The greatest index possible */
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector IndexLength;

	/** The current index */
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector Index;

	/** Optional value, only used when calculating flipflop */
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector IncrementDirection;

	UBTTask_ModifyIndex();

private:
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void InitializeFromAsset(UBehaviorTree& Asset) override;

	
};
