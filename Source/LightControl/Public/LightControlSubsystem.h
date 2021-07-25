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

    FControlledLight() : lightActor(nullptr), enabled_actual(false), enabled_target(true), dimmer_actual(-1.0f), dimmer_target(0.0f), colorIdx_actual(-1), colorIdx_target(0) {}

    AFGBuildableLightSource* lightActor;

    bool enabled_actual;
    bool enabled_target;

    int32 colorIdx_actual;
    int32 colorIdx_target;

    float dimmer_actual;
    float dimmer_target;
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
