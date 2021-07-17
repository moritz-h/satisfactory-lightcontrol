#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "LightControlSubsystem.generated.h"

/**
 *
 */
UCLASS()
class LIGHTCONTROL_API ALightControlSubsystem : public AModSubsystem
{
    GENERATED_BODY()

    ALightControlSubsystem();

    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void Tick(float DeltaSeconds) override;
};
