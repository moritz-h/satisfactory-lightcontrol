#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableControlPanelHost.h"
#include "Buildables/FGBuildableLightSource.h"
#include "LightControlSubsystem.h"

#include "ArtNetLightsControlPanel.generated.h"

USTRUCT()
struct FLightSourceInfo
{
    GENERATED_BODY()

    FLightSourceInfo() :
        Universe(0),
        Channel(1),
        enabled_actual(false),
        enabled_target(true),
        dimmer_actual(-1.0f),
        dimmer_target(0.0f),
        colorIdx_actual(-1),
        colorIdx_target(0) {}

    FLightSourceInfo(int32 universe, int32 channel) :
        Universe(universe),
        Channel(channel),
        enabled_actual(false),
        enabled_target(true),
        dimmer_actual(-1.0f),
        dimmer_target(0.0f),
        colorIdx_actual(-1),
        colorIdx_target(0) {}

    UPROPERTY( SaveGame )
    int32 Universe;

    UPROPERTY( SaveGame )
    int32 Channel;

    bool enabled_actual;
    bool enabled_target;

    int32 colorIdx_actual;
    int32 colorIdx_target;

    float dimmer_actual;
    float dimmer_target;
};

UCLASS()
class LIGHTCONTROL_API AArtNetLightsControlPanel : public AFGBuildableControlPanelHost
{
    GENERATED_BODY()
public:
    AArtNetLightsControlPanel();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION()
    void UpdateLights();

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    int32 GetDefaultUniverse() {
        return DefaultUniverse;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetDefaultUniverse(int32 universe) {
        DefaultUniverse = FMath::Clamp(universe, 0, 16);
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    int32 GetDefaultChannel() {
        return DefaultChannel;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetDefaultChannel(int32 channel) {
        DefaultChannel = FMath::Clamp(channel, 1, 512);
    }

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "LightControl|ArtNetLightsControlPanel")
    TArray<AFGBuildableLightSource*> GetLights() const;

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    int32 GetLightUniverse(AFGBuildableLightSource* light) {
        const auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            return lightData->Universe;
        }
        return DefaultUniverse;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetLightUniverse(AFGBuildableLightSource* light, int32 universe) {
        auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            lightData->Universe = FMath::Clamp(universe, 0, 16);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    int32 GetLightChannel(AFGBuildableLightSource* light) {
        const auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            return lightData->Channel;
        }
        return DefaultChannel;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetLightChannel(AFGBuildableLightSource* light, int32 channel) {
        auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            lightData->Channel = FMath::Clamp(channel, 0, 255);
        }
    }

protected:
    UPROPERTY( SaveGame )
    int32 DefaultUniverse;

    UPROPERTY( SaveGame )
    int32 DefaultChannel;

    UPROPERTY( SaveGame )
    TMap<AFGBuildableLightSource*, FLightSourceInfo> LightsMap;

    ALightControlSubsystem* LightControlSubsystem;

    bool isFirstTick;
};
