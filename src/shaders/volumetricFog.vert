// Full-screen quad pass for volumetric fog (post-process)
varying vec2 vUv;

void main() {
  vUv = uv;
  gl_Position = vec4(position, 1.0);
}
