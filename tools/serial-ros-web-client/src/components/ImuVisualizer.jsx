import { useRef, Suspense, useMemo } from 'react';
import { Canvas, useFrame, useLoader } from '@react-three/fiber';
import { OrbitControls, PerspectiveCamera, useGLTF, Grid, Center } from '@react-three/drei';
import * as THREE from 'three';

function RobotModel({ imu }) {
  const groupRef = useRef();
  const targetRotation = useRef(new THREE.Euler(0, 0, 0));
  const { scene } = useGLTF('/sme_4wcl.gltf');

  const clonedScene = useMemo(() => {
    const clone = scene.clone(true);
    // Apply a default material if meshes don't have visible ones
    clone.traverse((child) => {
      if (child.isMesh) {
        if (!child.material || child.material.opacity === 0) {
          child.material = new THREE.MeshStandardMaterial({
            color: '#6366f1',
            metalness: 0.5,
            roughness: 0.4,
          });
        }
        child.castShadow = true;
        child.receiveShadow = true;
      }
    });
    return clone;
  }, [scene]);

  useFrame(() => {
    if (!groupRef.current) return;
    if (imu) {
      const r = THREE.MathUtils.degToRad(imu.roll || 0);
      const p = THREE.MathUtils.degToRad(imu.pitch || 0);
      const y = THREE.MathUtils.degToRad(imu.yaw || 0);
      targetRotation.current.set(p, y, r);
    }

    groupRef.current.rotation.x += (targetRotation.current.x - groupRef.current.rotation.x) * 0.08;
    groupRef.current.rotation.y += (targetRotation.current.y - groupRef.current.rotation.y) * 0.08;
    groupRef.current.rotation.z += (targetRotation.current.z - groupRef.current.rotation.z) * 0.08;
  });

  return (
    <group ref={groupRef}>
      <Center precise>
        <group rotation={[-Math.PI / 2, 0, 0]}>
          <primitive object={clonedScene} scale={8} />
        </group>
      </Center>
      <axesHelper args={[0.6]} />
    </group>
  );
}

function LoadingFallback() {
  const meshRef = useRef();
  useFrame((_, delta) => {
    if (meshRef.current) meshRef.current.rotation.y += delta;
  });
  return (
    <mesh ref={meshRef}>
      <boxGeometry args={[0.4, 0.2, 0.3]} />
      <meshStandardMaterial color="#6366f1" wireframe />
    </mesh>
  );
}

export default function ImuVisualizer({ imu }) {
  return (
    <div className="imu-canvas-wrapper">
      <Canvas>
        <PerspectiveCamera makeDefault position={[1.5, 1.0, 1.5]} fov={45} />
        <OrbitControls enableZoom={true} enablePan={true} />
        <ambientLight intensity={0.6} />
        <directionalLight position={[5, 5, 5]} intensity={1.2} />
        <directionalLight position={[-3, 3, -3]} intensity={0.5} color="#818cf8" />
        <pointLight position={[0, -2, 0]} intensity={0.3} color="#22d3ee" />
        <hemisphereLight skyColor="#b1e1ff" groundColor="#444466" intensity={0.4} />
        <Suspense fallback={<LoadingFallback />}>
          <RobotModel imu={imu} />
        </Suspense>
        <Grid
          args={[10, 10]}
          cellSize={0.25}
          cellThickness={0.5}
          cellColor="#1e293b"
          sectionSize={1}
          sectionThickness={1}
          sectionColor="#334155"
          fadeDistance={4}
          fadeStrength={1}
          position={[0, -0.6, 0]}
        />
      </Canvas>
    </div>
  );
}
