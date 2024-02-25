// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHADOWOFTHEOTHERSIDE_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void Interact(AActor* Interactor, FHitResult Hit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		bool IsInteractable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnStartDetection(AActor* Interactor, FHitResult Hit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void OnFinishDetection();
};
