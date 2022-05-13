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
        Highlight(false) {}

    FLightSourceInfo(int32 universe, int32 channel, const FString& name) :
        Universe(universe),
        Channel(channel),
        Name(name),
        Highlight(false) {}

    UPROPERTY( SaveGame )
    int32 Universe;

    UPROPERTY( SaveGame )
    int32 Channel;

    UPROPERTY( SaveGame )
    FString Name;

    UPROPERTY( SaveGame )
    bool Highlight;
};

UENUM()
enum class ELightControlMode : uint8
{
    NONE,
    COLOR_IDX,
    RGB,
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
    ELightControlMode GetControlMode() {
        return ControlMode;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetControlMode(ELightControlMode mode) {
        ControlMode = mode;
    }

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
            lightData->Channel = FMath::Clamp(channel, 1, 512);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    FString GetLightName(AFGBuildableLightSource* light) {
        const auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            return lightData->Name;
        }
        return FString();
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetLightName(AFGBuildableLightSource* light, const FString& name) {
        auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            lightData->Name = name;
        }
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    bool GetLightHighlight(AFGBuildableLightSource* light) {
        const auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            return lightData->Highlight;
        }
        return false;
    }

    UFUNCTION(BlueprintCallable, Category = "LightControl|ArtNetLightsControlPanel")
    void SetLightHighlight(AFGBuildableLightSource* light, bool highlight) {
        auto* lightData = LightsMap.Find(light);
        if (lightData != nullptr) {
            lightData->Highlight = highlight;
        }
    }

protected:
    UPROPERTY( SaveGame )
    ELightControlMode ControlMode;

    UPROPERTY( SaveGame )
    int32 DefaultUniverse;

    UPROPERTY( SaveGame )
    int32 DefaultChannel;

    UPROPERTY( SaveGame )
    TMap<AFGBuildableLightSource*, FLightSourceInfo> LightsMap;

    ALightControlSubsystem* LightControlSubsystem;

    bool isFirstTick;

    float time;
};
