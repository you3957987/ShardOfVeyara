#include "Enemy/BaseExploderEnemy.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"

ABaseExploderEnemy::ABaseExploderEnemy()
{
	ExplosionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionRangeSphere"));
	ExplosionRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트
	ExplosionRangeSphere->ShapeColor = FColor::Yellow;
	ExplosionRangeSphere->SetVisibility(false);
	ExplosionRangeSphere->SetHiddenInGame(false); 
}

void ABaseExploderEnemy::Explode()
{
	// 한 프레임 오버랩 검사: ExplosionRangeSphere와 겹치는 액터들을 가져와 태그로 필터링
	if (ExplosionRangeSphere)
	{
		TArray<AActor*> OverlappingActors;
		ExplosionRangeSphere->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor->ActorHasTag(FName("Player")))
			{
				UE_LOG(LogTemp, Warning, TEXT("Explode overlap with Player: %s"), *Actor->GetName());
			}
		}
	}
	
	if (ExplosionEffect && ExplosionRangeSphere && GetWorld() )
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionEffect,
			ExplosionRangeSphere->GetComponentLocation(),
			ExplosionRangeSphere->GetComponentRotation()
		);
	}
	Die(); // 폭발 후 죽음 처리
}

#if WITH_EDITOR
void ABaseExploderEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr)
									? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseExploderEnemy, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( ExplosionRangeSphere ) ExplosionRangeSphere->SetVisibility(true);
		}
		else
		{
			if ( ExplosionRangeSphere ) ExplosionRangeSphere->SetVisibility(false);
		}
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseExploderEnemy, ExplosionRange))
	{
		if (ExplosionRangeSphere)
		{
			ExplosionRangeSphere->SetSphereRadius(ExplosionRange);
		}
	}
}
#endif