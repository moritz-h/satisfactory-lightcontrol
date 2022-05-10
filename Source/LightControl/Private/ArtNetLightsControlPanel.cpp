#include "ArtNetLightsControlPanel.h"

#include "LogLightControl.h"
#include "Subsystem/SubsystemActorManager.h"

AArtNetLightsControlPanel::AArtNetLightsControlPanel() :
    DefaultUniverse(0),
    DefaultChannel(0),
    LightControlSubsystem(nullptr),
    isFirstTick(true),
    time(0.0f)
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
}

void AArtNetLightsControlPanel::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogLightControl, Warning, TEXT("AArtNetLightsControlPanel: BeginPlay."));
    USubsystemActorManager* SubsystemActorManager = GetWorld()->GetSubsystem<USubsystemActorManager>();
    if (SubsystemActorManager) {
        LightControlSubsystem = SubsystemActorManager->GetSubsystemActor<ALightControlSubsystem>();
    } else {
        UE_LOG(LogLightControl, Warning, TEXT("AArtNetLightsControlPanel: SubsystemActorManager missing."));
    }
}

void AArtNetLightsControlPanel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // GetControlledBuildables is empty when called from BeginPlay.
    if (isFirstTick) {
        isFirstTick = false;
        mOnControlledBuildablesChanged.AddDynamic(this, &AArtNetLightsControlPanel::UpdateLights);
        UpdateLights();
    }

    time += DeltaSeconds;
    bool blink = static_cast<int32>(time * 2.0f) % 2 == 0;

    const int32 NumColors = LightControlSubsystem->GetNumColors();
    const int32 ColorsDmxRange = 255 / NumColors;

    for (auto& Elem : LightsMap) {
        auto* lightActor = Elem.Key;
        auto& info = Elem.Value;

        bool enabled = blink;
        float dimmer = 1.0f;
        int32 colorIdx = 0;

        if (!info.Highlight) {
            const uint8 dmxDimmer = LightControlSubsystem->GetDmxValue(info.Universe, info.Channel);
            const uint8 dmxColorIdx = LightControlSubsystem->GetDmxValue(info.Universe, info.Channel + 1);
            enabled = dmxDimmer > 0;
            dimmer = static_cast<float>(dmxDimmer > 0 ? dmxDimmer - 1 : 0) / 254.0f;
            colorIdx = FMath::Clamp(static_cast<int32>(dmxColorIdx) / ColorsDmxRange, 0, NumColors - 1);
        }

        if (lightActor->IsLightEnabled() != enabled) {
            lightActor->SetLightEnabled(enabled);
        }
        const auto& currentData = lightActor->GetLightControlData();
        if (colorIdx != currentData.ColorSlotIndex || dimmer != currentData.Intensity || currentData.IsTimeOfDayAware) {
            FLightSourceControlData data;
            data.ColorSlotIndex = colorIdx;
            data.Intensity = dimmer;
            data.IsTimeOfDayAware = false;
            lightActor->SetLightControlData(data);
        }
    }
}

void AArtNetLightsControlPanel::UpdateLights()
{
    TArray<AFGBuildable*>& connectedLights = GetControlledBuildables(AFGBuildableLightSource::StaticClass());

    UE_LOG(LogLightControl, Warning, TEXT("AArtNetLightsControlPanel: Update with %i connected lights."), connectedLights.Num());

    TMap<AFGBuildableLightSource*, FLightSourceInfo> OldLightsMap;
    Swap(LightsMap, OldLightsMap);

    for (auto& buildable : connectedLights) {
        AFGBuildableLightSource* lightSource = Cast<AFGBuildableLightSource>(buildable);

        const auto* oldInfo = OldLightsMap.Find(lightSource);
        if (oldInfo != nullptr) {
            LightsMap.Add(lightSource, *oldInfo);
            OldLightsMap.Remove(lightSource);
        } else {
            LightsMap.Add(lightSource, FLightSourceInfo(DefaultUniverse, DefaultChannel, lightSource->GetName()));
        }
    }

    // It may is kind of unexpected that the ArtNetLightsControlPanel could disable a light completely, especially
    // because a regular Lights Control Panel cannot (re)enable it remotely. Therefore, enable it on disconnect.
    // UpdateLights() is also called on dismantling the control panel itself.
    for (auto& lightSource : OldLightsMap) {
        lightSource.Key->SetLightEnabled(true);
    }
}

TArray<AFGBuildableLightSource*> AArtNetLightsControlPanel::GetLights() const
{
    TArray<AFGBuildableLightSource*> lights;
    LightsMap.GetKeys(lights);

    // Sort by name
    lights.Sort([this](const AFGBuildableLightSource& A, const AFGBuildableLightSource& B) {
        return LightsMap[&A].Name < LightsMap[&B].Name;
    });

    return lights;
}
