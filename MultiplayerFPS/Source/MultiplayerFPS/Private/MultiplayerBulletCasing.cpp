// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerBulletCasing.h"
#include "Components/StaticMeshComponent.h"
#include "MultiplayerGun.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMultiplayerBulletCasing::AMultiplayerBulletCasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BulletCasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Casing Mesh"));
	RootComponent = BulletCasingMesh;
	BulletCasingMesh->SetSimulatePhysics(true);
	BulletCasingMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	LaunchRotationScene = CreateDefaultSubobject<USceneComponent>(TEXT("Launch Rotation Scene"));
	LaunchRotationScene->SetupAttachment(BulletCasingMesh, NAME_None);

	BulletCasingMesh->OnComponentHit.AddDynamic(this, &AMultiplayerBulletCasing::OnCasingHit);

	LaunchStrength = 200.0f;
	LaunchVelocityChange = true;
	DelayToDestroySelf = 10.0f;
	CasingHitSoundMaxVolume = 1.0f;
	HitSoundVolumeBaseOnCasingVelocity = true;
	HitSoundVolumeMinVelocity = 200.0f;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AMultiplayerBulletCasing::LaunchCasing()
{
	if (LaunchRotationScene)
	{
		BulletCasingMesh->AddImpulse((LaunchRotationScene->GetRightVector() * LaunchStrength) + (LaunchRotationScene->GetUpVector() * (LaunchStrength / 2.0f)), NAME_None, LaunchVelocityChange);
	}
}

void AMultiplayerBulletCasing::OnCasingHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	TArray<USoundBase*> HitSounds;
	TArray<UPhysicalMaterial*> HitMaterials;
	
	CasingHitSounds.GenerateKeyArray(HitSounds);
	CasingHitSounds.GenerateValueArray(HitMaterials);

	int IndexOfSound = 0;
	USoundBase* SoundToPlay;
	UPhysicalMaterial* HitMaterial = Hit.PhysMaterial.Get();

	float SoundVolume = CasingHitSoundMaxVolume;
	float VectorLength = UKismetMathLibrary::VSize(BulletCasingMesh->GetPhysicsLinearVelocity());

	if (VectorLength > 0)
	{
		if (HitSoundVolumeBaseOnCasingVelocity == true)
		{
			SoundVolume = VectorLength / HitSoundVolumeMinVelocity;

			if (SoundVolume > CasingHitSoundMaxVolume)
			{
				SoundVolume = CasingHitSoundMaxVolume;
			}

			if (SoundVolume < 0)
			{
				SoundVolume = 0;
			}
		}
		else
		{
			SoundVolume = CasingHitSoundMaxVolume;
		}
	}

	if (HitMaterials.Contains(HitMaterial))
	{
		IndexOfSound = HitMaterials.Find(HitMaterial);
		SoundToPlay = HitSounds[IndexOfSound];
	}
	else
	{
		SoundToPlay = CasingHitDefaultSound;
	}

	if (SoundToPlay && SoundVolume > 0)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetActorLocation(), SoundVolume);
	}
}

void AMultiplayerBulletCasing::SetOwningGun(AMultiplayerGun* NewOwningPlayer)
{
	OwningGun = NewOwningPlayer;
}

AMultiplayerGun* AMultiplayerBulletCasing::GetOwningGun()
{
	return OwningGun;
}

void AMultiplayerBulletCasing::DestroySelf_Implementation()
{
	if (OwningGun)
	{
		OwningGun->SpawnedBulletCasings.Remove(this);
	}

	Destroy();
}

// Called when the game starts or when spawned
void AMultiplayerBulletCasing::BeginPlay()
{
	Super::BeginPlay();

	LaunchCasing();

	GetWorldTimerManager().SetTimer(DestroySelfTimerHandle, this, &AMultiplayerBulletCasing::DestroySelf, DelayToDestroySelf, false, DelayToDestroySelf);
}

// Called every frame
void AMultiplayerBulletCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMultiplayerBulletCasing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerBulletCasing, OwningGun);
}