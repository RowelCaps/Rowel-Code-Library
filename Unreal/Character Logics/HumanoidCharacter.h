// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterDataTypes.h"
#include "AnimationSynchData.h"
#include "HumanoidCharacter.generated.h"

class UInteractionComponent;
class USoundCue;
class UAudioComponent;
class USoundbase;

DECLARE_DYNAMIC_DELEGATE(FMoveDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGaitSignature, EGaitType, Gait);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActionModeSignature, EActionMode, ActionMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkeletalSynchSignature, FSkeletalSynchParams, Params);

UCLASS()
class SHADOWOFTHEOTHERSIDE_API AHumanoidCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHumanoidCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UAudioComponent* VoiceAudioComponent;

public:

	FORCEINLINE UAudioComponent* GetVoiceAudioComponent() { return VoiceAudioComponent; }

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Settings:|Gait")
		float WalkSpeed = 150;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Settings:|Gait")
		float RunSpeed = 300;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character Settings:|Gait")
		float CrouchSpeed = 50;

public:

	UPROPERTY(BlueprintAssignable)
		FGaitSignature OnGaitChange;

	UPROPERTY(BlueprintAssignable)
		FActionModeSignature OnActionModeChange;


protected:

	EGaitType Gait;
	EStanceType Stance;
	EActionMode ActionMode;

public:

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void SetGait(EGaitType NewGait);

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void SetStance(EStanceType NewStance);

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void SetActionMode(EActionMode NewActionMode);

	UFUNCTION(BlueprintPure, Category = "Humanoid Character")
		float GetMaxGaitSpeed();

	UFUNCTION(BlueprintPure, Category = "Humanoid Character")
		FORCEINLINE EGaitType GetGaitType() { return Gait; }

	UFUNCTION(BlueprintPure, Category = "Humanoid Character")
		FORCEINLINE EStanceType GetStance() { return Stance; }

	UFUNCTION(BlueprintPure, Category = "Humanoid Character")
		FORCEINLINE EActionMode GetActionMode() { return ActionMode; }

public:

	void MoveForward(float Val);
	void MoveRight(float Val);

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Synching Settings:")
		float SynchStoppingDistance = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation Synching Settings:")
		float RotationDuration = 0.1f;

protected:

	FAnimationSynchParams AnimSynchValue;

	FTimerHandle AnimationSynchHandle;

public:

	//Start Synching animation by moving the character to the play location
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Humanoid Character")
		void StartAnimationSynching(FAnimationSynchParams SynchParams);
	virtual void StartAnimationSynching_Implementation(FAnimationSynchParams SynchParams);

	//Callback when the character has arrive at the play location
	UFUNCTION(BlueprintNativeEvent, Category = "Humanoid Character")
		void OnEndMovingToInteraction();
	virtual void OnEndMovingToInteraction_Implementation();

	//Sets the timer to loop every 0.001 second and calls UpdateSynchAnimation
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Humanoid Character")
		void StartUpdatingCurves();
	virtual void StartUpdatingCurves_Implementation();

	//Passes the curve valus to the pair
	UFUNCTION(BlueprintNativeEvent, Category = "Humanoid Character")
		void UpdateSynchAnimation();
	virtual void UpdateSynchAnimation_Implementation();

	//Called upon blending out in the montage
	UFUNCTION(BlueprintNativeEvent, Category = "Humanoid Character")
		void OnSynchBlendOut(UAnimMontage* Montage, bool bInterrupted);
	virtual void OnSynchBlendOut_Implementation(UAnimMontage* Montage, bool bInterrupted);

	//Called when montage is finished
	UFUNCTION(BlueprintNativeEvent, Category = "Humanoid Character")
		void OnEndSynch(UAnimMontage* Montage, bool bInterrupted);
	virtual void OnEndSynch_Implementation(UAnimMontage* Montage, bool bInterrupted);

public:

	UPROPERTY(BlueprintAssignable, Category = "Humanoid Character")
		FSkeletalSynchSignature OnSkeletalSynchBegin;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid Character")
		FSkeletalSynchSignature OnSkeletalSynchEnd;

protected:

	FSkeletalSynchParams SkeletalSynchParams;

public:

	//The pair and this humanoid will disable all its functionality and 
	//Attach the pair to this humanoid and play their montage at the same time
	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void StartAnimSynchSkeletalPair(FSkeletalSynchParams SynchParams);

	UFUNCTION()
		void OnEndSynchSkeletal(UAnimMontage* Montage, bool bInterrupted);

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:", DisplayName = "Trace Distance")
		TEnumAsByte<ETraceTypeQuery> FootTraceChannel;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:", DisplayName = "Trace Distance")
		float FootstepTraceDistance = 100;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:")
		float WalkNoise = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:")
		float RunNoise = 1;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:")
		USoundAttenuation* FootstepAttenuation;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:")
		TMap<TEnumAsByte<EPhysicalSurface>, USoundCue*> FootStepSoundMap;

public:

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void PlayFootstepSound();

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voice Audio")
		class UDataTable* DialogueTable;

public:

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void PlayVoiceSound(USoundBase* SoundBase, FText Subtitles, bool bShowSubtitles = false);

	UFUNCTION(BlueprintCallable, Category = "Humanoid Character")
		void PlayVoiceSoundByDataTable(FName RowName);
};
