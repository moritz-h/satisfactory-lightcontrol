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
class LIGHTCONTROL_API ALightControlSubsystem : public AModSubsystem, public IFGSaveInterface
{
    GENERATED_BODY()

public:
    ALightControlSubsystem();

    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void Tick(float DeltaSeconds) override;

    virtual bool ShouldSave_Implementation() const override { return true; }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetNet() const {
        return ArtNet_Net;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetNet(int32 net) {
        ArtNet_Net = FMath::Clamp(net, 0, 16);
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetSubNet() const {
        return ArtNet_SubNet;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetSubNet(int32 subNet) {
        ArtNet_SubNet = FMath::Clamp(subNet, 0, 16);
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetColorUniverse() const {
        return ColorUniverse;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetColorUniverse(int32 universe) {
        ColorUniverse = FMath::Clamp(universe, 0, 16);
        UpdateColors();
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetColorChannel() const {
        return ColorChannel;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetColorChannel(int32 channel) {
        ColorChannel = FMath::Clamp(channel, 1, 512);
        UpdateColors();
    }

    FORCEINLINE uint8 GetDmxValue(int32 universe, int32 channel) const {
        if (universe >= 0 && universe < 16 && channel >= 1 && channel <= 512) {
            return DmxData[512 * universe + channel - 1];
        }
        return 0;
    }

    void Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint);

protected:
    void UpdateColors();

    UPROPERTY( SaveGame )
    int32 ArtNet_Net;

    UPROPERTY( SaveGame )
    int32 ArtNet_SubNet;

    UPROPERTY( SaveGame )
    int32 ColorUniverse;

    UPROPERTY( SaveGame )
    int32 ColorChannel;

    FSocket* socket;
    FUdpSocketReceiver* receiver;

    TArray<uint8> DmxData;

    TArray<FLinearColor> colors;
};
