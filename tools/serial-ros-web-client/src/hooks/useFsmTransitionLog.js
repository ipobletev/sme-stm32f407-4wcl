import { useCallback, useEffect, useRef, useState } from 'react';

const MAX_ROWS = 500;

function normErrors(v) {
  if (v === null || v === undefined) return 0n;
  return typeof v === 'bigint' ? v : BigInt(v);
}

/**
 * Records one log row whenever supervisor / mobility / arm state or error mask changes.
 * @param {object | null} sysStatus parsed 0x81 payload
 */
export function useFsmTransitionLog(sysStatus) {
  const [rows, setRows] = useState([]);
  const prevRef = useRef(null);

  useEffect(() => {
    if (!sysStatus) {
      prevRef.current = null;
      return;
    }

    const snap = {
      state: sysStatus.state ?? 0,
      mobility_state: sysStatus.mobility_state ?? 0,
      arm_state: sysStatus.arm_state ?? 0,
      errors: normErrors(sysStatus.errors),
    };

    const prev = prevRef.current;

    if (!prev) {
      prevRef.current = snap;
      const row = {
        id: `${performance.now()}-${Math.random().toString(36).slice(2, 9)}`,
        ts: Date.now(),
        kind: 'initial',
        sup: { from: snap.state, to: snap.state },
        mob: { from: snap.mobility_state, to: snap.mobility_state },
        arm: { from: snap.arm_state, to: snap.arm_state },
        errFrom: snap.errors,
        errTo: snap.errors,
      };
      setRows((r) => [row, ...r].slice(0, MAX_ROWS));
      return;
    }

    prevRef.current = snap;

    if (
      prev.state === snap.state &&
      prev.mobility_state === snap.mobility_state &&
      prev.arm_state === snap.arm_state &&
      prev.errors === snap.errors
    ) {
      return;
    }

    const row = {
      id: `${performance.now()}-${Math.random().toString(36).slice(2, 9)}`,
      ts: Date.now(),
      sup: { from: prev.state, to: snap.state },
      mob: { from: prev.mobility_state, to: snap.mobility_state },
      arm: { from: prev.arm_state, to: snap.arm_state },
      errFrom: prev.errors,
      errTo: snap.errors,
    };

    setRows((r) => [row, ...r].slice(0, MAX_ROWS));
  }, [sysStatus]);

  const clear = useCallback(() => {
    if (sysStatus) {
      prevRef.current = {
        state: sysStatus.state ?? 0,
        mobility_state: sysStatus.mobility_state ?? 0,
        arm_state: sysStatus.arm_state ?? 0,
        errors: normErrors(sysStatus.errors),
      };
    } else {
      prevRef.current = null;
    }
    setRows([]);
  }, [sysStatus]);

  return { rows, clear };
}
