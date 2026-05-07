// Copyright yeonheehan. All Rights Reserved.
// Runtime fog manager: attaches to any Actor and drives AExponentialHeightFogActor
// from Blueprint, C++, or Data Tables.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogManagerComponent.generated.h"

class AExponentialHeightFogActor;

// ─── Preset struct ────────────────────────────────────────────────────────────

/**
 * FFogPreset
 *
 * A full snapshot of fog + volumetric parameters for the panopticon scene.
 * Presets can be transitioned between at runtime; all colour fields are
 * smoothly interpolated during transitions.
 */
USTRUCT(BlueprintType)
struct FFogPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    FName PresetName = NAME_None;

    // ── Height Fog ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.0", ClampMax="10.0"))
    float FogDensity = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    float FogHeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.001", ClampMax="2.0"))
    float FogHeightFalloff = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(ClampMin="0.0", ClampMax="1.0"))
    float FogMaxOpacity = 1.0f;

    // ── Inscattering colour (transitions between grey → red on detection) ────

    /** Directional inscattering colour. Grey for idle state, red for surveillance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    FLinearColor InscatteringColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // ── Volumetric Fog ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset")
    bool bVolumetricFog = true;

    /** Albedo: grey-black for absorbing fog. Pure white = bright cloud, black = thick smoke. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(EditCondition="bVolumetricFog"))
    FColor VolumetricFogAlbedo = FColor(140, 140, 140);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.1", ClampMax="10.0"))
    float VolumetricFogExtinctionScale = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog Preset",
              meta=(EditCondition="bVolumetricFog", ClampMin="0.0"))
    float VolumetricFogDistance = 2500.0f;

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

    // ── Panopticon surveillance events ───────────────────────────────────────

    /**
     * Call when the Eye detects the player.
     * Transitions from "Panopticon_Idle" to "Panopticon_Detected" fog state:
     * fog becomes denser and inscattering shifts to red from the spotlight.
     * @param TransitionSec Override transition time (< 0 = use preset default).
     */
    UFUNCTION(BlueprintCallable, Category="Surveillance Fog")
    void OnPlayerDetected(float TransitionSec = -1.0f);

    /**
     * Call when surveillance ends (e.g. player hides or resets).
     * Transitions back to "Panopticon_Idle".
     */
    UFUNCTION(BlueprintCallable, Category="Surveillance Fog")
    void OnSurveillanceEnded(float TransitionSec = -1.0f);

    /** True while the eye is actively tracking the player. */
    UFUNCTION(BlueprintPure, Category="Surveillance Fog")
    bool IsSurveillanceActive() const { return bSurveillanceActive; }

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

    // Surveillance state
    bool       bSurveillanceActive  = false;
};
