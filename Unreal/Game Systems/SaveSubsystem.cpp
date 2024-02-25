// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "MainSaveGame.h"	
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "SaveLoadActorInterface.h"
#include "PlayableCharacter.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}


void USaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USaveSubsystem::SaveGame(UObject* WorldContext, FString SlotName)
{
	if (SaveGameSlot == nullptr)
		SaveGameSlot = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));

	SavePlayer(WorldContext);

	TArray<FActorSaveData> ActorSave;

	UWorld* World = WorldContext->GetWorld();

	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;

		if(Actor->IsPendingKill() || !Actor->ActorHasTag(TEXT("SaveObject")) 
			|| IsActorAPlayer(Actor))
			continue;

		if (Actor->Implements<USaveLoadActorInterface>())
			ISaveLoadActorInterface::Execute_OnActorSave(Actor);

		FActorSaveData Data;
		Data.ActorClass = Actor->GetClass();
		Data.ActorName = Actor->GetFName();
		Data.Transform = Actor->GetActorTransform();
		Data.ComponentsSaveData = SaveComponentData(Actor);

		FMemoryWriter Writer(Data.BinaryData);
		FObjectAndNameAsStringProxyArchive Ar(Writer, true);

		Ar.ArIsSaveGame = true;

		Actor->Serialize(Ar);
		ActorSave.Add(Data);
	}

	FLevelSaveData LevelData;
	LevelData.LevelActorData = ActorSave;

	if (!SaveGameSlot->WorldActorData.Contains(World->GetFName()))
		SaveGameSlot->WorldActorData.Add(World->GetFName(), LevelData);
	else
		SaveGameSlot->WorldActorData[World->GetFName()] = LevelData;

	if (!UGameplayStatics::SaveGameToSlot(SaveGameSlot, SlotName, 0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Failed to Save Game");
		return;
	}

	OnSaveGame.Broadcast(SaveGameSlot);
}

void USaveSubsystem::LoadGame(UObject* WorldContext, FString SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		SaveGameSlot = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		InitiateOnActorLoadedCallback(WorldContext);

		return;
	}

	SaveGameSlot = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	UWorld* World = WorldContext->GetWorld();

	//If there are no save data for this level then we just call the OnActorLoaded Interface
	if (!SaveGameSlot->WorldActorData.Contains(World->GetFName()))
	{
		InitiateOnActorLoadedCallback(WorldContext);
		return;
	}

	LoadPlayer(WorldContext);

	FLevelSaveData LevelData = SaveGameSlot->WorldActorData[World->GetFName()];
	TArray<FActorSaveData> SpawnData;

	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;
		bool IsDestroyed = true;

		if (!Actor->ActorHasTag(FName("SaveObject")) || IsActorAPlayer(Actor))
			continue;

		//Load data to all placed actors in world
		for (FActorSaveData ActorData : LevelData.LevelActorData)
		{
			if (Actor->GetFName() == ActorData.ActorName)
			{
				Actor->SetActorTransform(ActorData.Transform);
				
				FMemoryReader Reader(ActorData.BinaryData);

				FObjectAndNameAsStringProxyArchive Ar(Reader, true);
				Ar.ArIsSaveGame= true;

				Actor->Serialize(Ar);
				LoadDataToComponent(Actor, ActorData.ComponentsSaveData);

				SpawnData.Remove(ActorData);

				if (Actor->Implements<USaveLoadActorInterface>())
					ISaveLoadActorInterface::Execute_OnActorLoaded(Actor);

				IsDestroyed = false;
				break;
			}
		}

		if (IsDestroyed)
			Actor->Destroy();
	}

	//Load data to all spawned actors in world
	for (FActorSaveData ActorData : SpawnData)
	{
		AActor* Actor = World->SpawnActor<AActor>(ActorData.ActorClass, ActorData.Transform);

		FMemoryReader Reader(ActorData.BinaryData);

		FObjectAndNameAsStringProxyArchive Ar(Reader, true);
		Ar.ArIsSaveGame = true;

		Actor->Serialize(Ar);
		LoadDataToComponent(Actor, ActorData.ComponentsSaveData);

		if (Actor->Implements<USaveLoadActorInterface>())
			ISaveLoadActorInterface::Execute_OnActorLoaded(Actor);
	}

	OnLoadGame.Broadcast(SaveGameSlot);
}

FName USaveSubsystem::GetLastSaveLevel(FString SlotName, bool& HasSave)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		HasSave = false;
		return FName();
	}

	SaveGameSlot = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
	HasSave = true;

	return SaveGameSlot->PlayerData.CurrentLevel;
}

void USaveSubsystem::SavePlayer(UObject* WorldContext)
{
	if (SaveGameSlot == nullptr)
		return;

	APlayableCharacter* PlayerCharacter = Cast<APlayableCharacter>(UGameplayStatics::GetPlayerCharacter(WorldContext, 0));
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContext, 0);

	FPlayerSavedata PlayerSaveData;
	PlayerSaveData.CurrentLevel = WorldContext->GetFName();
	PlayerSaveData.PlayerTransform = PlayerCharacter->GetActorTransform();
	PlayerSaveData.ControlRotation = PlayerController->GetControlRotation();

	FMemoryWriter CharacterWriter(PlayerSaveData.CharacterBinaryData);
	FMemoryWriter ControllerWriter(PlayerSaveData.ControllerBinaryData);

	FObjectAndNameAsStringProxyArchive CharacterAr(CharacterWriter, true);
	FObjectAndNameAsStringProxyArchive ControllerAr(ControllerWriter, true);

	ControllerAr.ArIsSaveGame = true;
	CharacterAr.ArIsSaveGame = true;

	PlayerSaveData.CharacterComponentsSaveData = SaveComponentData(PlayerCharacter);
	PlayerSaveData.ControllerComponentsSaveData = SaveComponentData(PlayerController);

	PlayerCharacter->Serialize(CharacterAr);
	PlayerController->Serialize(ControllerAr);

	if (PlayerController->Implements<USaveLoadActorInterface>())
	{
		ISaveLoadActorInterface::Execute_OnActorSave(PlayerCharacter);
		ISaveLoadActorInterface::Execute_OnActorSave(PlayerController);
		PlayerSaveData.ControllerCustomData = ISaveLoadActorInterface::Execute_CustomSaveData(PlayerController);
	}

	SaveGameSlot->PlayerData = PlayerSaveData;
}

void USaveSubsystem::LoadPlayer(UObject* WorldContext)
{
	FPlayerSavedata Data = SaveGameSlot->PlayerData;

	APlayableCharacter* PlayerCharacter = Cast<APlayableCharacter>(UGameplayStatics::GetPlayerCharacter(WorldContext, 0));
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContext, 0);

	PlayerController->SetControlRotation(Data.ControlRotation);
	bool success = PlayerCharacter->SetActorLocation(Data.PlayerTransform.GetLocation(), false, nullptr, ETeleportType::ResetPhysics);


	FMemoryReader CharacterReader(Data.CharacterBinaryData);
	FMemoryReader ControllerReader(Data.ControllerBinaryData);

	FObjectAndNameAsStringProxyArchive CharacterAr(CharacterReader, true);
	FObjectAndNameAsStringProxyArchive ControllerAr(ControllerReader, true);

	CharacterAr.ArIsSaveGame = true;
	ControllerAr.ArIsSaveGame = true;


	PlayerCharacter->Serialize(CharacterAr);
	PlayerController->Serialize(ControllerAr);

	LoadDataToComponent(PlayerCharacter, Data.CharacterComponentsSaveData);
	LoadDataToComponent(PlayerController, Data.ControllerComponentsSaveData);

	if (PlayerController->Implements<USaveLoadActorInterface>())
	{
		ISaveLoadActorInterface::Execute_OnActorLoaded(PlayerCharacter);
		ISaveLoadActorInterface::Execute_OnActorLoaded(PlayerController);

		if(Data.ControllerCustomData.Num() > 0)
			ISaveLoadActorInterface::Execute_CustomLoadData(PlayerController, Data.ControllerCustomData);
	}
}

TArray<FActorComponentSaveData> USaveSubsystem::SaveComponentData(AActor* Actor)
{
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(USaveLoadActorInterface::StaticClass());
	TArray<FActorComponentSaveData> SaveComponents;

	for (UActorComponent* Component : ActorComponents)
	{
		FActorComponentSaveData Data;
		Data.ComponentName = Component->GetFName();

		FMemoryWriter Writer(Data.BinaryData);
		FObjectAndNameAsStringProxyArchive Ar(Writer, true);
		Ar.ArIsSaveGame = true;

		Component->Serialize(Ar);
		SaveComponents.Add(Data);

		ISaveLoadActorInterface::Execute_OnActorSave(Component);
	}

	return SaveComponents;
}

void USaveSubsystem::LoadDataToComponent(AActor* Actor, TArray<FActorComponentSaveData> Data)
{
	TArray<UActorComponent*> ActorComponents = Actor->GetComponentsByInterface(USaveLoadActorInterface::StaticClass());

	for (UActorComponent* Component : ActorComponents)
	{
		for (FActorComponentSaveData CompData : Data)
		{
			if (CompData.ComponentName != Component->GetFName())
				continue;

			FMemoryReader Reader(CompData.BinaryData);
			FObjectAndNameAsStringProxyArchive Ar(Reader, true);
			Ar.ArIsSaveGame = true;

			Component->Serialize(Ar);
			ISaveLoadActorInterface::Execute_OnActorLoaded(Component);

			break;
		}
	}
}

bool USaveSubsystem::IsActorAPlayer(AActor* Actor)
{
	return Cast<APlayerController>(Actor) || Cast<APlayableCharacter>(Actor);
}

void USaveSubsystem::InitiateOnActorLoadedCallback(UObject* WorldContext)
{
	for(FActorIterator It(WorldContext->GetWorld()); It; ++It)
	{
		AActor* Actor = *It;

		if(!Actor->ActorHasTag(FName("SaveObject")))
			continue;

		if (Actor->Implements<USaveLoadActorInterface>())
			ISaveLoadActorInterface::Execute_OnActorLoaded(Actor);

		TArray<UActorComponent*> Components = Actor->GetComponentsByInterface(USaveLoadActorInterface::StaticClass());

		for (UActorComponent* Comp : Components)
		{
			ISaveLoadActorInterface::Execute_OnActorLoaded(Comp);
		}

	}
}
