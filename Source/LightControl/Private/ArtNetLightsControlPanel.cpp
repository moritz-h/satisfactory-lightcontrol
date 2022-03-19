#include "ArtNetLightsControlPanel.h"

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

    UE_LOG(LogTemp, Warning, TEXT("AArtNetLightsControlPanel: BeginPlay."));
    USubsystemActorManager* SubsystemActorManager = GetWorld()->GetSubsystem<USubsystemActorManager>();
    if (SubsystemActorManager) {
        LightControlSubsystem = SubsystemActorManager->GetSubsystemActor<ALightControlSubsystem>();
    } else {
        UE_LOG(LogTemp, Warning, TEXT("AArtNetLightsControlPanel: SubsystemActorManager missing."));
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
    bool blink = static_cast<int>(time * 2.0f) % 2 == 0;

    const int32 NumColors = LightControlSubsystem->GetNumColors();
    const int32 ColorsDmxRange = 255 / NumColors;

    for (auto& Elem : LightsMap) {
        auto* lightActor = Elem.Key;
        auto& info = Elem.Value;

        if (!info.Highlight) {
            const uint8 dmxDimmer = LightControlSubsystem->GetDmxValue(info.Universe, info.Channel);
            const uint8 dmxColorIdx = LightControlSubsystem->GetDmxValue(info.Universe, info.Channel + 1);
            info.enabled_target = dmxDimmer > 0;
            info.dimmer_target = static_cast<float>(dmxDimmer > 0 ? dmxDimmer - 1 : 0) / 254.0f;
            info.colorIdx_target = FMath::Clamp(static_cast<int32>(dmxColorIdx) / ColorsDmxRange, 0, NumColors - 1);
        } else {
            if (blink) {
                info.enabled_target = true;
                info.dimmer_target = 1.0f;
                info.colorIdx_target = 0;
            } else {
                info.enabled_target = false;
            }
        }

        const bool enabled = info.enabled_target;
        if (/* TODO force update every frame? */ true || info.enabled_actual != enabled) {
            lightActor->SetLightEnabled(enabled);
            info.enabled_actual = enabled;
        }
        const int32 colorIdx = info.colorIdx_target;
        const float dimmer = info.dimmer_target;
        if (/* TODO force update every frame? */ true || info.colorIdx_actual != colorIdx || info.dimmer_actual != dimmer) {
            FLightSourceControlData data;
            data.ColorSlotIndex = colorIdx;
            data.Intensity = dimmer;
            data.IsTimeOfDayAware = false;
            lightActor->SetLightControlData(data);
            info.colorIdx_actual = colorIdx;
            info.dimmer_actual = dimmer;
        }
    }
}

void AArtNetLightsControlPanel::UpdateLights()
{
    TArray<AFGBuildable*>& connectedLights = GetControlledBuildables(AFGBuildableLightSource::StaticClass());

    UE_LOG(LogTemp, Warning, TEXT("AArtNetLightsControlPanel: Update with %i connected lights."), connectedLights.Num());

    TMap<AFGBuildableLightSource*, FLightSourceInfo> OldLightsMap;
    Swap(LightsMap, OldLightsMap);

    for (auto& buildable : connectedLights) {
        AFGBuildableLightSource* lightSource = Cast<AFGBuildableLightSource>(buildable);

        const auto* oldInfo = OldLightsMap.Find(lightSource);
        if (oldInfo != nullptr) {
            LightsMap.Add(lightSource, *oldInfo);
        } else {
            LightsMap.Add(lightSource, FLightSourceInfo(DefaultUniverse, DefaultChannel, lightSource->GetName()));
        }
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
