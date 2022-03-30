#include "LightControlSubsystem.h"

#include "Configuration/ModConfiguration.h"
#include "FGGameState.h"
#include "FGWorldSettings.h"
#include "LogLightControl.h"
#include "Util/RuntimeBlueprintFunctionLibrary.h"

ALightControlSubsystem::ALightControlSubsystem() :
    ArtNet_Net(0),
    ArtNet_SubNet(0),
    ColorUniverse(0),
    ColorChannel(1),
    colorsNeedUpdate(false)
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    socket = nullptr;
    receiver = nullptr;

    DmxData.Init(0, 16 * 512);
}

void ALightControlSubsystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogLightControl, Warning, TEXT("LightControlSubsystem::BeginPlay: Start"));

    auto* buildableSubsystem = Cast<AFGWorldSettings>(GetWorld()->GetWorldSettings())->GetBuildableSubsystem();
    colors = buildableSubsystem->GetBuildableLightColorSlots();
    UE_LOG(LogLightControl, Warning, TEXT("LightControlSubsystem: Init with %i colors."), buildableSubsystem->GetNumBuildableLightColorSlots());

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
    UE_LOG(LogLightControl, Warning, TEXT("LightControlSubsystem::EndPlay: Done"));
}

void ALightControlSubsystem::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Set colors
    if (colorsNeedUpdate.AtomicSet(false)) {
        auto* gameState = GetWorld()->GetGameState<AFGGameState>();
        for (int32 i = 0; i < colors.Num(); i++) {
            gameState->Server_SetBuildableLightColorSlot(i, colors[i]);
        }
    }
}

void ALightControlSubsystem::Receive(const FArrayReaderPtr& data, const FIPv4Endpoint& Endpoint)
{
    const int32 size = data->Num();
    const uint8* buffer = data->GetData();

    // 18 bytes ArtDmx packet header
    if (size < 18) {
        return;
    }

    if (strncmp(reinterpret_cast<const char*>(buffer), "Art-Net", 8) != 0) {
        return;
    }

    const int16 OpCode = (buffer[9] << 8) + buffer[8];
    if (OpCode != 0x5000) {
        return;
    }

    // const int8 ProtVerHi = buffer[10];
    // const int8 ProtVerLo = buffer[11];
    // const int8 Sequence = buffer[12];
    // const int8 Physical = buffer[13];
    const int8 Universe = buffer[14] & 0xF;
    const int8 SubNet = (buffer[14] >> 4) & 0xF;
    const int8 Net = buffer[15] & 0x7F;

    if (Net != ArtNet_Net || SubNet != ArtNet_SubNet || Universe < 0 || Universe >= 16) {
        return;
    }

    const int16 DataLength = ((buffer[16] << 8) | buffer[17]);
    if (DataLength < 2 || DataLength > 512 || size < 18 + DataLength) {
        return;
    }

    // Copy to DmxData storage
    FMemory::Memcpy(DmxData.GetData() + static_cast<int32>(Universe) * 512, buffer + 18, DataLength);

    // Update light colors
    if (Universe == ColorUniverse) {
        UpdateColors();
    }
}

void ALightControlSubsystem::UpdateColors()
{
    // 3 color channels per light
    for (int32 i = 0; i < colors.Num(); i++) {
        const float r = static_cast<float>(GetDmxValue(ColorUniverse, ColorChannel + 3 * i + 0)) / 255.0f;
        const float g = static_cast<float>(GetDmxValue(ColorUniverse, ColorChannel + 3 * i + 1)) / 255.0f;
        const float b = static_cast<float>(GetDmxValue(ColorUniverse, ColorChannel + 3 * i + 2)) / 255.0f;
        colors[i] = FLinearColor(r, g, b);
    }
    colorsNeedUpdate = true;
}
