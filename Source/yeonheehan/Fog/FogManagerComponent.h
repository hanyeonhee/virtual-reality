// Copyright yeonheehan. All Rights Reserved.
// Runtime fog manager: attaches to any Actor and drives AExponentialHeightFogActor
// from Blueprint, C++, or Data Tables.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogManagerComponent.generated.h"

class AExponentialHeightFogActor;

// ─── Preset struct ────────────────────────────────────────────────────────────

/** A snapshot of all fog settings, usable as a named preset. */
USTRUCT(BlueprintType)
struct FFogPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    FName PresetName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.0", ClampMax="10.0"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    float FogHeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.001", ClampMax="2.0"))
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.1", ClampMax="10.0"))
    float VolumetricFogExtinctionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(EditCondition="bVolumetricFog"))
    float VolumetricFogDistance = 6000.0f;

    /** Transition duration in seconds when blending to this preset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.0"))
    float TransitionDuration = 2.0f;
};

// ─── Component ────────────────────────────────────────────────────────────────

/**
 * UFogManagerComponent
 *
 * Attach to your Game Mode, Game State, or any manager Actor to gain
 * Blueprint-callable control over AExponentialHeightFogActor, including
 * preset transitions driven by smooth interpolation.
 *
 * Typical usage:
 *   1. Add component to BP_GameMode.
 *   2. Set FogActorReference (or use AutoFindFogActor=true).
 *   3. Call ApplyPreset("MorningMist") from Blueprint on level load.
 *   4. Call TransitionToPreset("DenseNight", 5.0f) at dusk.
 */
UCLASS(ClassGroup=(Fog), meta=(BlueprintSpawnableComponent))
class YEONHEEHAN_API UFogManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFogManagerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ────────────────────────────────────────────────────────

    /**
     * Direct reference to the AExponentialHeightFogActor in the level.
     * If null and bAutoFindFogActor is true, BeginPlay searches the world.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Manager")
    TObjectPtr<AExponentialHeightFogActor> FogActorReference;

    /** Auto-locate the first AExponentialHeightFogActor if no reference is set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Manager")
    bool bAutoFindFogActor = true;

    /** Named presets editable in the Details panel. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Manager")
    TArray<FFogPreset> Presets;

    // ── Runtime API ──────────────────────────────────────────────────────────

    /**
     * Immediately apply a named preset.
     * @param PresetName  Must match a name in the Presets array.
     * @return true if the preset was found and applied.
     */
    UFUNCTION(BlueprintCallable, Category="Fog Manager")
    bool ApplyPreset(FName PresetName);

    /**
     * Blend from the current fog state to a named preset over DurationSeconds.
     * If DurationSeconds < 0, the preset's own TransitionDuration is used.
     */
    UFUNCTION(BlueprintCallable, Category="Fog Manager")
    bool TransitionToPreset(FName PresetName, float DurationSeconds = -1.0f);

    /** Linearly interpolate between two presets, writing the result directly to the fog actor. */
    UFUNCTION(BlueprintCallable, Category="Fog Manager")
    void BlendPresets(const FFogPreset& A, const FFogPreset& B, float Alpha);

    /** Capture the current fog state as a runtime preset (not persisted). */
    UFUNCTION(BlueprintCallable, Category="Fog Manager")
    FFogPreset CaptureCurrent() const;

    /** Returns true while a preset transition is in progress. */
    UFUNCTION(BlueprintPure, Category="Fog Manager")
    bool IsTransitioning() const { return bTransitioning; }

    /** Cancel any in-progress transition, freezing values at the current interpolated state. */
    UFUNCTION(BlueprintCallable, Category="Fog Manager")
    void CancelTransition();

    // ── Fog component delegates (broadcast on transition complete) ───────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPresetApplied, FName, PresetName);

    UPROPERTY(BlueprintAssignable, Category="Fog Manager")
    FOnPresetApplied OnPresetApplied;

private:
    AExponentialHeightFogActor* GetFogActor() const;
    void                        ApplyPresetDirect(const FFogPreset& Preset);

    // Transition state
    bool       bTransitioning       = false;
    FFogPreset TransitionFrom;
    FFogPreset TransitionTo;
    float      TransitionDuration   = 0.0f;
    float      TransitionElapsed    = 0.0f;
    FName      TransitionTargetName = NAME_None;
};
