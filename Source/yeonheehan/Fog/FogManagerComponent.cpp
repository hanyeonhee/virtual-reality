// Copyright yeonheehan. All Rights Reserved.

#include "Fog/FogManagerComponent.h"
#include "Fog/ExponentialHeightFogActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UFogManagerComponent::UFogManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // ── Default presets ──────────────────────────────────────────────────────

    // Clear day
    FFogPreset& Clear = Presets.AddDefaulted_GetRef();
    Clear.PresetName               = FName("ClearDay");
    Clear.FogDensity               = 0.005f;
    Clear.FogHeightFalloff         = 0.15f;
    Clear.FogMaxOpacity            = 0.6f;
    Clear.bVolumetricFog           = true;
    Clear.VolumetricFogExtinctionScale = 0.3f;
    Clear.VolumetricFogDistance    = 8000.0f;
    Clear.TransitionDuration       = 3.0f;

    // Morning mist
    FFogPreset& Mist = Presets.AddDefaulted_GetRef();
    Mist.PresetName                = FName("MorningMist");
    Mist.FogDensity                = 0.04f;
    Mist.FogHeightFalloff          = 0.08f;
    Mist.FogHeightOffset           = -50.0f;
    Mist.FogMaxOpacity             = 0.9f;
    Mist.bVolumetricFog            = true;
    Mist.VolumetricFogExtinctionScale = 1.5f;
    Mist.VolumetricFogDistance     = 3000.0f;
    Mist.TransitionDuration        = 4.0f;

    // Dense night fog
    FFogPreset& Night = Presets.AddDefaulted_GetRef();
    Night.PresetName               = FName("DenseNight");
    Night.FogDensity               = 0.08f;
    Night.FogHeightFalloff         = 0.2f;
    Night.FogMaxOpacity            = 1.0f;
    Night.bVolumetricFog           = true;
    Night.VolumetricFogExtinctionScale = 3.0f;
    Night.VolumetricFogDistance    = 1500.0f;
    Night.TransitionDuration       = 6.0f;

    // High-altitude haze
    FFogPreset& Haze = Presets.AddDefaulted_GetRef();
    Haze.PresetName                = FName("HighAltitudeHaze");
    Haze.FogDensity                = 0.01f;
    Haze.FogHeightFalloff          = 0.05f;
    Haze.FogHeightOffset           = 200.0f;
    Haze.FogMaxOpacity             = 0.5f;
    Haze.bVolumetricFog            = false;
    Haze.TransitionDuration        = 2.0f;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void UFogManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!FogActorReference && bAutoFindFogActor)
    {
        AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), AExponentialHeightFogActor::StaticClass());
        FogActorReference = Cast<AExponentialHeightFogActor>(Found);

        if (!FogActorReference)
        {
            UE_LOG(LogTemp, Warning,
                   TEXT("UFogManagerComponent: No AExponentialHeightFogActor found in level. "
                        "Drop one or set FogActorReference manually."));
        }
    }
}

void UFogManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bTransitioning) return;

    TransitionElapsed += DeltaTime;
    const float RawAlpha   = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);
    // Smooth-step
    const float Alpha = RawAlpha * RawAlpha * (3.0f - 2.0f * RawAlpha);

    BlendPresets(TransitionFrom, TransitionTo, Alpha);

    if (RawAlpha >= 1.0f)
    {
        bTransitioning = false;
        ApplyPresetDirect(TransitionTo);
        OnPresetApplied.Broadcast(TransitionTargetName);
    }
}

// ─── Private helpers ──────────────────────────────────────────────────────────

AExponentialHeightFogActor* UFogManagerComponent::GetFogActor() const
{
    return FogActorReference;
}

void UFogManagerComponent::ApplyPresetDirect(const FFogPreset& Preset)
{
    AExponentialHeightFogActor* Fog = GetFogActor();
    if (!Fog) return;

    Fog->SetFogDensity(Preset.FogDensity);
    Fog->SetFogHeightOffset(Preset.FogHeightOffset);
    Fog->SetFogHeightFalloff(Preset.FogHeightFalloff);
    Fog->SetVolumetricFogEnabled(Preset.bVolumetricFog);
    Fog->SetVolumetricFogExtinctionScale(Preset.VolumetricFogExtinctionScale);
    Fog->SetVolumetricFogDistance(Preset.VolumetricFogDistance);
}

// ─── Public API ───────────────────────────────────────────────────────────────

bool UFogManagerComponent::ApplyPreset(FName PresetName)
{
    for (const FFogPreset& P : Presets)
    {
        if (P.PresetName == PresetName)
        {
            ApplyPresetDirect(P);
            OnPresetApplied.Broadcast(PresetName);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UFogManagerComponent::ApplyPreset — preset '%s' not found."),
           *PresetName.ToString());
    return false;
}

bool UFogManagerComponent::TransitionToPreset(FName PresetName, float DurationSeconds)
{
    for (const FFogPreset& P : Presets)
    {
        if (P.PresetName == PresetName)
        {
            TransitionFrom        = CaptureCurrent();
            TransitionTo          = P;
            TransitionDuration    = (DurationSeconds < 0.0f) ? P.TransitionDuration : DurationSeconds;
            TransitionElapsed     = 0.0f;
            TransitionTargetName  = PresetName;
            bTransitioning        = true;

            if (TransitionDuration <= 0.0f)
            {
                // Snap immediately
                ApplyPresetDirect(P);
                bTransitioning = false;
                OnPresetApplied.Broadcast(PresetName);
            }
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UFogManagerComponent::TransitionToPreset — preset '%s' not found."),
           *PresetName.ToString());
    return false;
}

void UFogManagerComponent::BlendPresets(const FFogPreset& A, const FFogPreset& B, float Alpha)
{
    AExponentialHeightFogActor* Fog = GetFogActor();
    if (!Fog) return;

    Fog->SetFogDensity(          FMath::Lerp(A.FogDensity,       B.FogDensity,       Alpha));
    Fog->SetFogHeightOffset(     FMath::Lerp(A.FogHeightOffset,  B.FogHeightOffset,  Alpha));
    Fog->SetFogHeightFalloff(    FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha));
    Fog->SetVolumetricFogExtinctionScale(
        FMath::Lerp(A.VolumetricFogExtinctionScale, B.VolumetricFogExtinctionScale, Alpha));
    Fog->SetVolumetricFogDistance(
        FMath::Lerp(A.VolumetricFogDistance, B.VolumetricFogDistance, Alpha));

    // Enable volumetric fog if either endpoint has it on
    Fog->SetVolumetricFogEnabled(A.bVolumetricFog || B.bVolumetricFog);
}

FFogPreset UFogManagerComponent::CaptureCurrent() const
{
    FFogPreset Current;
    Current.PresetName = FName("__Current__");

    const AExponentialHeightFogActor* Fog = GetFogActor();
    if (!Fog) return Current;

    Current.FogDensity                = Fog->FogDensity;
    Current.FogHeightOffset           = Fog->FogHeightOffset;
    Current.FogHeightFalloff          = Fog->FogHeightFalloff;
    Current.FogMaxOpacity             = Fog->FogMaxOpacity;
    Current.bVolumetricFog            = Fog->bVolumetricFog;
    Current.VolumetricFogExtinctionScale = Fog->VolumetricFogExtinctionScale;
    Current.VolumetricFogDistance     = Fog->VolumetricFogDistance;

    return Current;
}

void UFogManagerComponent::CancelTransition()
{
    bTransitioning = false;
}
