// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EventMessagingSystem.generated.h"

/**
 * 
 */
UCLASS()
class SHADOWOFTHEOTHERSIDE_API UEventMessagingSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "Sender"))
		static void SendMessageByTag(UObject* Sender, FName ReceiverTag, FName EventName);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "Sender"))
		static void SendMessageByClass(UObject* Sender, UClass* ReceiverClass, FName EventName);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "Sender"))
		static void SendMessageToAllReceivers(UObject* Sender, FName EventName);


private:

	static void GetReceiverActors(UObject* ContextObject, TArray<AActor*>& OutActors);
};
