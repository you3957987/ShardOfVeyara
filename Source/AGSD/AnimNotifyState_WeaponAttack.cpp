// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_WeaponAttack.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void UAnimNotifyState_WeaponAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp)
	{
		// 시작 시 타격 배열 초기화 및 시작 소켓 위치 저장
		HitActorsMap.Add(MeshComp, TArray<AActor*>());
		PreviousLocationMap.Add(MeshComp, MeshComp->GetSocketLocation(SocketName));
		Player = Cast<AAGSDCharacter>(MeshComp->GetOwner());
		if (Player)
		{
			// 기본 데미지 설정
			BaseDamage = Player->WeaponDamage();
		}
	}
}

void UAnimNotifyState_WeaponAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;

	// 이전 위치가 맵에 없다면 무시
	if (!PreviousLocationMap.Contains(MeshComp)) return;

	FVector PreviousLocation = PreviousLocationMap[MeshComp];
	FVector CurrentLocation = MeshComp->GetSocketLocation(SocketName);

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MeshComp->GetOwner()); // 무기 소유자(자신)는 타격 무시
	
	// 이미 타격한 액터들을 트레이스 단계에서부터 아예 무시되도록 추가 (최적화)
	TArray<AActor*>& HitActors = HitActorsMap[MeshComp];
	ActorsToIgnore.Append(HitActors);

	// Multi Sphere Trace by Channel 수행 (찌르기/휘두르기 궤적 커버)
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		MeshComp->GetWorld(),
		PreviousLocation,
		CurrentLocation,
		TraceRadius,
		WeaponTraceChannel,
		false,
		ActorsToIgnore,
		DrawDebugTrace, // 에디터에서 설정한 디버그 트레이스 모드 사용
		HitResults,
		true
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			// 유효한 액터인지, 이번 공격에서 이미 맞은 액터가 아닌지, 그리고 타겟 태그를 가지고 있는지 확인
			if (HitActor && !HitActors.Contains(HitActor) && HitActor->ActorHasTag(TargetTag))
			{
				HitActors.Add(HitActor); // 중복 방지 등록
				AController* InstigatorController = MeshComp->GetOwner() ? MeshComp->GetOwner()->GetInstigatorController() : nullptr;
				
				// 타격 방향 계산
				FVector AttackDirection = (CurrentLocation - PreviousLocation).GetSafeNormal();

				// 데미지 가하기
				UGameplayStatics::ApplyPointDamage(
					HitActor,
					BaseDamage,
					AttackDirection,
					Hit,
					InstigatorController,
					MeshComp->GetOwner(),
					UDamageType::StaticClass()
				);

				// 타격 이펙트 (나이아가라) 재생
				if (HitEffect)
				{
					// Hit.ImpactPoint : 타격된 정확한 위치
					// Hit.ImpactNormal.Rotation() : 타격면의 법선 방향 (이펙트가 표면을 향하도록 설정)
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						MeshComp->GetWorld(),
						HitEffect,
						Hit.ImpactPoint,
						Hit.ImpactNormal.Rotation()
					);
				}
			}
		}
	}

	// 다음 프레임을 위해 현재 위치를 갱신 (연속성 유지)
	PreviousLocationMap[MeshComp] = CurrentLocation;
}

void UAnimNotifyState_WeaponAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 몽타주 종료 시 메모리에서 맵핑 정보 제거
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
		PreviousLocationMap.Remove(MeshComp);
	}
}
