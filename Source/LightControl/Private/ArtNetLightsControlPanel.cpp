#include "ArtNetLightsControlPanel.h"

#include "LogLightControl.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGPowerInfoComponent.h"

AArtNetLightsControlPanel::AArtNetLightsControlPanel() :
    ControlMode(ELightControlMode::COLOR_IDX),
    Net(0),
    SubNet(0),
    DefaultUniverse(0),
    DefaultChannel(1),
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
        UE_LOG(LogLightControl, Error, TEXT("AArtNetLightsControlPanel: SubsystemActorManager missing."));
    }
}

void AArtNetLightsControlPanel::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (LightControlSubsystem == nullptr) {
        return;
    }

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

        const uint8 dmxDimmer = LightControlSubsystem->GetDmxValue(Net, SubNet, info.Universe, info.Channel);
        const float dimmer = info.Highlight ? 1.0f : static_cast<float>(dmxDimmer > 0 ? dmxDimmer - 1 : 0) / 254.0f;
        bool enabled = info.Highlight ? blink : dmxDimmer > 0;
        if (lightActor->IsLightEnabled() != enabled) {
            lightActor->SetLightEnabled(enabled);
        }

        const auto& lightData = lightActor->GetLightControlData();
        if (ControlMode == ELightControlMode::COLOR_IDX && !info.Highlight) {
            const uint8 dmxColorIdx = LightControlSubsystem->GetDmxValue(Net, SubNet, info.Universe, info.Channel + 1);
            const int32 colorIdx = FMath::Clamp(static_cast<int32>(dmxColorIdx) / ColorsDmxRange, 0, NumColors - 1);
            if (lightData.ColorSlotIndex != colorIdx || lightData.Intensity != dimmer || lightData.IsTimeOfDayAware || info.UsedPrivateInterface) {
                // SetLightControlData uses internal change detection but if we used the private interface
                // these changes are not noticed. Therefore, send dummy data to force a value change.
                if (info.UsedPrivateInterface) {
                    const FLightSourceControlData dummyData{colorIdx, dimmer < 0.5f ? 1.0f : 0.0f, false};
                    lightActor->SetLightControlData(dummyData);
                    info.UsedPrivateInterface = false;
                }
                const FLightSourceControlData data{colorIdx, dimmer, false};
                lightActor->SetLightControlData(data);
            }
        } else {
            // Disable IsTimeOfDayAware using the SetLightControlData interface. Many changes are done internally.
            if (lightData.IsTimeOfDayAware) {
                const FLightSourceControlData data{0, 0.0f, false};
                lightActor->SetLightControlData(data);
            }
            FLinearColor color(1.0f, 1.0f, 1.0f);
            if (!info.Highlight) {
                const uint8 dmxR = LightControlSubsystem->GetDmxValue(Net, SubNet, info.Universe, info.Channel + 1);
                const uint8 dmxG = LightControlSubsystem->GetDmxValue(Net, SubNet, info.Universe, info.Channel + 2);
                const uint8 dmxB = LightControlSubsystem->GetDmxValue(Net, SubNet, info.Universe, info.Channel + 3);
                color.R = static_cast<float>(dmxR) / 255.0f;
                color.G = static_cast<float>(dmxG) / 255.0f;
                color.B = static_cast<float>(dmxB) / 255.0f;
            }
            if (lightActor->mLightControlData.Intensity != dimmer || lightActor->mCurrentLightColor != color) {
                lightActor->mLightControlData.Intensity = dimmer;
                lightActor->mPowerInfo->SetTargetConsumption(dimmer);
                lightActor->mCurrentLightColor = color;
                lightActor->UpdateMeshDataAndHandles();
            }
            if (!info.UsedPrivateInterface) {
                info.UsedPrivateInterface = true;
            }
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

void AArtNetLightsControlPanel::AutoAddressLights()
{
    TArray<AFGBuildableLightSource*> lights;
    LightsMap.GetKeys(lights);

    lights.Sort([this](const AFGBuildableLightSource& A, const AFGBuildableLightSource& B) {
        // Round to 10 cm precision.
        const auto ALoc = A.GetActorLocation() / 10.0f;
        const auto BLoc = B.GetActorLocation() / 10.0f;
        FIntVector ALocInt(FMath::RoundToInt(ALoc.X), FMath::RoundToInt(ALoc.Y), FMath::RoundToInt(ALoc.Z));
        FIntVector BLocInt(FMath::RoundToInt(BLoc.X), FMath::RoundToInt(BLoc.Y), FMath::RoundToInt(BLoc.Z));
        if (ALocInt.Z == BLocInt.Z) {
            if (ALocInt.Y == BLocInt.Y) {
                return ALocInt.X < BLocInt.X;
            }
            return ALocInt.Y < BLocInt.Y;
        }
        return ALocInt.Z < BLocInt.Z;
    });

    const int32 dmxOffset = (ControlMode == ELightControlMode::RGB) ? 4 : 2;
    int32 universe = DefaultUniverse;
    int32 channel = DefaultChannel;

    for (int32 i = 0; i < lights.Num(); i++) {
        auto* lightData = LightsMap.Find(lights[i]);
        if (lightData != nullptr) {
            lightData->Name = FString::Printf(TEXT("Light_%05i"), i + 1);
            lightData->Universe = universe;
            lightData->Channel = channel;

            if (universe < 15 || (universe == 15 && channel <= 512 - 2 * dmxOffset + 1)) {
                channel += dmxOffset;
                if (channel > 512 - dmxOffset + 1) {
                    channel = 1;
                    universe++;
                }
            }
        }
    }
}
