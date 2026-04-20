import { useState, useEffect, useRef } from 'react';

/**
 * Hook to maintain a historical buffer of telemetry data for charting.
 */
export function useTelemetryHistory(telemetry, frequencies = {}, maxPoints = 50) {
  const [history, setHistory] = useState([]);
  const lastUpdateRef = useRef(0);

  useEffect(() => {
    // Throttling to avoid excessive React renders (update approx 5Hz for graphs)
    const now = Date.now();
    if (now - lastUpdateRef.current < 200) return;
    lastUpdateRef.current = now;

    const { sysStatus, imu, odometry, pidDebug } = telemetry;
    
    // We only add a point if we have at least some data
    if (!sysStatus && !imu && !odometry && !pidDebug) return;

    const newPoint = {
      timestamp: now,
      timeLabel: new Date(now).toLocaleTimeString([], { hour12: false, minute: '2-digit', second: '2-digit' }),
      
      // System Status
      battery: sysStatus?.v_batt || 0,
      temp: sysStatus?.temp || 0,
      
      // IMU
      roll: imu?.roll || 0,
      pitch: imu?.pitch || 0,
      yaw: imu?.yaw || 0,
      ax: imu?.accel?.x || 0,
      ay: imu?.accel?.y || 0,
      az: imu?.accel?.z || 0,
      gx: imu?.gyro?.x || 0,
      gy: imu?.gyro?.y || 0,
      gz: imu?.gyro?.z || 0,
      
      // Odometry
      vx: odometry?.linear_x || 0,
      wz: odometry?.angular_z || 0,
      rps1: odometry?.rps_1 || 0,
      rps2: odometry?.rps_2 || 0,
      rps3: odometry?.rps_3 || 0,
      rps4: odometry?.rps_4 || 0,
      enc1: odometry?.encoders?.[0] || 0,
      enc2: odometry?.encoders?.[1] || 0,
      enc3: odometry?.encoders?.[2] || 0,
      enc4: odometry?.encoders?.[3] || 0,
      
      // PID Debug
      pid_target:   pidDebug?.targetRps   || [0,0,0,0],
      pid_measured: pidDebug?.measuredRps || [0,0,0,0],
      pid_pwm:      pidDebug?.pwmOutput   || [0,0,0,0],

      // Frequencies (Hz)
      freq_sys: frequencies?.['0x81'] || 0,
      freq_imu: frequencies?.['0x82'] || 0,
      freq_odom: frequencies?.['0x83'] || 0,
    };

    setHistory(prev => {
      const updated = [...prev, newPoint];
      if (updated.length > maxPoints) {
        return updated.slice(updated.length - maxPoints);
      }
      return updated;
    });
  }, [telemetry, frequencies, maxPoints]);

  return history;
}
