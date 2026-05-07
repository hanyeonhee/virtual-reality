// Copyright yeonheehan. All Rights Reserved.
// ExponentialHeightFog Actor with full Volumetric Fog support (UE5)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExponentialHeightFogActor.generated.h"

class UExponentialHeightFogComponent;

/**
 * AExponentialHeightFogActor
 *
 * Blueprint-friendly actor that wraps UExponentialHeightFogComponent
 * and exposes every height-fog + volumetric-fog parameter as a
 * UPROPERTY so designers can tweak them in the Details panel or
 * drive them at runtime from Blueprint / C++.
 *
 * Place this actor in the level instead of the engine default
 * ExponentialHeightFog to get runtime-adjustable volumetric fog.
 *
 * Usage (Blueprint):
 *   1. Drag into level → configure in Details.
 *   2. Call SetFogDensity() / SetVolumetricFog() etc. at runtime.
 *
 * Usage (C++):
 *   AExponentialHeightFogActor* FogActor =
 *       World->SpawnActor<AExponentialHeightFogActor>();
 *   FogActor->SetFogDensity(0.02f);
 *   FogActor->SetVolumetricFogEnabled(true);
 */
UCLASS(BlueprintType, Blueprintable, HideCategories=(Rendering, Replication, Input, Actor))
class YEONHEEHAN_API AExponentialHeightFogActor : public AActor
{
    GENERATED_BODY()

public:
    AExponentialHeightFogActor();

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    // ── Core component ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Fog")
    TObjectPtr<UExponentialHeightFogComponent> FogComponent;

    // ─────────────────────────────────────────────────────────────────────────
    // SECTION 1 : Exponential Height Fog
    // ─────────────────────────────────────────────────────────────────────────

    /** Global fog density scalar. Larger values = thicker fog. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(ClampMin="0.0", ClampMax="10.0", UIMin="0.0", UIMax="1.0"))
    float FogDensity = 0.02f;

    /**
     * Height (world units) at which the fog starts.
     * Fog density is highest at this height and falls off exponentially above it.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(UIMin="-10000.0", UIMax="10000.0"))
    float FogHeightOffset = 0.0f;

    /**
     * Controls how quickly fog density falls off above FogHeightOffset.
     * Smaller = fog spreads higher into the sky.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(ClampMin="0.001", ClampMax="2.0", UIMin="0.001", UIMax="2.0"))
    float FogHeightFalloff = 0.2f;

    /** Maximum opacity (0–1). Prevents fog from completely obscuring distant objects. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float FogMaxOpacity = 1.0f;

    /** Distance (cm) from the camera before fog starts accumulating. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(ClampMin="0.0", UIMin="0.0", UIMax="5000.0"))
    float StartDistance = 0.0f;

    /** World-space distance at which fog is fully opaque (-1 = infinite). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exponential Height Fog",
              meta=(UIMin="-1.0", UIMax="200000.0"))
    float FogCutoffDistance = -1.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // SECTION 2 : Directional Inscattering (Sun glow)
    // ─────────────────────────────────────────────────────────────────────────

    /** Colour of directional inscattering (sun halo in the fog). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inscattering")
    FLinearColor DirectionalInscatteringColor = FLinearColor(0.25f, 0.25f, 0.125f);

    /**
     * Exponent controlling how concentrated the sun halo is.
     * Higher = tighter highlight around the sun direction.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inscattering",
              meta=(ClampMin="0.000001", ClampMax="64.0"))
    float DirectionalInscatteringExponent = 4.0f;

    /** Distance at which directional inscattering begins (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inscattering",
              meta=(ClampMin="0.0"))
    float DirectionalInscatteringStartDistance = 10000.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // SECTION 3 : Second Fog Layer (optional)
    // ─────────────────────────────────────────────────────────────────────────

    /** Enable a second independent fog layer (e.g. high-altitude haze). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Second Fog Layer")
    bool bEnableSecondFogLayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Second Fog Layer",
              meta=(EditCondition="bEnableSecondFogLayer", ClampMin="0.0", ClampMax="10.0"))
    float SecondFogDensity = 0.005f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Second Fog Layer",
              meta=(EditCondition="bEnableSecondFogLayer", ClampMin="0.001", ClampMax="2.0"))
    float SecondFogHeightFalloff = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Second Fog Layer",
              meta=(EditCondition="bEnableSecondFogLayer"))
    float SecondFogHeightOffset = 200.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // SECTION 4 : Volumetric Fog
    // ─────────────────────────────────────────────────────────────────────────

    /** Enable full volumetric fog (ray-marched, supports dynamic shadows, light shafts). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog")
    bool bVolumetricFog = true;

    /**
     * Scattering distribution (Henyey-Greenstein g parameter).
     * 0 = isotropic. Negative = back-scatter. Positive = forward-scatter (sun glow).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="-0.9", ClampMax="0.9"))
    float VolumetricFogScatteringDistribution = 0.2f;

    /**
     * Albedo: fraction of light scattered vs. absorbed.
     * White = fully scattering (bright clouds). Black = fully absorbing (smoke).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog"))
    FColor VolumetricFogAlbedo = FColor(255, 255, 255);

    /** Emissive glow colour from the fog itself (e.g. fire, lava glow). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog"))
    FLinearColor VolumetricFogEmissive = FLinearColor::Black;

    /**
     * Multiplier on the fog extinction coefficient.
     * Increasing this makes the volumetric fog thicker/denser.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.1", ClampMax="10.0"))
    float VolumetricFogExtinctionScale = 1.0f;

    /** Maximum distance (cm) the volumetric fog is computed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.0", UIMax="200000.0"))
    float VolumetricFogDistance = 6000.0f;

    /**
     * Near fade distance (cm).
     * Helps avoid harsh contact between the camera near-plane and the volume.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.0"))
    float VolumetricFogNearFadeInDistance = 0.0f;

    /**
     * How strongly baked (static) lighting contributes to volumetric inscattering.
     * 0 = no baked light in fog, 1 = full contribution.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.0", ClampMax="1.0"))
    float VolumetricFogStaticLightingScatteringIntensity = 1.0f;

    /**
     * Overrides the per-frame temporal reprojection blend factor.
     * < 0 = use engine default. 0 = no reprojection (crisp, flickery). 1 = max blur.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Volumetric Fog",
              meta=(EditCondition="bVolumetricFog", ClampMin="-1.0", ClampMax="1.0"))
    float VolumetricFogTemporalReprojectionBlendFactor = -1.0f;

    // ─────────────────────────────────────────────────────────────────────────
    // SECTION 5 : Runtime API (BlueprintCallable)
    // ─────────────────────────────────────────────────────────────────────────

    /** Apply a new fog density at runtime. */
    UFUNCTION(BlueprintCallable, Category="Exponential Height Fog")
    void SetFogDensity(float NewDensity);

    /** Move the fog base height at runtime. */
    UFUNCTION(BlueprintCallable, Category="Exponential Height Fog")
    void SetFogHeightOffset(float NewOffset);

    /** Adjust height falloff at runtime. */
    UFUNCTION(BlueprintCallable, Category="Exponential Height Fog")
    void SetFogHeightFalloff(float NewFalloff);

    /** Enable or disable volumetric fog at runtime. */
    UFUNCTION(BlueprintCallable, Category="Volumetric Fog")
    void SetVolumetricFogEnabled(bool bEnabled);

    /** Set volumetric extinction scale at runtime. */
    UFUNCTION(BlueprintCallable, Category="Volumetric Fog")
    void SetVolumetricFogExtinctionScale(float NewScale);

    /** Set volumetric fog distance at runtime. */
    UFUNCTION(BlueprintCallable, Category="Volumetric Fog")
    void SetVolumetricFogDistance(float NewDistance);

    /** Set volumetric albedo (scattering colour) at runtime. */
    UFUNCTION(BlueprintCallable, Category="Volumetric Fog")
    void SetVolumetricFogAlbedo(FColor NewAlbedo);

    /** Set Henyey-Greenstein scattering distribution (–0.9 .. 0.9). */
    UFUNCTION(BlueprintCallable, Category="Volumetric Fog")
    void SetVolumetricFogScatteringDistribution(float NewG);

    /** Smoothly interpolate fog density toward a target over DurationSeconds. */
    UFUNCTION(BlueprintCallable, Category="Exponential Height Fog")
    void AnimateFogDensityTo(float TargetDensity, float DurationSeconds);

    // ─────────────────────────────────────────────────────────────────────────
    // Internals
    // ─────────────────────────────────────────────────────────────────────────

private:
    /** Push all UPROPERTY values to the fog component. */
    void ApplyAllSettings();

    // Density animation state
    bool   bAnimatingDensity   = false;
    float  AnimStartDensity    = 0.0f;
    float  AnimTargetDensity   = 0.0f;
    float  AnimDuration        = 0.0f;
    float  AnimElapsed         = 0.0f;

    virtual void Tick(float DeltaTime) override;
};
