// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveLoadActorInterface.h"
#include "QuestSystem.generated.h"

class UQuest;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuestSystemDelegate, UQuest*, Quest);

UCLASS(BlueprintType, Blueprintable)
class SHADOWOFTHEOTHERSIDE_API UQuestSystem : public UObject
{
	GENERATED_BODY()
	
protected:

	UPROPERTY()
		UQuest* CurrentQuest;

	UPROPERTY(SaveGame)
		TSubclassOf<UQuest> CurrentQuestClass;

	UPROPERTY(SaveGame)
		TArray<uint8> QuestTaskData;

	UPROPERTY(SaveGame)
		TArray<TSubclassOf<UQuest>> FinishedQuest;

public:

	UPROPERTY(BlueprintAssignable, Category = "Quest System")
		FQuestSystemDelegate OnStartQuest;

	UPROPERTY(BlueprintAssignable, Category = "Quest System")
		FQuestSystemDelegate OnFinishQuest;

public:

	UFUNCTION(BlueprintCallable, Category = "Quest System")
		void StartQuest(UQuest* Quest);

	//Starts a quest if there are no current quest
	UFUNCTION(BlueprintCallable, Category = "Quest System")
		void StartQuestByClass(TSubclassOf<UQuest> QuestClass);

	UFUNCTION(BlueprintCallable, Category = "Quest System")
		void SendQuestSignal(UObject* Sender, FName SignalName);

	UFUNCTION(BlueprintCallable, Category = "Quest System")
		void OnFinishedQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
		void ForceEndQuest();

	UFUNCTION(BlueprintPure, Category = "Quest System")
		bool IsQuestFinished(TSubclassOf<UQuest> QuestClass);

	UFUNCTION(BlueprintPure, Category = "Quest System")
		FORCEINLINE bool HasCurrentQuest() { return CurrentQuest != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Quest System")
		FORCEINLINE UQuest* GetCurrentQuest() { return CurrentQuest; }

	UFUNCTION(BlueprintPure, Category = "Quest System")
		FORCEINLINE TArray<TSubclassOf<UQuest>> GetFinishedQuest() { return FinishedQuest; }

public:

	TArray<uint8> SaveData();
	void LoadData(TArray<uint8> Data);
};
