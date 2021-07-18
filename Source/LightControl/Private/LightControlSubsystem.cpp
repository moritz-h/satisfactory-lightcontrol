#include "LightControlSubsystem.h"

#include <string>

#include "FGGameState.h"

ALightControlSubsystem::ALightControlSubsystem()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    socket = nullptr;
    receiver = nullptr;
    for (int i = 0; i < 7; i++) {
        colors[i] = FLinearColor(0.0f, 0.0f, 0.0f);
    }
    for (int i = 0; i < 8; i++) {
        lightActors[i] = nullptr;
        dimmers[i] = 1.0f;
        colorIdxs[i] = 0;
    }
}

void ALightControlSubsystem::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("########## ALightControlSubsystem::BeginPlay"));

    std::vector <std::string> lightNames{
        "Build_FloodlightPole_C_2147397259",
        "Build_FloodlightPole_C_2147397502",
        "Build_FloodlightWall_C_2147393450",
        "Build_FloodlightWall_C_2147393322",
        "Build_StreetLight_C_2147390838",
        "Build_StreetLight_C_2147389914",
        "Build_StreetLight_C_2147390022",
        "Build_StreetLight_C_2147391627",
    };

    TArray<AActor*> FoundLightSourceActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildableLightSource::StaticClass(), FoundLightSourceActors);

    for (AActor* TActor : FoundLightSourceActors) {
        AFGBuildableLightSource* lightSource = Cast<AFGBuildableLightSource>(TActor);
        if (lightSource != nullptr) {
            std::string name = std::string(TCHAR_TO_UTF8(*lightSource->GetName()));
            for (int i = 0; i < 8; i++) {
                if (name == lightNames[i]) {
                    lightActors[i] = lightSource;
                }
            }
        }
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
    UE_LOG(LogTemp, Warning, TEXT("########## ALightControlSubsystem::EndPlay"));
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
    for (int i = 0; i < 8; i++) {
        if (lightActors[i] != nullptr) {
            FLightSourceControlData data;
            data.ColorSlotIndex = colorIdxs[i];
            data.Intensity = dimmers[i];
            data.IsTimeOfDayAware = false;
            lightActors[i]->SetLightControlData(data);
            if (dimmers[i] == 0.0f) {
                lightActors[i]->SetLightEnabled(false);
            } else {
                lightActors[i]->SetLightEnabled(true);
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
    int OpCode = (buffer[9] << 8) + buffer[8];

    if (OpCode != 0x5000 || size <= 18) {
        return;
    }

    int8 ProtVerHi = buffer[10];
    int8 ProtVerLo = buffer[11];
    int8 Sequence = buffer[12];
    int8 Physical = buffer[13];
    int8 SubUni = buffer[14];
    int8 Net = buffer[15];
    if (SubUni != 0 || Net != 0) {
        return;
    }

    int DataLength = ((buffer[16] << 8) | buffer[17]);
    if (DataLength < 2 || DataLength > 512 || size < 18 + DataLength ) {
        return;
    }

    const unsigned char* uBuffer = reinterpret_cast<const unsigned char*>(buffer);

    // 7 * 3 color channels
    for (int i = 0; i < 7; i++) {
        float r = static_cast<float>(uBuffer[18 + 3 * i + 0]) / 255.0f;
        float g = static_cast<float>(uBuffer[18 + 3 * i + 1]) / 255.0f;
        float b = static_cast<float>(uBuffer[18 + 3 * i + 2]) / 255.0f;
        colors[i] = FLinearColor(r, g, b);
    }

    // 8 * 2 devices with dimmer and color idx
    for (int i = 0; i < 8; i++) {
        dimmers[i] = static_cast<float>(uBuffer[18 + 3 * 7 + 2 * i + 0]) / 255.0f;
        colorIdxs[i] = static_cast<int>(uBuffer[18 + 3 * 7 + 2 * i + 1]) / 36;
        if (colorIdxs[i] < 0) {
            colorIdxs[i] = 0;
        }
        if (colorIdxs[i] > 6) {
            colorIdxs[i] = 6;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("########## Network data: %i"), DataLength);
}
