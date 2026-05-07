/**
 * VolumetricFog — post-process pass using ray-marching
 *
 * Integrates with @react-three/postprocessing (Effect API).
 * Uses a depth texture to reconstruct world positions, then
 * ray-marches through the exponential height-fog volume adding
 * single-scattered sun light via Henyey-Greenstein phase function.
 *
 * Usage:
 *   <EffectComposer depthBuffer>
 *     <VolumetricFog ... />
 *   </EffectComposer>
 *
 * Props
 * ─────
 * fogColor             – atmosphere / ambient fog colour   default '#c8d8e8'
 * fogDensity           – global density multiplier         default 0.015
 * fogHeight            – world-Y fog base                  default 0.0
 * fogHeightFalloff     – falloff per unit above base       default 0.12
 * fogStart             – distance before fog begins        default 0.0
 * marchSteps           – ray-march samples (8..64)         default 24
 * marchMaxDistance     – max march distance                default 200
 * inscatteringColor    – sun light colour                  default '#ffcc88'
 * inscatteringStrength – sun inscattering intensity        default 1.2
 * sunDirection         – toward-sun vec3 (normalised)      default [0.3,0.8,0.5]
 * sunColor             – direct sun light colour           default '#fff0d0'
 * mieG                 – Mie anisotropy [-1,1]             default 0.76
 * ambientStrength      – isotropic fog light               default 0.4
 * noiseTiling          – 3-D noise tiling scale            default 0.08
 * noiseStrength        – 0=off  1=max noise modulation     default 0.35
 */

import { forwardRef, useMemo } from 'react'
import { Effect, BlendFunction } from 'postprocessing'
import * as THREE from 'three'
import { useThree } from '@react-three/fiber'
import volumetricVert from '../shaders/volumetricFog.vert?raw'
import volumetricFrag from '../shaders/volumetricFog.frag?raw'

// ─── Core postprocessing Effect ──────────────────────────────────────────────

class VolumetricFogEffect extends Effect {
  constructor({
    fogColor            = new THREE.Color('#c8d8e8'),
    fogDensity          = 0.015,
    fogHeight           = 0.0,
    fogHeightFalloff    = 0.12,
    fogStart            = 0.0,
    marchSteps          = 24,
    marchMaxDistance    = 200,
    inscatteringColor   = new THREE.Color('#ffcc88'),
    inscatteringStrength = 1.2,
    sunDirection        = new THREE.Vector3(0.3, 0.8, 0.5),
    sunColor            = new THREE.Color('#fff0d0'),
    mieG                = 0.76,
    ambientStrength     = 0.4,
    noiseTiling         = 0.08,
    noiseStrength       = 0.35,
    camera,
  } = {}) {
    super('VolumetricFogEffect', volumetricFrag, {
      blendFunction: BlendFunction.NORMAL,
      uniforms: new Map([
        ['uFogColor',             new THREE.Uniform(new THREE.Color(fogColor))],
        ['uFogDensity',           new THREE.Uniform(fogDensity)],
        ['uFogHeight',            new THREE.Uniform(fogHeight)],
        ['uFogHeightFalloff',     new THREE.Uniform(fogHeightFalloff)],
        ['uFogStart',             new THREE.Uniform(fogStart)],
        ['uMarchSteps',           new THREE.Uniform(marchSteps)],
        ['uMarchMaxDistance',     new THREE.Uniform(marchMaxDistance)],
        ['uInscatteringColor',    new THREE.Uniform(new THREE.Color(inscatteringColor))],
        ['uInscatteringStrength', new THREE.Uniform(inscatteringStrength)],
        ['uSunDirection',         new THREE.Uniform(new THREE.Vector3().copy(sunDirection).normalize())],
        ['uSunColor',             new THREE.Uniform(new THREE.Color(sunColor))],
        ['uMieG',                 new THREE.Uniform(mieG)],
        ['uAmbientStrength',      new THREE.Uniform(ambientStrength)],
        ['uNoiseTiling',          new THREE.Uniform(noiseTiling)],
        ['uNoiseStrength',        new THREE.Uniform(noiseStrength)],
        ['uCameraPosition',       new THREE.Uniform(camera ? camera.position.clone() : new THREE.Vector3())],
        ['uInverseProjection',    new THREE.Uniform(new THREE.Matrix4())],
        ['uInverseView',          new THREE.Uniform(new THREE.Matrix4())],
        ['uTime',                 new THREE.Uniform(0)],
      ]),
    })

    this._camera = camera
  }

  update(renderer, inputBuffer, deltaTime) {
    const u = this.uniforms
    if (this._camera) {
      u.get('uCameraPosition').value.copy(this._camera.position)
      u.get('uInverseProjection').value.copy(this._camera.projectionMatrixInverse)
      u.get('uInverseView').value.copy(this._camera.matrixWorld)
    }
    u.get('uTime').value += deltaTime
  }

  // Convenience setters for live prop updates
  set fogDensity(v)           { this.uniforms.get('uFogDensity').value = v }
  set fogHeight(v)            { this.uniforms.get('uFogHeight').value = v }
  set fogHeightFalloff(v)     { this.uniforms.get('uFogHeightFalloff').value = v }
  set fogStart(v)             { this.uniforms.get('uFogStart').value = v }
  set marchSteps(v)           { this.uniforms.get('uMarchSteps').value = v }
  set marchMaxDistance(v)     { this.uniforms.get('uMarchMaxDistance').value = v }
  set inscatteringStrength(v) { this.uniforms.get('uInscatteringStrength').value = v }
  set mieG(v)                 { this.uniforms.get('uMieG').value = v }
  set ambientStrength(v)      { this.uniforms.get('uAmbientStrength').value = v }
  set noiseStrength(v)        { this.uniforms.get('uNoiseStrength').value = v }
  set noiseTiling(v)          { this.uniforms.get('uNoiseTiling').value = v }
  set sunDirection(v)         { this.uniforms.get('uSunDirection').value.copy(v).normalize() }
  set fogColor(v)             { this.uniforms.get('uFogColor').value.set(v) }
  set inscatteringColor(v)    { this.uniforms.get('uInscatteringColor').value.set(v) }
  set sunColor(v)             { this.uniforms.get('uSunColor').value.set(v) }
}

// ─── React wrapper ───────────────────────────────────────────────────────────

const VolumetricFog = forwardRef(function VolumetricFog(props, ref) {
  const {
    fogColor            = '#c8d8e8',
    fogDensity          = 0.015,
    fogHeight           = 0.0,
    fogHeightFalloff    = 0.12,
    fogStart            = 0.0,
    marchSteps          = 24,
    marchMaxDistance    = 200,
    inscatteringColor   = '#ffcc88',
    inscatteringStrength = 1.2,
    sunDirection        = [0.3, 0.8, 0.5],
    sunColor            = '#fff0d0',
    mieG                = 0.76,
    ambientStrength     = 0.4,
    noiseTiling         = 0.08,
    noiseStrength       = 0.35,
  } = props

  const { camera } = useThree()

  const effect = useMemo(() => new VolumetricFogEffect({
    fogColor:            new THREE.Color(fogColor),
    fogDensity,
    fogHeight,
    fogHeightFalloff,
    fogStart,
    marchSteps,
    marchMaxDistance,
    inscatteringColor:   new THREE.Color(inscatteringColor),
    inscatteringStrength,
    sunDirection:        new THREE.Vector3(...sunDirection),
    sunColor:            new THREE.Color(sunColor),
    mieG,
    ambientStrength,
    noiseTiling,
    noiseStrength,
    camera,
  }), [camera]) // eslint-disable-line react-hooks/exhaustive-deps

  // Push live prop changes to the underlying effect
  effect.fogColor            = fogColor
  effect.fogDensity          = fogDensity
  effect.fogHeight           = fogHeight
  effect.fogHeightFalloff    = fogHeightFalloff
  effect.fogStart            = fogStart
  effect.marchSteps          = marchSteps
  effect.marchMaxDistance    = marchMaxDistance
  effect.inscatteringColor   = inscatteringColor
  effect.inscatteringStrength = inscatteringStrength
  effect.sunDirection        = new THREE.Vector3(...sunDirection)
  effect.sunColor            = sunColor
  effect.mieG                = mieG
  effect.ambientStrength     = ambientStrength
  effect.noiseTiling         = noiseTiling
  effect.noiseStrength       = noiseStrength

  return <primitive ref={ref} object={effect} />
})

export default VolumetricFog
export { VolumetricFogEffect }
