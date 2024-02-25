// Fill out your copyright notice in the Description page of Project Settings.


#include "EventMessagingSystem.h"
#include "EventReceiverInterface.h"
#include "ExamineObject.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

void UEventMessagingSystem::SendMessageByTag(UObject* Sender, FName ReceiverTag, FName EventName)
{
	TArray<AActor*> ReceiverActors;
	GetReceiverActors(Sender, ReceiverActors);

	for (AActor* Actor : ReceiverActors)
	{
		if (Actor->ActorHasTag(ReceiverTag))
			IEventReceiverInterface::Execute_OnReceiveEvent(Actor, EventName, Sender);
	}
}

void UEventMessagingSystem::SendMessageByClass(UObject* Sender, UClass* ReceiverClass, FName EventName)
{
	TArray<AActor*> ReceiverActors;
	GetReceiverActors(Sender, ReceiverActors);

	for (AActor* Actor : ReceiverActors)
	{
		if(Actor->GetClass() == ReceiverClass)
			IEventReceiverInterface::Execute_OnReceiveEvent(Actor, EventName, Sender);
	}
}

void UEventMessagingSystem::SendMessageToAllReceivers(UObject* Sender, FName EventName)
{
	if (Sender == nullptr)
		return;

	TArray<AActor*> ReceiverActors;
	GetReceiverActors(Sender, ReceiverActors);

	for (AActor* Actor : ReceiverActors)
	{
		IEventReceiverInterface::Execute_OnReceiveEvent(Actor, EventName, Sender);
	}
}

void UEventMessagingSystem::GetReceiverActors(UObject* ContextObject, TArray<AActor*>& OutActors)
{
	//Cut down memcpy operations
	//OutActors.Empty(512);

	UWorld* World = GEngine->GetWorldFromContextObject(ContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!IsValid(World))
		return;

	TArray<ULevel*> Levels = World->GetLevels();

	for (ULevel* Level : Levels)
	{
		if (IsValid(Level) && Level->bIsVisible)
			OutActors.Append(Level->Actors);
	}
	
	

	for (int32 i = 0; i < OutActors.Num(); i++)
	{
		if (OutActors[i] != nullptr)
		{
			if (OutActors[i]->Implements<UEventReceiverInterface>())
				continue;
		}

		OutActors.RemoveAt(i, 1, false);
		i--;
	}

	OutActors.Shrink();
}