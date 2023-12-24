// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayerBulletCasing.generated.h"

class USceneComponent;

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerBulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiplayerBulletCasing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BulletCasingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (Tooltip = "Rotate this to control which direction the casing launches"))
	USceneComponent* LaunchRotationScene;

	UFUNCTION(BlueprintCallable)
	virtual void LaunchCasing();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable)
	virtual void OnCasingHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	virtual void SetOwningGun(AMultiplayerGun* NewOwningPlayer);

	UFUNCTION(BlueprintCallable)
	AMultiplayerGun* GetOwningGun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated)
	AMultiplayerGun* OwningGun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launching")
	float LaunchStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launching", meta = (Tooltip = "If true, the strength is taken as a change in velocity instead of an impulse (ie. mass will have no effect)"))
	bool LaunchVelocityChange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destroy Self")
	float DelayToDestroySelf;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* CasingHitDefaultSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "Using the physical material allows you to have a different sound for each surface"))
	TMap<USoundBase*, UPhysicalMaterial*> CasingHitSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float CasingHitSoundMaxVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool HitSoundVolumeBaseOnCasingVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "If the casing is at or above this speed then the sound volume will be at max, but any lower than this it be quieter"))
	float HitSoundVolumeMinVelocity;

	FTimerHandle DestroySelfTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};