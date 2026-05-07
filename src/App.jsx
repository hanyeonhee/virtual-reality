import { Suspense, useRef } from 'react'
import { Canvas, useFrame } from '@react-three/fiber'
import { OrbitControls, Environment, Grid } from '@react-three/drei'
import { EffectComposer } from '@react-three/postprocessing'
import * as THREE from 'three'
import VolumetricFog from './components/VolumetricFog'
import { useFogControls } from './hooks/useFogControls'

// ─── Demo scene geometry ─────────────────────────────────────────────────────

function Trees() {
  const positions = [
    [-8, 0, -12], [-3, 0, -18], [5, 0, -10], [12, 0, -20],
    [-15, 0, -8], [18, 0, -15], [-20, 0, -25], [8, 0, -30],
    [-5, 0, -35], [22, 0, -5],  [-25, 0, -18], [15, 0, -8],
  ]

  return (
    <group>
      {positions.map(([x, y, z], i) => {
        const scale = 0.7 + Math.sin(i * 1.7) * 0.4
        const trunkH = 2 * scale
        const coneH  = 5 * scale
        return (
          <group key={i} position={[x, y, z]}>
            {/* trunk */}
            <mesh position={[0, trunkH / 2, 0]}>
              <cylinderGeometry args={[0.15 * scale, 0.25 * scale, trunkH, 6]} />
              <meshStandardMaterial color="#5c3d1e" roughness={0.9} />
            </mesh>
            {/* canopy */}
            <mesh position={[0, trunkH + coneH / 2, 0]}>
              <coneGeometry args={[1.5 * scale, coneH, 7]} />
              <meshStandardMaterial color="#2d6a3f" roughness={0.8} />
            </mesh>
          </group>
        )
      })}
    </group>
  )
}

function Terrain() {
  return (
    <>
      {/* Ground plane */}
      <mesh rotation={[-Math.PI / 2, 0, 0]} receiveShadow>
        <planeGeometry args={[200, 200, 32, 32]} />
        <meshStandardMaterial color="#4a7c3f" roughness={1} />
      </mesh>

      {/* Rolling hills */}
      {[
        { pos: [-30, -1, -40], scale: [40, 8, 30] },
        { pos: [25, -2, -50],  scale: [50, 12, 35] },
        { pos: [-10, -3, -70], scale: [60, 10, 40] },
      ].map(({ pos, scale }, i) => (
        <mesh key={i} position={pos} scale={scale} receiveShadow>
          <sphereGeometry args={[1, 16, 8, 0, Math.PI * 2, 0, Math.PI / 2]} />
          <meshStandardMaterial color="#3d6b35" roughness={1} />
        </mesh>
      ))}

      {/* Rocky outcrops */}
      {[[-12, 0, -22], [20, 0, -18], [-5, 0, -45]].map(([x, y, z], i) => (
        <mesh key={i} position={[x, y + 1.5, z]} rotation={[0, i * 1.2, 0]} castShadow>
          <dodecahedronGeometry args={[2.5 + i * 0.5]} />
          <meshStandardMaterial color="#7a7060" roughness={0.95} />
        </mesh>
      ))}
    </>
  )
}

function FloatingOrbs() {
  const groupRef = useRef()
  useFrame(({ clock }) => {
    const t = clock.getElapsedTime()
    if (groupRef.current) {
      groupRef.current.children.forEach((c, i) => {
        c.position.y = Math.sin(t * 0.5 + i * 1.3) * 0.8 + 3 + i * 2
      })
    }
  })

  return (
    <group ref={groupRef}>
      {[
        { pos: [3, 3, -8],    color: '#88aaff', emissive: '#3355cc' },
        { pos: [-6, 5, -15],  color: '#ffaa44', emissive: '#cc6600' },
        { pos: [10, 7, -12],  color: '#aaffaa', emissive: '#33aa33' },
        { pos: [-2, 9, -25],  color: '#ffaaff', emissive: '#aa33aa' },
      ].map(({ pos, color, emissive }, i) => (
        <mesh key={i} position={pos} castShadow>
          <sphereGeometry args={[0.6, 16, 16]} />
          <meshStandardMaterial color={color} emissive={emissive} emissiveIntensity={0.5} />
        </mesh>
      ))}
    </group>
  )
}

// ─── Main scene ──────────────────────────────────────────────────────────────

function Scene() {
  const controls = useFogControls()

  return (
    <>
      {/* Lighting */}
      <ambientLight intensity={0.4} color="#c0d8f0" />
      <directionalLight
        position={[
          controls.sunDirection[0] * 50,
          controls.sunDirection[1] * 50,
          controls.sunDirection[2] * 50,
        ]}
        intensity={1.8}
        color={controls.sunColor}
        castShadow
        shadow-mapSize={[2048, 2048]}
      />

      {/* Geometry */}
      <Terrain />
      <Trees />
      <FloatingOrbs />

      {/* Post-processing */}
      {controls.enabled && (
        <EffectComposer>
          <VolumetricFog
            fogColor={controls.fogColor}
            fogDensity={controls.fogDensity}
            fogHeight={controls.fogHeight}
            fogHeightFalloff={controls.fogHeightFalloff}
            fogStart={controls.fogStart}
            marchSteps={controls.marchSteps}
            marchMaxDistance={controls.marchMaxDistance}
            inscatteringColor={controls.inscatteringColor}
            inscatteringStrength={controls.inscatteringStrength}
            sunDirection={controls.sunDirection}
            sunColor={controls.sunColor}
            mieG={controls.mieG}
            ambientStrength={controls.ambientStrength}
            noiseStrength={controls.noiseStrength}
            noiseTiling={controls.noiseTiling}
          />
        </EffectComposer>
      )}

      <OrbitControls
        target={[0, 2, -15]}
        minDistance={2}
        maxDistance={150}
        maxPolarAngle={Math.PI / 2 - 0.05}
      />
    </>
  )
}

// ─── App root ────────────────────────────────────────────────────────────────

export default function App() {
  return (
    <Canvas
      camera={{ position: [0, 8, 20], fov: 60, near: 0.1, far: 1000 }}
      shadows
      gl={{ antialias: true }}
      style={{ background: '#c8d8e8' }}
    >
      <Suspense fallback={null}>
        <Scene />
      </Suspense>
    </Canvas>
  )
}
