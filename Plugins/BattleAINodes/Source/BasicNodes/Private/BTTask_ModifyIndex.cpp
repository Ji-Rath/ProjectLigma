#include "BTTask_ModifyIndex.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ModifyIndex::UBTTask_ModifyIndex()
{
	Index.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ModifyIndex, Index));
	IndexLength.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ModifyIndex, IndexLength));
	IncrementDirection.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ModifyIndex, IncrementDirection));
}

EBTNodeResult::Type UBTTask_ModifyIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	// Fetch blackboard values
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	int32 IndexValue = Blackboard->GetValueAsInt(Index.SelectedKeyName);
	int32 IndexLengthValue = Blackboard->GetValueAsInt(IndexLength.SelectedKeyName);
	int32 IncrementDirectionValue = Blackboard->GetValueAsInt(IncrementDirection.SelectedKeyName);

	// Perform modifications to index
	switch (ModifyIndex)
	{
		case Reset:
			IndexValue += 1;
			if (IndexValue > IndexLengthValue)
				IndexValue = 0;
			break;
		case Flipflop:
			IndexValue += 1 * IncrementDirectionValue;
			if (IndexValue > IndexLengthValue || IndexValue <= 0)
				IncrementDirectionValue *= -1;
			break;
		case Random:
			IndexValue = FMath::RandRange(0, IndexLengthValue - 1);
			break;
	}

	// Update blackboard values
	Blackboard->SetValueAsInt(Index.SelectedKeyName, IndexValue);
	Blackboard->SetValueAsInt(IncrementDirection.SelectedKeyName, IncrementDirectionValue);

	return EBTNodeResult::Succeeded;
}

void UBTTask_ModifyIndex::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		Index.ResolveSelectedKey(*BBAsset);
		IndexLength.ResolveSelectedKey(*BBAsset);

	}
	else
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("Can't initialize task: %s, make sure that behavior tree specifies blackboard asset!"), *GetName());
	}
}
