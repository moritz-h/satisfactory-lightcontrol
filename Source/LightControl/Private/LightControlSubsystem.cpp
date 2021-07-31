#include "LightControlSubsystem.h"

#include "FGGameState.h"

#include "Configuration/ModConfiguration.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"

ALightControlSubsystem::ALightControlSubsystem()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    socket = nullptr;
    receiver = nullptr;
    const FLinearColor col(0.0f, 0.0f, 0.0f);
    colors.Init(col, 7);
}

void ALightControlSubsystem::BeginPlay()
{
    Super::BeginPlay();

    const FConfigId configId{"LightControl", ""};
    UConfigPropertySection* lightControlConfiguration = Cast<UConfigPropertySection>(URuntimeBlueprintFunctionLibrary::GetModConfigurationProperty(configId));
    UConfigPropertyArray* propertyArray = Cast<UConfigPropertyArray>(URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertySectionToConfigProperty(lightControlConfiguration, "LightActors"));
    TArray<UConfigProperty*> propertyArrayValues = URuntimeBlueprintFunctionLibrary::Conv_ConfigPropertyArrayToConfigPropertyArray(propertyArray);

    const int32 lightNum = propertyArrayValues.Num();

    TArray<FString> lightNames;
    lightNames.Init(TEXT(""), lightNum);

    for (int32 i = 0; i < lightNum; i++) {
        lightNames[i] = Cast<UConfigPropertyString>(propertyArrayValues[i])->Value;
    }

    UE_LOG(LogTemp, Warning, TEXT("#### ALightControlSubsystem: Init %i lights"), lightNum);
    const FControlledLight initLight;
    lights.Init(initLight, lightNum);

    TArray<AActor*> FoundLightSourceActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildableLightSource::StaticClass(), FoundLightSourceActors);

    for (AActor* TActor : FoundLightSourceActors) {
        AFGBuildableLightSource* lightSource = Cast<AFGBuildableLightSource>(TActor);
        if (lightSource != nullptr) {
            for (int32 i = 0; i < lightNum; i++) {
                if (lightSource->GetName() == lightNames[i]) {
                    lights[i].lightActor = lightSource;
                }
            }
        }
    }

    for (int i = 0; i < lightNum; i++) {
        UE_LOG(LogTemp, Warning, TEXT("#### ALightControlSubsystem: Light actor ptr: %u (name: %s)"), lights[i].lightActor, *lightNames[i]);
    }

    socket = FUdpSocketBuilder(TEXT("Art-Net"))
        .AsNonBlocking()
        .AsReusable()
        .BoundToAddress(FIPv4Address::Any)
        .BoundToPort(6454)
        .WithBroadcast()
        .WithReceiveBufferSize(2 * 1024 * 1024)
        .Build();

    FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
    receiver = new FUdpSocketReceiver(socket, ThreadWaitTime, TEXT("UDP RECEIVER"));
    receiver->OnDataReceived().BindUObject(this, &ALightControlSubsystem::Receive);
    receiver->Start();
}

void ALightControlSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    receiver->Stop();
    lights.Empty();
    UE_LOG(LogTemp, Warning, TEXT("#### ALightControlSubsystem::EndPlay"));
}

void ALightControlSubsystem::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Set colors
    auto gameState = GetWorld()->GetGameState<AFGGameState>();
    for (int i = 0; i < 7; i++) {
        gameState->Server_SetBuildableLightColorSlot(i, colors[i]);
    }

    // Set Actors
    for (FControlledLight& l : lights) {
        if (l.lightActor != nullptr) {
            const bool enabled = l.enabled_target;
            if (l.enabled_actual != enabled) {
                l.lightActor->SetLightEnabled(enabled);
                l.enabled_actual = enabled;
            }
            const int32 colorIdx = l.colorIdx_target;
            const int dimmer = l.dimmer_target;
            if (l.colorIdx_actual != colorIdx || l.dimmer_actual != dimmer) {
                FLightSourceControlData data;
                data.ColorSlotIndex = colorIdx;
                data.Intensity = dimmer;
                data.IsTimeOfDayAware = false;
                l.lightActor->SetLightControlData(data);
                l.colorIdx_actual = colorIdx;
                l.dimmer_actual = dimmer;
            }
        }
    }
}

void ALightControlSubsystem::Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint)
{
    const char* buffer = reinterpret_cast<const char*>(data->GetData());
    int32 size = data->Num();

    if (size < 10 || strncmp(&buffer[0], "Art-Net", 8) != 0) {
        return;
    }
    const int OpCode = (buffer[9] << 8) + buffer[8];

    if (OpCode != 0x5000 || size <= 18) {
        return;
    }

    const int8 ProtVerHi = buffer[10];
    const int8 ProtVerLo = buffer[11];
    const int8 Sequence = buffer[12];
    const int8 Physical = buffer[13];
    const int8 SubUni = buffer[14];
    const int8 Net = buffer[15];
    if (SubUni != 0 || Net != 0) {
        return;
    }

    const int DataLength = ((buffer[16] << 8) | buffer[17]);
    if (DataLength < 2 || DataLength > 512 || size < 18 + DataLength) {
        return;
    }

    const unsigned char* uBuffer = reinterpret_cast<const unsigned char*>(buffer);

    // 7 * 3 color channels
    for (int i = 0; i < 7; i++) {
        const float r = static_cast<float>(uBuffer[18 + 3 * i + 0]) / 255.0f;
        const float g = static_cast<float>(uBuffer[18 + 3 * i + 1]) / 255.0f;
        const float b = static_cast<float>(uBuffer[18 + 3 * i + 2]) / 255.0f;
        colors[i] = FLinearColor(r, g, b);
    }

    // 8 * 2 devices with dimmer and color idx
    for (int32 i = 0; i < lights.Num(); i++) {
        FControlledLight& l = lights[i];
        const unsigned char dmxDimmer = uBuffer[18 + 3 * 7 + 2 * i + 0];
        const unsigned char dmxColorIdx = uBuffer[18 + 3 * 7 + 2 * i + 1];

        l.enabled_target = dmxDimmer > 0;
        l.dimmer_target = static_cast<float>(dmxDimmer > 0 ? dmxDimmer - 1 : 0) / 254.0f;
        l.colorIdx_target = FMath::Clamp(static_cast<int32>(dmxColorIdx) / 36, 0, 6);
    }
}
