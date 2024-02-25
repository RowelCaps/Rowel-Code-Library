// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UENUM(BlueprintType)
enum class EDetectionType : uint8
{
	PlayerCamera,
	AI
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionComponentSignature, AActor*, Actor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHADOWOFTHEOTHERSIDE_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:")
		EDetectionType DetectionType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:")
		bool bDebugMode = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:", meta = (ClampMin = "0")	)
		float DebugDrawDuration = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:")
		float Distance = 300.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:")
		TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction Settings:")
		TEnumAsByte<ETraceTypeQuery> UsableActorChannel;

	UPROPERTY(BlueprintAssignable, Category = "Interaction Settings:")
		FInteractionComponentSignature OnBeginDetection;

	UPROPERTY(BlueprintAssignable, Category = "Interaction Settings:")
		FInteractionComponentSignature OnEndDetection;

private:

	FHitResult InteractableDetected;

	bool bInteractionActive = true;

public:

	//Interacts with the detected object
	UFUNCTION(BlueprintCallable, Category = "Interaction Component")
		void InteractObject();

	//Line trace from the camera location
	UFUNCTION(BlueprintCallable, Category = "Interaction Component")
		bool LineTraceFromPlayerCamera(FHitResult& OutHit);

	UFUNCTION(BlueprintCallable, Category = "Interaction Component")
		void SetInteractionActive(bool bVal);

	//Checks if we detected an interactable
	UFUNCTION(BlueprintCallable, Category = "Interaction Component")
		bool HasDetectedInteractable() { return InteractableDetected.Actor.Get() != nullptr; }

	//Get Current Interactable Detected
	UFUNCTION(BlueprintPure, Category = "Interaction Component")
		FORCEINLINE AActor* GetDetectedInteractable() { return InteractableDetected.Actor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction Component")
		FORCEINLINE bool IsInteractionActive() { return bInteractionActive; }

	UFUNCTION(BlueprintPure, Category = "Interaction Component")
		bool GetUsableItemActor(AActor*& UsableItemActor);

private:

	void DetectInteractablesFromPlayerCamera();

};
