// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestSystem.h"
#include "Quest.h"
#include "SaveDataType.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void UQuestSystem::StartQuest(UQuest* Quest)
{
	if (Quest == nullptr)
		return;
	
	Quest->BeginQuest();
	Quest->OnEndQuest.AddDynamic(this, &UQuestSystem::OnFinishedQuest);

	CurrentQuest = Quest;
	CurrentQuestClass = CurrentQuest->GetClass();

	OnStartQuest.Broadcast(CurrentQuest);
}

void UQuestSystem::StartQuestByClass(TSubclassOf<UQuest> QuestClass)
{
	if (QuestClass == nullptr)
		return;

	UQuest* NewQuest = NewObject<UQuest>(this, QuestClass);
	NewQuest->InitializeQuest();

	StartQuest(NewQuest);
}

void UQuestSystem::SendQuestSignal(UObject* Sender, FName SignalName)
{
	if (CurrentQuest == nullptr)
		return;

	CurrentQuest->ReceiveQuestSignal(Sender, SignalName);
}

void UQuestSystem::OnFinishedQuest()
{
	if (CurrentQuest == nullptr)
		return;

	FinishedQuest.Add(CurrentQuest->GetClass());
	OnFinishQuest.Broadcast(CurrentQuest);

	CurrentQuest = nullptr;
	CurrentQuestClass = nullptr;
}

void UQuestSystem::ForceEndQuest()
{
	if (CurrentQuest == nullptr)
		return;

	CurrentQuest->EndQuest();
}

bool UQuestSystem::IsQuestFinished(TSubclassOf<UQuest> QuestClass)
{
	return false;
}

TArray<uint8> UQuestSystem::SaveData()
{
	TArray<uint8> Data;

	FMemoryWriter Writer(Data);
	FObjectAndNameAsStringProxyArchive Ar(Writer, true);
	Ar.ArIsSaveGame = true;

	this->Serialize(Ar);

	return Data;
}

void UQuestSystem::LoadData(TArray<uint8> Data)
{
	FMemoryReader Reader(Data);
	FObjectAndNameAsStringProxyArchive Ar(Reader, true);
	Ar.ArIsSaveGame = true;

	Serialize(Ar);

	if (CurrentQuestClass == nullptr)
		return;

	StartQuestByClass(CurrentQuestClass);
}
