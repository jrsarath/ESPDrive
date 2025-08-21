import React, { useCallback, useEffect, useRef, useState } from "react";
import { cn } from "@/lib/utils";

export type JoystickChange = {
  x: number; // -1..1 (left/right)
  y: number; // -1..1 (forward/back)
};

interface JoystickProps {
  onChange?: (v: JoystickChange) => void;
  onRelease?: () => void;
  size?: number; // px
  axis?: "both" | "x" | "y";
  className?: string;
}

const clamp = (v: number, min: number, max: number) => Math.min(max, Math.max(min, v));

export const Joystick: React.FC<JoystickProps> = ({ onChange, onRelease, size = 240, className, axis = "both" }) => {
  const areaRef = useRef<HTMLDivElement>(null);
  const [pos, setPos] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const pointerIdRef = useRef<number | null>(null);

  const center = useCallback(() => {
    setPos({ x: 0, y: 0 });
    onChange?.({ x: 0, y: 0 });
  }, [onChange]);

  // Normalize position -1..1 within radius
  const computeNormalized = useCallback(
    (clientX: number, clientY: number) => {
      const el = areaRef.current;
      if (!el) return { x: 0, y: 0 };
      const rect = el.getBoundingClientRect();
      const cx = rect.left + rect.width / 2;
      const cy = rect.top + rect.height / 2;
      const dx = clientX - cx;
      const dy = clientY - cy;
      const r = rect.width / 2;
      let nx = clamp(dx / r, -1, 1);
      let ny = clamp(dy / r, -1, 1);
      // clamp to circle
      const mag = Math.hypot(nx, ny);
      if (mag > 1) {
        nx /= mag;
        ny /= mag;
      }
      return { x: nx, y: ny };
    },
    []
  );

  const onPointerDown = (e: React.PointerEvent<HTMLDivElement>) => {
    const el = e.currentTarget;
    el.setPointerCapture(e.pointerId);
    pointerIdRef.current = e.pointerId;
    const n = computeNormalized(e.clientX, e.clientY);
    const n2 = { x: axis === "y" ? 0 : n.x, y: axis === "x" ? 0 : n.y };
    setPos(n2);
    onChange?.(n2);
  };

  const onPointerMove = (e: React.PointerEvent<HTMLDivElement>) => {
    if (pointerIdRef.current !== e.pointerId) return;
    const n = computeNormalized(e.clientX, e.clientY);
    const n2 = { x: axis === "y" ? 0 : n.x, y: axis === "x" ? 0 : n.y };
    setPos(n2);
    onChange?.(n2);
  };

  const onPointerUp = (e: React.PointerEvent<HTMLDivElement>) => {
    if (pointerIdRef.current !== e.pointerId) return;
    pointerIdRef.current = null;
    center();
    onRelease?.();
  };

  // Visual handle position in px
  const handleStyle: React.CSSProperties = {
    transform: `translate(${pos.x * (size / 2 - 28)}px, ${pos.y * (size / 2 - 28)}px)`,
  };

  useEffect(() => {
    return () => {
      pointerIdRef.current = null;
    };
  }, []);

  return (
    <div
      className={cn(
        "select-none",
        className
      )}
      style={{ touchAction: "none" }}
    >
      <div
        ref={areaRef}
        onPointerDown={onPointerDown}
        onPointerMove={onPointerMove}
        onPointerUp={onPointerUp}
        onPointerCancel={onPointerUp}
        className={cn(
          "relative mx-auto rounded-full",
          "bg-gradient-to-br from-background to-muted",
          "border border-border",
          "shadow-sm",
          "animate-enter"
        )}
        style={{ width: size, height: size }}
        aria-label="Joystick control"
        role="slider"
        aria-valuetext={`x ${pos.x.toFixed(2)}, y ${(-pos.y).toFixed(2)}`}
      >
        {/* Rings */}
        <div className="absolute inset-6 rounded-full border border-border/70" />
        <div className="absolute inset-12 rounded-full border border-border/50" />
        <div className="absolute inset-16 rounded-full border border-border/40" />

        {/* Center dot */}
        <div className="absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 size-2 rounded-full bg-foreground/50" />

        {/* Handle */}
        <div
          className={cn(
            "absolute left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2",
            "size-14 rounded-full",
            "bg-primary bg-linear-to-br from-primary-500 to-primary-800 shadow-primary-500/50 ring-primary-900",
            "border border-border",
            "shadow-lg",
            "transition-transform duration-50 will-change-transform",
            "focus:outline-none"
          )}
          style={handleStyle}
        />
      </div>
    </div>
  );
};

export default Joystick;
