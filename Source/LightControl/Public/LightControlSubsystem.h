#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Buildables/FGBuildableLightSource.h"
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

    void Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint);

protected:
    FSocket* socket;
    FUdpSocketReceiver* receiver;

    FLinearColor colors[7];

    AFGBuildableLightSource* lightActors[8];
    float dimmers[8];
    int colorIdxs[8];
};
