/**
 * ExponentialHeightFog
 *
 * Applies an analytical exponential-height-fog pass to every mesh that uses
 * the provided material override, or can be used as a post-process effect via
 * the companion <VolumetricFog> component.
 *
 * Props
 * ─────
 * fogColor            – vec3 RGB sky/fog colour          default #c8d8e8
 * fogDensity          – global density multiplier        default 0.02
 * fogHeight           – world-Y base of the fog layer    default 0.0
 * fogHeightFalloff    – density falloff above base       default 0.15
 * fogStart            – distance before fog begins       default 0.0
 * fogMaxOpacity       – 0..1 clamp on maximum fog        default 0.95
 * inscatteringColor   – sun-inscattering tint            default #ffe0a0
 * inscatteringStrength                                   default 0.6
 * sunDirection        – world-space toward sun           default [0.3,0.8,0.5]
 * mieG                – Henyey-Greenstein anisotropy     default 0.76
 */

import { useMemo, useRef } from 'react'
import { useFrame, useThree } from '@react-three/fiber'
import * as THREE from 'three'
import heightFogVert from '../shaders/heightFog.vert?raw'
import heightFogFrag from '../shaders/heightFog.frag?raw'

export function createHeightFogMaterial(params = {}) {
  const {
    fogColor            = new THREE.Color('#c8d8e8'),
    fogDensity          = 0.02,
    fogHeight           = 0.0,
    fogHeightFalloff    = 0.15,
    fogStart            = 0.0,
    fogMaxOpacity       = 0.95,
    inscatteringColor   = new THREE.Color('#ffe0a0'),
    inscatteringStrength = 0.6,
    sunDirection        = new THREE.Vector3(0.3, 0.8, 0.5).normalize(),
    mieG                = 0.76,
    map                 = null,
  } = params

  return new THREE.ShaderMaterial({
    vertexShader:   heightFogVert,
    fragmentShader: heightFogFrag,
    transparent:    true,
    uniforms: {
      uFogColor:             { value: new THREE.Color(fogColor) },
      uFogDensity:           { value: fogDensity },
      uFogHeight:            { value: fogHeight },
      uFogHeightFalloff:     { value: fogHeightFalloff },
      uFogStart:             { value: fogStart },
      uFogMaxOpacity:        { value: fogMaxOpacity },
      uInscatteringColor:    { value: new THREE.Color(inscatteringColor) },
      uInscatteringStrength: { value: inscatteringStrength },
      uSunDirection:         { value: sunDirection instanceof THREE.Vector3 ? sunDirection : new THREE.Vector3(...sunDirection) },
      uMieG:                 { value: mieG },
      uCameraPosition:       { value: new THREE.Vector3() },
      uTexture:              { value: map || new THREE.Texture() },
    },
  })
}

// ─── React component (updates camera uniform each frame) ────────────────────

export default function ExponentialHeightFog({
  fogColor            = '#c8d8e8',
  fogDensity          = 0.02,
  fogHeight           = 0.0,
  fogHeightFalloff    = 0.15,
  fogStart            = 0.0,
  fogMaxOpacity       = 0.95,
  inscatteringColor   = '#ffe0a0',
  inscatteringStrength = 0.6,
  sunDirection        = [0.3, 0.8, 0.5],
  mieG                = 0.76,
}) {
  const materialRef = useRef(null)
  const { camera } = useThree()

  const material = useMemo(() => createHeightFogMaterial({
    fogColor:            new THREE.Color(fogColor),
    fogDensity,
    fogHeight,
    fogHeightFalloff,
    fogStart,
    fogMaxOpacity,
    inscatteringColor:   new THREE.Color(inscatteringColor),
    inscatteringStrength,
    sunDirection:        new THREE.Vector3(...sunDirection).normalize(),
    mieG,
  }), [
    fogColor, fogDensity, fogHeight, fogHeightFalloff, fogStart,
    fogMaxOpacity, inscatteringColor, inscatteringStrength,
    sunDirection[0], sunDirection[1], sunDirection[2], mieG,
  ])

  materialRef.current = material

  // Sync live props to uniforms every frame
  useFrame(() => {
    const u = materialRef.current?.uniforms
    if (!u) return
    u.uCameraPosition.value.copy(camera.position)
    u.uFogDensity.value           = fogDensity
    u.uFogHeight.value            = fogHeight
    u.uFogHeightFalloff.value     = fogHeightFalloff
    u.uFogStart.value             = fogStart
    u.uFogMaxOpacity.value        = fogMaxOpacity
    u.uInscatteringStrength.value = inscatteringStrength
    u.uMieG.value                 = mieG
  })

  // Expose material via context so child meshes can use it
  return null
}

export { ExponentialHeightFog }
