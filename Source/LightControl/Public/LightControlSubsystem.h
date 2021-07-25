#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Subsystem/ModSubsystem.h"

#include "LightControlSubsystem.generated.h"

USTRUCT()
struct FControlledLight
{
    GENERATED_BODY()

    FControlledLight() : lightActor(nullptr), dimmer(0.0f), colorIdx(0) {}

    AFGBuildableLightSource* lightActor;

    float dimmer;

    int32 colorIdx;
};

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

    void Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint);

protected:
    FSocket* socket;
    FUdpSocketReceiver* receiver;

    TArray<FLinearColor> colors;
    TArray<FControlledLight> lights;
};
