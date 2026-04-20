import { useRef, Suspense, useMemo } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, PerspectiveCamera, useGLTF, Grid, Center } from '@react-three/drei';
import * as THREE from 'three';
import { Target } from 'lucide-react';

function RobotModel({ imu }) {
  const groupRef = useRef();
  const targetRotation = useRef(new THREE.Euler(0, 0, 0));
  const { scene } = useGLTF('/sme_4wcl.gltf');

  const clonedScene = useMemo(() => {
    const clone = scene.clone(true);
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
      // Data is now natively in Radians (ROS standard)
      const r = imu.roll || 0;
      const p = imu.pitch || 0;
      const y = imu.yaw || 0;
      targetRotation.current.set(p, y, r);
    }

    groupRef.current.rotation.x += (targetRotation.current.x - groupRef.current.rotation.x) * 0.80;
    groupRef.current.rotation.y += (targetRotation.current.y - groupRef.current.rotation.y) * 0.80;
    groupRef.current.rotation.z += (targetRotation.current.z - groupRef.current.rotation.z) * 0.80;
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
  const controlsRef = useRef();
  const cameraRef = useRef();

  const handleResetView = () => {
    if (controlsRef.current && cameraRef.current) {
      // Reset camera position
      cameraRef.current.position.set(3.0, 2.0, 3.0);
      // Reset controls target
      controlsRef.current.target.set(0, 0, 0);
      controlsRef.current.update();
    }
  };

  return (
    <div className="imu-canvas-wrapper">
      <button 
        className="reset-view-btn" 
        onClick={handleResetView}
        title="Centrar Cámara"
      >
        <Target size={14} />
      </button>

      <Canvas 
        onCreated={({ gl }) => {
          gl.shadowMap.enabled = true;
          gl.shadowMap.type = THREE.PCFShadowMap;
        }}
      >
        <PerspectiveCamera 
          ref={cameraRef}
          makeDefault 
          position={[3.0, 2.0, 3.0]} 
          fov={45} 
        />
        <OrbitControls 
          ref={controlsRef}
          enableZoom={true} 
          enablePan={true} 
          makeDefault
        />
        
        <ambientLight intensity={0.6} />
        <directionalLight position={[5, 5, 5]} intensity={1.2} castShadow 
          shadow-mapSize-width={1024} 
          shadow-mapSize-height={1024} 
        />
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
