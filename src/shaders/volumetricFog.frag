// Volumetric Height Fog — ray-march post-process pass
// Reconstructs world position from depth, then marches toward the camera
// accumulating density + single-scattering from a directional light.

precision highp float;

uniform sampler2D uSceneColor;
uniform sampler2D uDepthTexture;

uniform mat4 uInverseProjection;
uniform mat4 uInverseView;
uniform vec3 uCameraPosition;

// Exponential height fog params
uniform float uFogDensity;
uniform float uFogHeight;
uniform float uFogHeightFalloff;
uniform float uFogStart;

// Volumetric params
uniform int   uMarchSteps;          // number of ray-march steps (8..64)
uniform float uMarchMaxDistance;    // max ray distance to march

// Lighting
uniform vec3  uFogColor;
uniform vec3  uInscatteringColor;
uniform float uInscatteringStrength;
uniform vec3  uSunDirection;        // toward sun (normalised)
uniform vec3  uSunColor;
uniform float uMieG;
uniform float uAmbientStrength;

uniform float uTime;
uniform float uNoiseTiling;
uniform float uNoiseStrength;       // 0 = off, 1 = full noise modulation

varying vec2 vUv;

// ─── helpers ────────────────────────────────────────────────────────────────

float henyeyGreenstein(float cosTheta, float g) {
  float g2 = g * g;
  return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

// Simple value-noise hash
float hash(vec3 p) {
  p = fract(p * 0.3183099 + 0.1);
  p *= 17.0;
  return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

// 3-D value noise (trilinear)
float noise3D(vec3 p) {
  vec3 i = floor(p);
  vec3 f = fract(p);
  vec3 u = f * f * (3.0 - 2.0 * f);   // smoothstep

  return mix(
    mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), u.x),
        mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), u.x), u.y),
    mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), u.x),
        mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), u.x), u.y),
    u.z
  );
}

// Fractal Brownian Motion (2 octaves — cheap)
float fbm(vec3 p) {
  float v = 0.0;
  float a = 0.5;
  for (int i = 0; i < 2; i++) {
    v += a * noise3D(p);
    p  = p * 2.0 + vec3(uTime * 0.03, 0.0, uTime * 0.02);
    a *= 0.5;
  }
  return v;
}

// Local density at world position p
float sampleDensity(vec3 p) {
  float heightAboveBase = p.y - uFogHeight;
  float baseDensity     = uFogDensity * exp(-uFogHeightFalloff * max(heightAboveBase, 0.0));

  // Optional noise modulation
  float n = fbm(p * uNoiseTiling);
  float noiseModulation = mix(1.0, n * 2.0, uNoiseStrength);

  return baseDensity * noiseModulation;
}

// Reconstruct world-space position from depth buffer sample
vec3 reconstructWorldPos(vec2 uv, float depth) {
  // NDC [-1,1]
  vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

  // View space
  vec4 viewPos = uInverseProjection * ndc;
  viewPos /= viewPos.w;

  // World space
  vec4 worldPos = uInverseView * viewPos;
  return worldPos.xyz;
}

// ─── main ───────────────────────────────────────────────────────────────────

void main() {
  vec4 sceneColor = texture2D(uSceneColor, vUv);
  float depth     = texture2D(uDepthTexture, vUv).r;

  // Sky / background — depth == 1.0
  bool isSky = (depth >= 0.9999);

  vec3 worldPos = isSky
    ? uCameraPosition + normalize(reconstructWorldPos(vUv, 0.5) - uCameraPosition) * uMarchMaxDistance
    : reconstructWorldPos(vUv, depth);

  // Ray
  vec3  rayVec  = worldPos - uCameraPosition;
  float rayLen  = length(rayVec);
  vec3  rayDir  = rayVec / max(rayLen, 1e-6);

  float marchLen = min(max(rayLen - uFogStart, 0.0), uMarchMaxDistance);

  if (marchLen <= 0.0) {
    gl_FragColor = sceneColor;
    return;
  }

  // ── Ray march ─────────────────────────────────────────────────────────────
  float stepSize  = marchLen / float(uMarchSteps);
  float startDist = uFogStart;

  float transmittance   = 1.0;
  vec3  inscattering    = vec3(0.0);

  float cosTheta = dot(rayDir, normalize(uSunDirection));
  float phase    = henyeyGreenstein(cosTheta, uMieG);

  for (int i = 0; i < 64; i++) {
    if (i >= uMarchSteps) break;

    float t   = startDist + (float(i) + 0.5) * stepSize;
    vec3  pos = uCameraPosition + rayDir * t;

    float density = sampleDensity(pos);
    float sigma   = density * stepSize;   // optical contribution this step

    // Beer-Lambert transmittance for this step
    float stepTransmit = exp(-sigma);

    // Single scattering: energy entering this step = transmittance * density
    float scatter = transmittance * (1.0 - stepTransmit);

    // Direct sun inscattering
    inscattering += scatter * phase * uSunColor * uInscatteringStrength;

    // Ambient inscattering (isotropic)
    inscattering += scatter * uFogColor * uAmbientStrength;

    transmittance *= stepTransmit;

    if (transmittance < 0.01) break; // early-out: fully opaque
  }

  // Composite
  vec3 finalColor = sceneColor.rgb * transmittance + inscattering;

  gl_FragColor = vec4(finalColor, sceneColor.a);
}
