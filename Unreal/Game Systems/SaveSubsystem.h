// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveDataType.h"
#include "SaveSubsystem.generated.h"

class UMainSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveGame, UMainSaveGame*, SaveGameSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadGame, UMainSaveGame*, SaveGameSlot);

UCLASS()
class SHADOWOFTHEOTHERSIDE_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

protected:

	UPROPERTY()
		UMainSaveGame* SaveGameSlot;
	
public:

	UPROPERTY(BlueprintAssignable, Category = "Save Subsystem")
		FOnSaveGame OnSaveGame;

	UPROPERTY(BlueprintAssignable, Category = "Save Subsystem")
		FOnLoadGame OnLoadGame;

public:

	UFUNCTION(BlueprintCallable, Category = "Save Subsystem", meta = (WorldContext = "WorldContext"))
		void SaveGame(UObject* WorldContext,const FString SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save Subsystem", meta = (WorldContext = "WorldContext"))
		void LoadGame(UObject* WorldContext, const FString SlotName);

	UFUNCTION(BlueprintPure, Category = "Save Subsystem")
		FName GetLastSaveLevel(FString SlotName, bool& HasSave);

	UFUNCTION(BlueprintPure, Category = "Save Subsystem")
		FORCEINLINE UMainSaveGame* GetSaveGameObject() { return SaveGameSlot;  }

private:

	void SavePlayer(UObject* WorldContext);
	void LoadPlayer(UObject* WorldContext);

	TArray<FActorComponentSaveData> SaveComponentData(AActor* Actor);
	void LoadDataToComponent(AActor* Actor, TArray<FActorComponentSaveData> Data);

	bool IsActorAPlayer(AActor* Actor);

	void InitiateOnActorLoadedCallback(UObject* WorldContext);
};
