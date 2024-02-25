// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanoidCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InteractionComponent.h"	
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "AnimationSynchingInterface.h"
#include "RowelSystemLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "SolitaryGameMode.h"
#include "DialogueDataTypes.h"

// Sets default values
AHumanoidCharacter::AHumanoidCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Voice Audio Component"));
	VoiceAudioComponent->SetupAttachment(GetMesh(), FName("head"));
}

// Called when the game starts or when spawned
void AHumanoidCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

// Called every frame
void AHumanoidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHumanoidCharacter::SetGait(EGaitType NewGait)
{
	if (NewGait == Gait)
		return;

	Gait = NewGait;

	switch (Gait)
	{
		case EGaitType::Walking:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;

		case EGaitType::Running:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;

		default:
			break;
	}

	OnGaitChange.Broadcast(Gait);
}

void AHumanoidCharacter::SetStance(EStanceType NewStance)
{
	if (NewStance == Stance)
		return;

	Stance = NewStance;

	switch (Stance)
	{
		case EStanceType::Standing:
			UnCrouch();
			break;

		default:
			Crouch();
			break;
	}
}

void AHumanoidCharacter::SetActionMode(EActionMode NewActionMode)
{
	if (ActionMode == NewActionMode)
		return;

	switch (NewActionMode)
	{
		case EActionMode::Disabled:
			SetActorEnableCollision(false);
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

			break;

		default:
			SetActorEnableCollision(true);
			GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
			break;


	}

	ActionMode = NewActionMode;
	OnActionModeChange.Broadcast(ActionMode);
}

float AHumanoidCharacter::GetMaxGaitSpeed()
{
	if (Stance == EStanceType::Crouching)
		return CrouchSpeed;

	switch (Gait)
	{
		case EGaitType::Running:
			return RunSpeed;

		default:
			return WalkSpeed;
	}
}

void AHumanoidCharacter::MoveForward(float Val)
{
	if (Controller == nullptr && Val == 0.0f)
		return;

	const FRotator Rotation = Controller->GetControlRotation();	
	const FRotator YawRotation = FRotator(0, Rotation.Yaw, 0);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Val);
}

void AHumanoidCharacter::MoveRight(float Val)
{
	if (Controller == nullptr && Val == 0.0f)
		return;

	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, "Tuuturuu");
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation = FRotator(0, Rotation.Yaw, 0);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Val);
}

void AHumanoidCharacter::StartAnimationSynching_Implementation(FAnimationSynchParams SynchParams)
{
	if (!SynchParams.IsValid() || !SynchParams.Pair->GetClass()->ImplementsInterface(UAnimationSynchingInterface::StaticClass()))
		return;

	AnimSynchValue = SynchParams;
	AnimSynchValue.InteractLocation.Z = GetActorLocation().Z;

	//Bind OnEndMovingInteraction and pass it to MoveTo Function
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "OnEndMovingToInteraction";
	LatentInfo.Linkage = 1;

	//Moves the root component
	URowelSystemLibrary::MoveHumanoidTo(GetWorld(), this, SynchParams.InteractLocation, SynchParams.InteractRotation, SynchStoppingDistance,RotationDuration, EMoveHumanoidAction::Move, LatentInfo);
}

void AHumanoidCharacter::OnEndMovingToInteraction_Implementation()
{
	SetActionMode(EActionMode::Montage);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	AnimInstance->OnMontageEnded.AddDynamic(this, &AHumanoidCharacter::OnEndSynch);
	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &AHumanoidCharacter::OnSynchBlendOut);

	//Plays the montage for synching
	const float MontageLength = AnimInstance->Montage_Play(AnimSynchValue.Montage);

	//Call the OnStartSynching function of the pair
	IAnimationSynchingInterface::Execute_OnStartSynching(AnimSynchValue.Pair, this);
}

void AHumanoidCharacter::StartUpdatingCurves_Implementation()
{
	//Sets the timer to play every end of the frame
	GetWorld()->GetTimerManager().SetTimer(AnimationSynchHandle, this, &AHumanoidCharacter::UpdateSynchAnimation, 0.001, true);
}

void AHumanoidCharacter::UpdateSynchAnimation_Implementation()
{
	if (!AnimSynchValue.Pair->GetClass()->ImplementsInterface(UAnimationSynchingInterface::StaticClass()))
		return;

	AActor* Pair = AnimSynchValue.Pair;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	//Get all curve values of AnimSynchValues and pass it to the actor pair
	for (FName Curve : AnimSynchValue.Curves)
	{
		float CurveValue = AnimInstance->GetCurveValue(Curve);
		IAnimationSynchingInterface::Execute_SynchUpdate(Pair, FAnimationSynchReceiverData(Curve, CurveValue), this);
	}
}

void AHumanoidCharacter::OnSynchBlendOut_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	if (!AnimSynchValue.Pair->GetClass()->ImplementsInterface(UAnimationSynchingInterface::StaticClass()))
		return;

	GetWorld()->GetTimerManager().ClearTimer(AnimationSynchHandle);
	IAnimationSynchingInterface::Execute_OnBlendOut(AnimSynchValue.Pair, this);
}

void AHumanoidCharacter::OnEndSynch_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	if (!AnimSynchValue.Pair->GetClass()->ImplementsInterface(UAnimationSynchingInterface::StaticClass()))
		return;

	AActor* Pair = AnimSynchValue.Pair;
	IAnimationSynchingInterface::Execute_OnEndSyching(Pair, this);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	AnimInstance->OnMontageEnded.Remove(this, "OnEndSynch");
	AnimInstance->OnMontageBlendingOut.Remove(this, "OnSynchBlendOut");
}

void AHumanoidCharacter::StartAnimSynchSkeletalPair(FSkeletalSynchParams SynchParams)
{
	if (!SynchParams.IsValid())
		return;

	SynchParams.Pair->SetActionMode(EActionMode::Montage);
	SetActionMode(EActionMode::Montage);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->OnMontageEnded.AddDynamic(this, &AHumanoidCharacter::OnEndSynchSkeletal);

	SynchParams.Pair->SetActorLocation(SynchParams.SynchLocation);
	SynchParams.Pair->SetActorRotation(SynchParams.SynchRotation);

	SynchParams.Pair->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	AnimInstance->Montage_Play(SynchParams.Montage);
	SynchParams.Pair->GetMesh()->GetAnimInstance()->Montage_Play(SynchParams.PairMontage);

	SkeletalSynchParams = SynchParams;
	OnSkeletalSynchBegin.Broadcast(SynchParams);
}

void AHumanoidCharacter::OnEndSynchSkeletal(UAnimMontage* Montage, bool bInterrupted)
{
	if (!SkeletalSynchParams.IsValid())
		return;

	OnSkeletalSynchEnd.Broadcast(SkeletalSynchParams);
}

void AHumanoidCharacter::PlayFootstepSound()
{
	FVector StartLoc = GetMesh()->DoesSocketExist(FName("Footstep Audio Socket")) ?
		GetMesh()->GetSocketLocation(FName("Footstep Audio Socket")) :
		GetActorLocation();

	FVector EndLoc = StartLoc + (FVector::DownVector * FootstepTraceDistance);

	TArray<AActor*> IgnoredActor;
	IgnoredActor.Add(GetOwner());

	FHitResult OutHit;

	USoundCue* FootStepSound = UKismetSystemLibrary::LineTraceSingle(
		this, 
		StartLoc, 
		EndLoc, 
		FootTraceChannel, 
		true, 
		IgnoredActor,
		EDrawDebugTrace::None, 
		OutHit, 
		true) ?
			FootStepSoundMap[UGameplayStatics::GetSurfaceType(OutHit)] :
			FootStepSoundMap[EPhysicalSurface::SurfaceType_Default];

	if (FootStepSound == nullptr || FootstepAttenuation == nullptr)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, FootStepSound, OutHit.ImpactPoint, 1,1,0, FootstepAttenuation);
	MakeNoise(GetGaitType() == EGaitType::Walking ? WalkNoise : RunNoise, nullptr, StartLoc, 0.0f, FName("Noise"));
}

void AHumanoidCharacter::PlayVoiceSound(USoundBase* SoundBase, FText Subtitles, bool bShowSubtitles)
{
	VoiceAudioComponent->SetSound(SoundBase);
	VoiceAudioComponent->Play();

	ASolitaryGameMode* GameMode = Cast<ASolitaryGameMode>(UGameplayStatics::GetGameMode(this));

	if (GameMode == nullptr || !bShowSubtitles)
		return;

	GameMode->DisplaySubtitles(Subtitles, VoiceAudioComponent);
}

void AHumanoidCharacter::PlayVoiceSoundByDataTable(FName RowName)
{
	if (DialogueTable == nullptr)
		return;

	FString ContextString;
	FDialogueData* Data = DialogueTable->FindRow<FDialogueData>(RowName, ContextString);

	if (Data == nullptr)
		return;

	if (Data->SoundFile == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Sound File is missing!"));
		return;
	}

	PlayVoiceSound(Data->SoundFile, Data->EnglishDialogue);
}
