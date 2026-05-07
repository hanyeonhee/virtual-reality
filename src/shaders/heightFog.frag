// Exponential Height Fog
// Based on Unreal Engine's fog model with analytical height-fog integral

uniform vec3  uFogColor;
uniform float uFogDensity;         // global density scalar
uniform float uFogHeight;          // world-space base height
uniform float uFogHeightFalloff;   // density falloff per unit above base
uniform float uFogStart;           // distance before fog begins
uniform float uFogMaxOpacity;      // clamp fog opacity [0,1]

// Volumetric / inscattering
uniform vec3  uInscatteringColor;
uniform float uInscatteringStrength;
uniform vec3  uSunDirection;       // normalised toward sun
uniform float uMieG;               // Henyey-Greenstein anisotropy [-1,1]

// Scene
uniform vec3  uCameraPosition;
uniform sampler2D uTexture;

varying vec3 vWorldPosition;
varying vec2 vUv;

// ─── helpers ────────────────────────────────────────────────────────────────

float henyeyGreenstein(float cosTheta, float g) {
  float g2 = g * g;
  return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Analytical integral of exp(-falloff * (h0 + slope*t - fogBase)) dt from 0 to rayLen
// Gives the optical depth along the ray segment.
float heightFogOpticalDepth(
  float h0,        // camera height relative to fog base
  float slope,     // ray dir y component (dh/dt)
  float rayLen,    // total ray length (clipped to uFogStart)
  float falloff
) {
  float clampedH0 = max(h0, 0.0);

  if (abs(slope) < 1e-4) {
    // Horizontal ray — density is constant along the segment
    return exp(-falloff * clampedH0) * rayLen;
  }

  // Integral of exp(-falloff*(h0 + slope*t)) dt
  //   = exp(-falloff*h0) * (1 - exp(-falloff*slope*rayLen)) / (falloff*slope)
  float a = -falloff * slope * rayLen;
  float integral = exp(-falloff * clampedH0) * (1.0 - exp(a)) / (falloff * slope);
  return max(integral, 0.0);
}

// ─── main ───────────────────────────────────────────────────────────────────

void main() {
  vec4 texColor = texture2D(uTexture, vUv);

  // Ray from camera to fragment
  vec3 rayVec   = vWorldPosition - uCameraPosition;
  float rayLen  = length(rayVec);
  vec3 rayDir   = rayVec / rayLen;

  // Subtract fog-start distance
  float effectiveLen = max(rayLen - uFogStart, 0.0);

  // Height relative to fog base
  float h0    = uCameraPosition.y - uFogHeight;
  float slope = rayDir.y;

  // Optical depth
  float opticalDepth = uFogDensity * heightFogOpticalDepth(h0, slope, effectiveLen, uFogHeightFalloff);

  // Transmittance
  float transmittance = exp(-opticalDepth);
  float fogFactor = clamp(1.0 - transmittance, 0.0, uFogMaxOpacity);

  // ── Inscattering / volumetric glow ──────────────────────────────────────
  float cosTheta = dot(rayDir, normalize(-uSunDirection));
  float phase    = henyeyGreenstein(cosTheta, uMieG);

  // Inscattering accumulates proportionally to optical depth
  float inscatterAmt = (1.0 - transmittance) * phase * uInscatteringStrength;
  vec3  inscatter    = uInscatteringColor * inscatterAmt;

  // Final composite: blend fog over scene colour then add inscattering
  vec3 finalColor = mix(texColor.rgb, uFogColor, fogFactor) + inscatter;

  gl_FragColor = vec4(finalColor, texColor.a);
}
