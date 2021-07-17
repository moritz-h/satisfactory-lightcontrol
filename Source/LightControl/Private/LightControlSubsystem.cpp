#include "LightControlSubsystem.h"

ALightControlSubsystem::ALightControlSubsystem()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
}

void ALightControlSubsystem::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("########## ALightControlSubsystem::BeginPlay"));
}

void ALightControlSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    UE_LOG(LogTemp, Warning, TEXT("########## ALightControlSubsystem::EndPlay"));
}

void ALightControlSubsystem::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UE_LOG(LogTemp, Warning, TEXT("########## ALightControlSubsystem::Tick"));
}
