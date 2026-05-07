// Copyright yeonheehan. All Rights Reserved.

#include "Fog/ExponentialHeightFogActor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

AExponentialHeightFogActor::AExponentialHeightFogActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFogComponent"));
    SetRootComponent(FogComponent);

    // Apply defaults immediately so the actor looks correct when dropped into the level
    ApplyAllSettings();
}

void AExponentialHeightFogActor::BeginPlay()
{
    Super::BeginPlay();
    ApplyAllSettings();
}

void AExponentialHeightFogActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Density animation ───────────────────────────────────────────────────
    if (bAnimatingDensity)
    {
        AnimElapsed += DeltaTime;
        const float Alpha = FMath::Clamp(AnimElapsed / AnimDuration, 0.0f, 1.0f);
        // Smooth-step easing
        const float EasedAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
        FogDensity = FMath::Lerp(AnimStartDensity, AnimTargetDensity, EasedAlpha);
        FogComponent->SetFogDensity(FogDensity);

        if (Alpha >= 1.0f)
        {
            bAnimatingDensity = false;
        }
    }
}

// ─── Private helper ───────────────────────────────────────────────────────────

void AExponentialHeightFogActor::ApplyAllSettings()
{
    if (!FogComponent) return;

    // ── Exponential Height Fog ─────────────────────────────────────────────
    FogComponent->SetFogDensity(FogDensity);
    FogComponent->SetFogHeightFalloff(FogHeightFalloff);
    FogComponent->SetFogMaxOpacity(FogMaxOpacity);
    FogComponent->SetStartDistance(StartDistance);
    FogComponent->SetFogCutoffDistance(FogCutoffDistance);

    // Height offset: move the actor along Z to shift fog base
    FVector ActorLocation = GetActorLocation();
    ActorLocation.Z = FogHeightOffset;
    SetActorLocation(ActorLocation);

    // ── Directional Inscattering ───────────────────────────────────────────
    FogComponent->SetInscatteringColorCubemapAngle(0.0f);
    FogComponent->DirectionalInscatteringColor      = DirectionalInscatteringColor;
    FogComponent->DirectionalInscatteringExponent   = DirectionalInscatteringExponent;
    FogComponent->DirectionalInscatteringStartDistance = DirectionalInscatteringStartDistance;
    FogComponent->MarkRenderStateDirty();

    // ── Second Fog Layer ───────────────────────────────────────────────────
    FogComponent->SecondFogData.FogDensity      = bEnableSecondFogLayer ? SecondFogDensity      : 0.0f;
    FogComponent->SecondFogData.FogHeightFalloff = bEnableSecondFogLayer ? SecondFogHeightFalloff : 0.2f;
    FogComponent->SecondFogData.FogHeightOffset  = bEnableSecondFogLayer ? SecondFogHeightOffset  : 0.0f;

    // ── Volumetric Fog ─────────────────────────────────────────────────────
    FogComponent->SetVolumetricFog(bVolumetricFog);

    if (bVolumetricFog)
    {
        FogComponent->SetVolumetricFogScatteringDistribution(VolumetricFogScatteringDistribution);
        FogComponent->SetVolumetricFogAlbedo(VolumetricFogAlbedo);
        FogComponent->SetVolumetricFogEmissive(VolumetricFogEmissive);
        FogComponent->SetVolumetricFogExtinctionScale(VolumetricFogExtinctionScale);
        FogComponent->SetVolumetricFogDistance(VolumetricFogDistance);
        FogComponent->SetVolumetricFogNearFadeInDistance(VolumetricFogNearFadeInDistance);
        FogComponent->SetVolumetricFogStaticLightingScatteringIntensity(
            VolumetricFogStaticLightingScatteringIntensity);

        if (VolumetricFogTemporalReprojectionBlendFactor >= 0.0f)
        {
            FogComponent->VolumetricFogTemporalReprojectionBlendFactor =
                VolumetricFogTemporalReprojectionBlendFactor;
        }
    }
}

// ─── Editor live-preview ──────────────────────────────────────────────────────

#if WITH_EDITOR
void AExponentialHeightFogActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyAllSettings();
}
#endif

// ─── Runtime API ─────────────────────────────────────────────────────────────

void AExponentialHeightFogActor::SetFogDensity(float NewDensity)
{
    FogDensity = FMath::Max(NewDensity, 0.0f);
    if (FogComponent)
    {
        FogComponent->SetFogDensity(FogDensity);
    }
}

void AExponentialHeightFogActor::SetFogHeightOffset(float NewOffset)
{
    FogHeightOffset = NewOffset;
    FVector Loc = GetActorLocation();
    Loc.Z = FogHeightOffset;
    SetActorLocation(Loc);
}

void AExponentialHeightFogActor::SetFogHeightFalloff(float NewFalloff)
{
    FogHeightFalloff = FMath::Max(NewFalloff, 0.001f);
    if (FogComponent)
    {
        FogComponent->SetFogHeightFalloff(FogHeightFalloff);
    }
}

void AExponentialHeightFogActor::SetVolumetricFogEnabled(bool bEnabled)
{
    bVolumetricFog = bEnabled;
    if (FogComponent)
    {
        FogComponent->SetVolumetricFog(bVolumetricFog);
    }
}

void AExponentialHeightFogActor::SetVolumetricFogExtinctionScale(float NewScale)
{
    VolumetricFogExtinctionScale = FMath::Max(NewScale, 0.1f);
    if (FogComponent && bVolumetricFog)
    {
        FogComponent->SetVolumetricFogExtinctionScale(VolumetricFogExtinctionScale);
    }
}

void AExponentialHeightFogActor::SetVolumetricFogDistance(float NewDistance)
{
    VolumetricFogDistance = FMath::Max(NewDistance, 0.0f);
    if (FogComponent && bVolumetricFog)
    {
        FogComponent->SetVolumetricFogDistance(VolumetricFogDistance);
    }
}

void AExponentialHeightFogActor::SetVolumetricFogAlbedo(FColor NewAlbedo)
{
    VolumetricFogAlbedo = NewAlbedo;
    if (FogComponent && bVolumetricFog)
    {
        FogComponent->SetVolumetricFogAlbedo(VolumetricFogAlbedo);
    }
}

void AExponentialHeightFogActor::SetVolumetricFogScatteringDistribution(float NewG)
{
    VolumetricFogScatteringDistribution = FMath::Clamp(NewG, -0.9f, 0.9f);
    if (FogComponent && bVolumetricFog)
    {
        FogComponent->SetVolumetricFogScatteringDistribution(VolumetricFogScatteringDistribution);
    }
}

void AExponentialHeightFogActor::AnimateFogDensityTo(float TargetDensity, float DurationSeconds)
{
    if (DurationSeconds <= 0.0f)
    {
        SetFogDensity(TargetDensity);
        return;
    }

    AnimStartDensity  = FogDensity;
    AnimTargetDensity = FMath::Max(TargetDensity, 0.0f);
    AnimDuration      = DurationSeconds;
    AnimElapsed       = 0.0f;
    bAnimatingDensity = true;
}
