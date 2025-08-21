// Extend global Navigator type for Web Bluetooth API
declare global {
  interface Navigator {
    bluetooth: {
      requestDevice(options: RequestDeviceOptions): Promise<BluetoothDevice>;
    };
  }
  interface RequestDeviceOptions {
    acceptAllDevices?: boolean;
    filters?: Array<any>;
    optionalServices?: Array<string>;
  }
}
// Type declaration for BluetoothDevice (Web Bluetooth API)
declare interface BluetoothDevice {
  gatt?: BluetoothRemoteGATTServer;
  id: string;
  name?: string;
  // Add more properties as needed
}
declare interface BluetoothRemoteGATTServer {
  disconnect(): void;
}
export type RcCommand = {
  throttle: number; // -1..1 forward/back
  steering: number; // -1..1 left/right
  headlights?: boolean;
  horn?: boolean;
};

export interface Transport {
  name: string;
  connected: boolean;
  connect: () => Promise<void>;
  disconnect: () => Promise<void>;
  send: (cmd: RcCommand) => void;
}

export function createWebSocketTransport(url: string, onStatus?: (s: string) => void): Transport {
  let socket: WebSocket | null = null;
  const t: Transport = {
    name: "WebSocket",
    connected: false,
    async connect() {
      return new Promise<void>((resolve, reject) => {
        try {
          socket = new WebSocket(url);
          onStatus?.("Connecting...");
          socket.addEventListener("open", () => {
            t.connected = true;
            onStatus?.("Connected");
            resolve();
          });
          socket.addEventListener("close", () => {
            t.connected = false;
            onStatus?.("Disconnected");
          });
          socket.addEventListener("error", (e) => {
            onStatus?.("Connection error");
            reject(e);
          });
        } catch (e) {
          reject(e);
        }
      });
    },
    async disconnect() {
      if (socket) {
        socket.close();
        socket = null;
      }
      t.connected = false;
    },
    send(cmd: RcCommand) {
      if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ type: "rc_command", payload: cmd }));
      }
    },
  };
  return t;
}

export function createBluetoothTransport(onStatus?: (s: string) => void): Transport {
  // Note: Generic placeholder implementation; write characteristic is device-specific.
  let device: BluetoothDevice | null = null;
  const t: Transport = {
    name: "Bluetooth",
    connected: false,
    async connect() {
      if (!navigator.bluetooth) {
        throw new Error("Web Bluetooth not supported on this device/browser");
      }
      onStatus?.("Scanning...");
      // Accept all devices. For real hardware, provide filters & services.
      device = await navigator.bluetooth.requestDevice({ acceptAllDevices: true });
      onStatus?.("Device selected");
      t.connected = !!device;
    },
    async disconnect() {
      try {
        await device?.gatt?.disconnect();
      } catch {}
      t.connected = false;
      device = null;
    },
    send(_cmd: RcCommand) {
      // Implement characteristic writes for your device here
      // This placeholder does nothing.
    },
  };
  return t;
}

export function createWiFiTransport(baseUrl: string, onStatus?: (s: string) => void): Transport {
  let normalized = baseUrl.replace(/\/$/, "");
  const t: Transport = {
    name: "Wi-Fi",
    connected: false,
    async connect() {
      // For HTTP-based control there's no persistent connection
      onStatus?.("Ready");
      t.connected = true;
    },
    async disconnect() {
      t.connected = false;
      onStatus?.("Disconnected");
    },
    send(cmd: RcCommand) {
      if (!t.connected) return;
      try {
        // Best-effort, avoid blocking UI and reduce CORS failures visibility
        fetch(`${normalized}/control`, {
          method: "POST",
          mode: "no-cors",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(cmd),
          keepalive: true,
        }).catch(() => {});
      } catch {}
    },
  };
  return t;
}

