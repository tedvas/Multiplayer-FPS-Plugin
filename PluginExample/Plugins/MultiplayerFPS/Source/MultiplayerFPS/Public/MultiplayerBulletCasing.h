// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayerBulletCasing.generated.h"

class USceneComponent;
class USoundBase;

UCLASS()
class MULTIPLAYERFPS_API AMultiplayerBulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMultiplayerBulletCasing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BulletCasingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (Tooltip = "Rotate this to control which direction the casing launches"))
	USceneComponent* LaunchRotationScene;

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void LaunchCasing();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Functions")
	virtual void DestroySelf();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void OnCasingHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	virtual void SetOwningGun(AMultiplayerGun* NewOwningPlayer);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	AMultiplayerGun* GetOwningGun();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Functions")
	void PlayCasingHitSound_BP(USoundBase* CasingSound, float CasingSoundVolume);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (ClampMin = 0.0f))
	float CasingHitSoundMaxVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool HitSoundVolumeBaseOnCasingVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "If the casing is at or above this speed then the sound volume will be at max, but any lower than this it be quieter", ClampMin = 0.0f))
	float HitSoundVolumeMinVelocity;

	FTimerHandle DestroySelfTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};