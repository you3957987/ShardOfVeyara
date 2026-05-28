// AGSDInventoryComponent.cpp - 인벤토리 컴포넌트 구현
// 기존 블루프린트 BPAC_InventoryComponent의 모든 로직을 C++로 재구현합니다.

#include "AGSDInventoryComponent.h"
#include "SOVGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UAGSDInventoryComponent::UAGSDInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ═══════════════════════════════════════════════════
// 라이프사이클
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSlots();
}

void UAGSDInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveToGameInstance();
	Super::EndPlay(EndPlayReason);
}

// ═══════════════════════════════════════════════════
// 초기화
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::InitializeSlots()
{
	// GameInstance에서 데이터 복원 시도
	LoadFromGameInstance();

	// 복원된 데이터가 없거나 크기가 맞지 않으면 새로 초기화
	if (InventorySlots.Num() != MaxSlots)
	{
		InventorySlots.Empty();
		InventorySlots.SetNum(MaxSlots);

		for (int32 i = 0; i < MaxSlots; ++i)
		{
			InventorySlots[i] = FStruct_InventorySlotData();
			InventorySlots[i].SlotIndex = i;
			InventorySlots[i].IsEmpty = true;
		}
	}

	// 핫바 선택 인덱스도 GameInstance에서 복원
	if (USOVGameInstance* GI = GetGameInstance())
	{
		CurrentSelectedHotbar = FMath::Clamp(GI->CurrentSelectedHotbar, HOTBAR_START_INDEX, HOTBAR_END_INDEX);
	}

	OnInventoryFullyUpdated.Broadcast();
}

// ═══════════════════════════════════════════════════
// 아이템 추가
// ═══════════════════════════════════════════════════

bool UAGSDInventoryComponent::AddItem(FStruct_ItemData ItemData, int32& OutRemainingQty)
{
	OutRemainingQty = ItemData.CurrentQuantity;

	if (OutRemainingQty <= 0)
	{
		return false;
	}

	bool bAddedAny = false;

	// 1단계: 기존 스택에 중첩 시도
	for (int32 i = 0; i < MaxSlots && OutRemainingQty > 0; ++i)
	{
		if (!InventorySlots[i].IsEmpty &&
			InventorySlots[i].ItemData.ItemID == ItemData.ItemID &&
			InventorySlots[i].ItemData.CurrentQuantity < InventorySlots[i].ItemData.MaxQuantity)
		{
			const int32 SpaceAvailable = InventorySlots[i].ItemData.MaxQuantity - InventorySlots[i].ItemData.CurrentQuantity;
			const int32 AmountToAdd = FMath::Min(SpaceAvailable, OutRemainingQty);

			InventorySlots[i].ItemData.CurrentQuantity += AmountToAdd;
			OutRemainingQty -= AmountToAdd;
			bAddedAny = true;

			OnInventorySlotUpdated.Broadcast(i);
			OnItemAdded.Broadcast(i, InventorySlots[i].ItemData);
		}
	}

	// 2단계: 빈 슬롯에 배치
	while (OutRemainingQty > 0)
	{
		const int32 EmptySlot = FindEmptySlot();
		if (EmptySlot == -1)
		{
			// 인벤토리 가득 참
			break;
		}

		const int32 AmountToPlace = FMath::Min(ItemData.MaxQuantity, OutRemainingQty);

		InventorySlots[EmptySlot].IsEmpty = false;
		InventorySlots[EmptySlot].ItemData = ItemData;
		InventorySlots[EmptySlot].ItemData.CurrentQuantity = AmountToPlace;
		OutRemainingQty -= AmountToPlace;
		bAddedAny = true;

		OnInventorySlotUpdated.Broadcast(EmptySlot);
		OnItemAdded.Broadcast(EmptySlot, InventorySlots[EmptySlot].ItemData);
	}

	return bAddedAny;
}

// ═══════════════════════════════════════════════════
// 아이템 제거
// ═══════════════════════════════════════════════════

bool UAGSDInventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
	if (!ValidateSlotIndex(SlotIndex, TEXT("RemoveItem"))) return false;
	if (InventorySlots[SlotIndex].IsEmpty) return false;

	const FStruct_ItemData RemovedItemData = InventorySlots[SlotIndex].ItemData;

	if (Quantity <= 0 || Quantity >= InventorySlots[SlotIndex].ItemData.CurrentQuantity)
	{
		// 전체 제거
		ClearSlot(SlotIndex);
	}
	else
	{
		// 일부 제거
		InventorySlots[SlotIndex].ItemData.CurrentQuantity -= Quantity;
		OnInventorySlotUpdated.Broadcast(SlotIndex);
	}

	OnItemRemoved.Broadcast(SlotIndex, RemovedItemData);
	return true;
}

// ═══════════════════════════════════════════════════
// 슬롯 교환
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::SwapSlots(int32 FromIndex, int32 ToIndex)
{
	if (!ValidateSlotIndex(FromIndex, TEXT("SwapSlots (From)"))) return;
	if (!ValidateSlotIndex(ToIndex, TEXT("SwapSlots (To)"))) return;
	if (FromIndex == ToIndex) return;

	// 같은 아이템이면 합치기 시도
	if (!InventorySlots[FromIndex].IsEmpty && !InventorySlots[ToIndex].IsEmpty)
	{
		if (InventorySlots[FromIndex].ItemData.ItemID == InventorySlots[ToIndex].ItemData.ItemID)
		{
			const int32 SpaceAvailable = InventorySlots[ToIndex].ItemData.MaxQuantity - InventorySlots[ToIndex].ItemData.CurrentQuantity;
			
			if (SpaceAvailable > 0)
			{
				const int32 AmountToMerge = FMath::Min(SpaceAvailable, InventorySlots[FromIndex].ItemData.CurrentQuantity);
				InventorySlots[ToIndex].ItemData.CurrentQuantity += AmountToMerge;
				InventorySlots[FromIndex].ItemData.CurrentQuantity -= AmountToMerge;

				if (InventorySlots[FromIndex].ItemData.CurrentQuantity <= 0)
				{
					InventorySlots[FromIndex].IsEmpty = true;
					InventorySlots[FromIndex].ItemData = FStruct_ItemData();
				}

				OnInventorySlotUpdated.Broadcast(FromIndex);
				OnInventorySlotUpdated.Broadcast(ToIndex);
				return;
			}
		}
	}

	// 다른 아이템이거나 합치기 불가 → 위치 교환
	FStruct_InventorySlotData TempSlot = InventorySlots[FromIndex];
	
	InventorySlots[FromIndex] = InventorySlots[ToIndex];
	InventorySlots[FromIndex].SlotIndex = FromIndex;
	
	InventorySlots[ToIndex] = TempSlot;
	InventorySlots[ToIndex].SlotIndex = ToIndex;

	OnInventorySlotUpdated.Broadcast(FromIndex);
	OnInventorySlotUpdated.Broadcast(ToIndex);
}

// ═══════════════════════════════════════════════════
// 슬롯 비우기
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::ClearSlot(int32 SlotIndex)
{
	if (!ValidateSlotIndex(SlotIndex, TEXT("ClearSlot"))) return;

	InventorySlots[SlotIndex].IsEmpty = true;
	InventorySlots[SlotIndex].ItemData = FStruct_ItemData();
	InventorySlots[SlotIndex].SlotIndex = SlotIndex;

	OnInventorySlotUpdated.Broadcast(SlotIndex);
}

// ═══════════════════════════════════════════════════
// 빈 슬롯 탐색
// ═══════════════════════════════════════════════════

int32 UAGSDInventoryComponent::FindEmptySlot() const
{
	// 0번부터 MaxSlots-1까지 순차적으로 빈 슬롯을 탐색합니다.
	// (인덱스 구조상 핫바(0~9) 영역이 가방(10~) 영역보다 우선 탐색됩니다.)
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (InventorySlots[i].IsEmpty)
		{
			return i;
		}
	}

	return -1; // 인벤토리 가득 참
}

// ═══════════════════════════════════════════════════
// 중첩 가능 슬롯 탐색
// ═══════════════════════════════════════════════════

int32 UAGSDInventoryComponent::FindStackableSlot(const FString& ItemID) const
{
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (!InventorySlots[i].IsEmpty &&
			InventorySlots[i].ItemData.ItemID == ItemID &&
			InventorySlots[i].ItemData.CurrentQuantity < InventorySlots[i].ItemData.MaxQuantity)
		{
			return i;
		}
	}

	return -1;
}

// ═══════════════════════════════════════════════════
// 아이템 드롭
// ═══════════════════════════════════════════════════

bool UAGSDInventoryComponent::DropItem(int32 SlotIndex)
{
	if (!ValidateSlotIndex(SlotIndex, TEXT("DropItem"))) return false;
	if (InventorySlots[SlotIndex].IsEmpty) return false;

	const FStruct_ItemData& ItemData = InventorySlots[SlotIndex].ItemData;

	// ItemBPClass가 유효한 경우에만 월드에 스폰
	if (ItemData.ItemBPClass)
	{
		UWorld* World = GetWorld();
		AActor* Owner = GetOwner();
		if (World && Owner)
		{
			// 캐릭터 전방 1.5m 위치를 기준으로 설정
			const FVector StartLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 150.0f;
			// 바닥 높이 검출을 위해 아래로 20m (2000 유닛) 레이캐스트
			const FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 2000.0f);

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Owner); // 드롭한 액터(플레이어) 무시

			FVector DropLocation = StartLocation;

			// Visibility 채널을 통해 지형 바닥 탐색
			if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
			{
				DropLocation = HitResult.Location;
			}

			const FRotator DropRotation = FRotator::ZeroRotator;
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			World->SpawnActor<AActor>(ItemData.ItemBPClass, DropLocation, DropRotation, SpawnParams);
		}
	}

	// 슬롯 비우기
	const FStruct_ItemData RemovedData = ItemData;
	ClearSlot(SlotIndex);
	OnItemRemoved.Broadcast(SlotIndex, RemovedData);

	return true;
}

// ═══════════════════════════════════════════════════
// 인벤토리 정렬
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::SortInventory()
{
	// 가방 영역만 정렬 (핫바 0~9는 유지)
	// 비어 있지 않은 슬롯들을 임시 배열에 수집
	TArray<FStruct_InventorySlotData> FilledSlots;

	for (int32 i = BAG_START_INDEX; i < MaxSlots; ++i)
	{
		if (!InventorySlots[i].IsEmpty)
		{
			FilledSlots.Add(InventorySlots[i]);
		}
	}

	// 아이템 ID 기준으로 정렬
	FilledSlots.Sort([](const FStruct_InventorySlotData& A, const FStruct_InventorySlotData& B)
	{
		return A.ItemData.ItemID < B.ItemData.ItemID;
	});

	// 가방 영역 재배치
	int32 FillIndex = 0;
	for (int32 i = BAG_START_INDEX; i < MaxSlots; ++i)
	{
		if (FillIndex < FilledSlots.Num())
		{
			InventorySlots[i] = FilledSlots[FillIndex];
			InventorySlots[i].SlotIndex = i;
			++FillIndex;
		}
		else
		{
			InventorySlots[i] = FStruct_InventorySlotData();
			InventorySlots[i].SlotIndex = i;
			InventorySlots[i].IsEmpty = true;
		}
	}

	OnInventoryFullyUpdated.Broadcast();
}

// ═══════════════════════════════════════════════════
// 핫바 선택
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::SelectHotbar(int32 NewIndex)
{
	if (NewIndex < HOTBAR_START_INDEX || NewIndex > HOTBAR_END_INDEX)
	{
		UE_LOG(LogTemp, Warning, TEXT("SelectHotbar: 유효하지 않은 핫바 인덱스 %d"), NewIndex);
		return;
	}

	const int32 PreviousIndex = CurrentSelectedHotbar;
	CurrentSelectedHotbar = NewIndex;

	if (PreviousIndex != NewIndex)
	{
		OnHotbarSelectionChanged.Broadcast(PreviousIndex, NewIndex);
	}
}

void UAGSDInventoryComponent::CycleHotbar(bool bForward)
{
	int32 NewIndex = CurrentSelectedHotbar + (bForward ? 1 : -1);

	// 순환 로직
	if (NewIndex > HOTBAR_END_INDEX)
	{
		NewIndex = HOTBAR_START_INDEX;
	}
	else if (NewIndex < HOTBAR_START_INDEX)
	{
		NewIndex = HOTBAR_END_INDEX;
	}

	SelectHotbar(NewIndex);
}

// ═══════════════════════════════════════════════════
// Getter
// ═══════════════════════════════════════════════════

FStruct_InventorySlotData UAGSDInventoryComponent::GetSlotData(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < InventorySlots.Num())
	{
		return InventorySlots[SlotIndex];
	}

	UE_LOG(LogTemp, Warning, TEXT("GetSlotData: 유효하지 않은 슬롯 인덱스 %d"), SlotIndex);
	return FStruct_InventorySlotData();
}

FStruct_ItemData UAGSDInventoryComponent::GetCurrentHotbarItemData() const
{
	if (CurrentSelectedHotbar >= 0 && CurrentSelectedHotbar < InventorySlots.Num())
	{
		return InventorySlots[CurrentSelectedHotbar].ItemData;
	}

	return FStruct_ItemData();
}

bool UAGSDInventoryComponent::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < InventorySlots.Num();
}

bool UAGSDInventoryComponent::IsHotbarSlot(int32 SlotIndex) const
{
	return SlotIndex >= HOTBAR_START_INDEX && SlotIndex <= HOTBAR_END_INDEX;
}

// ═══════════════════════════════════════════════════
// GameInstance 연동 (영속성)
// ═══════════════════════════════════════════════════

void UAGSDInventoryComponent::SaveToGameInstance()
{
	// 튜토리얼 레벨에서는 백업하지 않음
	UWorld* World = GetWorld();
	if (World)
	{
		const FString CurrentLevelName = World->GetMapName();
		// GetMapName()이 "UEDPIE_0_Tutorial_Village" 등으로 반환할 수 있으므로 Contains 사용
		if (CurrentLevelName.Contains(TutorialLevelName))
		{
			return;
		}
	}

	USOVGameInstance* GI = GetGameInstance();
	if (!GI) return;

	// 전체 인벤토리 slots 데이터를 통합 백업 (0~MaxSlots-1)
	GI->TempInventory = InventorySlots;

	// 핫바 선택 인덱스 백업
	GI->CurrentSelectedHotbar = CurrentSelectedHotbar;
}

void UAGSDInventoryComponent::LoadFromGameInstance()
{
	USOVGameInstance* GI = GetGameInstance();
	if (!GI) return;

	// 통합 백업 데이터가 존재하는 경우 복원
	if (GI->TempInventory.Num() > 0)
	{
		InventorySlots = GI->TempInventory;

		// 크기가 맞지 않는 경우 안전하게 예외처리 및 패딩
		if (InventorySlots.Num() != MaxSlots)
		{
			InventorySlots.SetNum(MaxSlots);
			for (int32 i = 0; i < MaxSlots; ++i)
			{
				if (InventorySlots[i].SlotIndex != i)
				{
					InventorySlots[i] = FStruct_InventorySlotData();
					InventorySlots[i].SlotIndex = i;
					InventorySlots[i].IsEmpty = true;
				}
			}
		}
		else
		{
			// 슬롯 인덱스 강제 동기화
			for (int32 i = 0; i < MaxSlots; ++i)
			{
				InventorySlots[i].SlotIndex = i;
			}
		}
	}

	CurrentSelectedHotbar = FMath::Clamp(GI->CurrentSelectedHotbar, HOTBAR_START_INDEX, HOTBAR_END_INDEX);
}

// ═══════════════════════════════════════════════════
// 내부 헬퍼
// ═══════════════════════════════════════════════════

USOVGameInstance* UAGSDInventoryComponent::GetGameInstance()
{
	if (!CachedGameInstance)
	{
		CachedGameInstance = Cast<USOVGameInstance>(UGameplayStatics::GetGameInstance(this));
	}
	return CachedGameInstance;
}

bool UAGSDInventoryComponent::ValidateSlotIndex(int32 SlotIndex, const FString& CallerName) const
{
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: 유효하지 않은 슬롯 인덱스 %d (범위: 0~%d)"), *CallerName, SlotIndex, InventorySlots.Num() - 1);
		return false;
	}
	return true;
}
