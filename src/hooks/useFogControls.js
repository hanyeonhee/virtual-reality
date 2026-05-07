import { useControls, folder } from 'leva'

export function useFogControls() {
  const fog = useControls('Exponential Height Fog', {
    fogColor: { value: '#c8d8e8', label: 'Fog Color' },
    fogDensity: { value: 0.015, min: 0.001, max: 0.2, step: 0.001, label: 'Density' },
    fogHeight: { value: 0.0, min: -20, max: 50, step: 0.5, label: 'Height Base' },
    fogHeightFalloff: { value: 0.12, min: 0.01, max: 1.0, step: 0.01, label: 'Height Falloff' },
    fogStart: { value: 5.0, min: 0, max: 100, step: 1, label: 'Start Distance' },
  })

  const vol = useControls('Volumetric Fog', {
    enabled: { value: true, label: 'Enabled' },
    marchSteps: { value: 24, min: 4, max: 64, step: 4, label: 'March Steps' },
    marchMaxDistance: { value: 200, min: 50, max: 500, step: 10, label: 'Max Distance' },
    ambientStrength: { value: 0.4, min: 0, max: 2, step: 0.05, label: 'Ambient' },
    noiseStrength: { value: 0.35, min: 0, max: 1, step: 0.05, label: 'Noise Strength' },
    noiseTiling: { value: 0.08, min: 0.01, max: 0.5, step: 0.01, label: 'Noise Tiling' },
  })

  const light = useControls('Sun / Inscattering', {
    inscatteringColor: { value: '#ffcc88', label: 'Inscatter Color' },
    inscatteringStrength: { value: 1.2, min: 0, max: 5, step: 0.1, label: 'Inscatter Strength' },
    sunColor: { value: '#fff0d0', label: 'Sun Color' },
    mieG: { value: 0.76, min: -0.99, max: 0.99, step: 0.01, label: 'Mie G (anisotropy)' },
    sunAzimuth: { value: 45, min: 0, max: 360, step: 1, label: 'Sun Azimuth °' },
    sunElevation: { value: 35, min: 1, max: 89, step: 1, label: 'Sun Elevation °' },
  })

  // Compute sun direction from azimuth / elevation
  const azRad = (light.sunAzimuth * Math.PI) / 180
  const elRad = (light.sunElevation * Math.PI) / 180
  const sunDirection = [
    Math.cos(elRad) * Math.sin(azRad),
    Math.sin(elRad),
    Math.cos(elRad) * Math.cos(azRad),
  ]

  return {
    ...fog,
    ...vol,
    inscatteringColor: light.inscatteringColor,
    inscatteringStrength: light.inscatteringStrength,
    sunColor: light.sunColor,
    mieG: light.mieG,
    sunDirection,
  }
}
