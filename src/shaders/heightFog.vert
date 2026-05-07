varying vec3 vWorldPosition;
varying vec3 vViewPosition;
varying vec2 vUv;

void main() {
  vUv = uv;

  vec4 worldPos = modelMatrix * vec4(position, 1.0);
  vWorldPosition = worldPos.xyz;

  vec4 mvPosition = viewMatrix * worldPos;
  vViewPosition = -mvPosition.xyz;

  gl_Position = projectionMatrix * mvPosition;
}
