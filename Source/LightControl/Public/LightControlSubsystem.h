#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "FGSaveInterface.h"
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
    int32 GetColorNet() const {
        return ColorNet;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetColorNet(int32 net) {
        ColorNet = FMath::Clamp(net, 0, 127);
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetColorSubNet() const {
        return ColorSubNet;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetColorSubNet(int32 subNet) {
        ColorSubNet = FMath::Clamp(subNet, 0, 15);
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    int32 GetColorUniverse() const {
        return ColorUniverse;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetColorUniverse(int32 universe) {
        ColorUniverse = FMath::Clamp(universe, 0, 15);
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

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    bool GetUseColors() const {
        return UseColors;
    }

    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "LightControl|LightControlSubsystem" )
    void SetUseColors(bool useColors) {
        UseColors = useColors;
    }

    FORCEINLINE uint8 GetDmxValue(int32 net, int32 subnet, int32 universe, int32 channel) const {
        if (net >= 0 && net < 128 && subnet >= 0 && subnet < 16 && universe >= 0 && universe < 16 && channel >= 1 && channel <= 512) {
            return DmxData[512 * (16 * (16 * net + subnet) + universe) + channel - 1];
        }
        return 0;
    }

    FORCEINLINE FLinearColor GetDmxColorRGB(int32 net, int32 subnet, int32 universe, int32 channel) const {
        const uint8 dmxR = GetDmxValue(net, subnet, universe, channel + 0);
        const uint8 dmxG = GetDmxValue(net, subnet, universe, channel + 1);
        const uint8 dmxB = GetDmxValue(net, subnet, universe, channel + 2);
        return FLinearColor(static_cast<float>(dmxR) / 255.0f, static_cast<float>(dmxG) / 255.0f, static_cast<float>(dmxB) / 255.0f);
    }

    FORCEINLINE int32 GetNumColors() const {
        return colors.Num();
    }

    void Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint);

protected:
    void UpdateColors();

    UPROPERTY( SaveGame )
    int32 ColorNet;

    UPROPERTY( SaveGame )
    int32 ColorSubNet;

    UPROPERTY( SaveGame )
    int32 ColorUniverse;

    UPROPERTY( SaveGame )
    int32 ColorChannel;

    UPROPERTY( SaveGame )
    bool UseColors;

    FSocket* socket;
    FUdpSocketReceiver* receiver;

    TArray<uint8> DmxData;

    TArray<FLinearColor> colors;
    FThreadSafeBool colorsNeedUpdate;
};
