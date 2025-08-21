import { useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import { Card, CardContent } from "@/components/ui/card";
import { Bluetooth, Lightbulb, Volume2, Power, Wifi } from "lucide-react";
import Joystick from "@/components/controls/JoyStick";
import { createBluetoothTransport, Transport } from "@/lib/transport";
import { toast } from "sonner";
import { WEBSOCKET_URL } from "@/constants";

const Index = () => {
  const lastCmdRef = useRef<{ throttle: number; steering: number }>({ throttle: 0, steering: 0 });

  const [ws, setWs] = useState<WebSocket | null>(null);
  const [transport, setTransport] = useState<Transport | null>(null);
  const [connectionMode, setConnectionMode] = useState<"websocket" | "bluetooth">("websocket");
  const [status, setStatus] = useState("Disconnected");
  const [headlights, setHeadlights] = useState(false);
  const [horn, setHorn] = useState(false);
  const [throttle, setThrottle] = useState(0);
  const [steering, setSteering] = useState(0);

  const isConnected = status === "Connected";

  useEffect(() => {
    document.title = "ESPDrive – Sport Mode";

    let reconnectTimer: NodeJS.Timeout | null = null;
    let socket: WebSocket | null = null;

    const connect = () => {
      socket = new WebSocket(WEBSOCKET_URL);

      socket.onopen = () => {
        setConnectionMode("websocket");
        setStatus("Connected");
        setWs(socket);
        toast("WebSocket Connected");
        if (reconnectTimer) {
          clearTimeout(reconnectTimer);
          reconnectTimer = null;
        }
      };

      socket.onclose = () => {
        setStatus("Disconnected");
        setWs(null);
        toast("WebSocket Disconnected");
        scheduleReconnect();
      };

      socket.onerror = () => {
        setStatus("Disconnected");
        setWs(null);
        toast.error("WebSocket Error");
        socket?.close(); // force close to trigger onclose → reconnect
      };

      socket.onmessage = (event) => {
        console.log(event.data);
        // handle messages here
      };
    };

    const scheduleReconnect = () => {
      if (!reconnectTimer) {
        reconnectTimer = setTimeout(() => {
          console.log("Reconnecting WebSocket...");
          connect();
        }, 3000); // retry after 3s
      }
    };

    connect();

    return () => {
      if (reconnectTimer) clearTimeout(reconnectTimer);
      socket?.close();
    };
  }, []);
  useEffect(() => {
    // Send commands at 20Hz
    const interval = setInterval(() => {
      if (!transport?.connected) return;
      const { throttle, steering } = lastCmdRef.current;
      transport.send({ throttle, steering, headlights, horn });
    }, 50);
    return () => clearInterval(interval);
  }, [transport, headlights, horn]);

  const connect = async () => {
    try {
      const t = createBluetoothTransport(setStatus);
      await t.connect();
      setTransport(t);
      setStatus("Connected");
      toast("Connected", { description: "Bluetooth device ready" });
    } catch (e: any) {
      setStatus("Disconnected");
      toast.error("Connection failed", { description: e?.message ?? String(e) });
    }
  };
  const disconnect = async () => {
    try {
      await transport?.disconnect();
      setTransport(null);
      setStatus("Disconnected");
      toast("Disconnected", { description: "Device disconnected" });
    } catch {}
  };
  const sendControlCommand = (cmd: string) => {
    if (connectionMode === "websocket" && ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ type: "control", command: cmd }));
    } else if (connectionMode === "bluetooth" && transport?.connected) {
      // You can expand this for actual command structure
      transport.send({ throttle: 0, steering: 0, headlights, horn });
    }
  };
  const handleJoystickChange = ({ x, y }: { x: number; y: number }) => {
    const s = Math.max(-1, Math.min(1, x));
    const t = Math.max(-1, Math.min(1, -y));
    setSteering(s);
    setThrottle(t);
    lastCmdRef.current.steering = s;
    lastCmdRef.current.throttle = t;

    // Decide command
    let throttleCmd = "stop";
    if (t > 0.3) throttleCmd = "fwd";
    else if (t < -0.3) throttleCmd = "rev";

    let steeringCmd = "center";
    if (s > 0.3) steeringCmd = "right";
    else if (s < -0.3) steeringCmd = "left";

    sendControlCommand(throttleCmd);
    sendControlCommand(steeringCmd);
  };


  return (
    <div className="min-h-screen bg-gradient-to-b from-background to-primary/10 flex flex-col">
      <header className="px-6 pt-6">
        <div className="max-w-4xl mx-auto flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold tracking-tight bg-gradient-to-r from-foreground to-foreground/60 bg-clip-text text-transparent">
              ESPDrive
            </h1>
            <p className="text-sm text-muted-foreground">Sport mode</p>
          </div>
          <div
            className={`flex items-center gap-2 px-3 py-1.5 rounded-full text-sm font-medium border ${
              isConnected
                ? "text-green-500 border-green-500/30 bg-green-500/10"
                : "text-destructive border-destructive/30 bg-destructive/10"
            }`}
            aria-live="polite"
          >
            <span
              className={`inline-block size-2 rounded-full ${
                isConnected ? "bg-green-500" : "bg-destructive"
              }`}
            />
            {status}
          </div>
        </div>
      </header>

      <main className="flex flex-col px-6 py-10 grow">
        <div className="flex flex-col grow space-y-6">
          {/* Connection Panel */}
          <section className="hidden">
            <Card className="border-0 shadow-lg bg-card/60 backdrop-blur-sm">
              <CardContent className="flex items-center justify-between">
                <div className="flex items-center gap-3">
                  <div className="p-2 rounded-lg bg-primary/10">
                    {connectionMode === "bluetooth" && <Bluetooth className="w-5 h-5 text-primary" />}
                    {connectionMode === "websocket" && <Wifi className="w-5 h-5 text-primary" />}
                  </div>
                  <div>
                    <h2 className="font-semibold leading-none capitalize">{connectionMode} Connection</h2>
                    <p className="text-sm text-muted-foreground">{isConnected ? 'Connected' : 'Connect'} to your RC device</p>
                  </div>
                </div>
                {!isConnected ? (
                  <Button onClick={connect} className="gap-2">
                    <Power className="w-4 h-4" /> Connect
                  </Button>
                ) : (
                  <Button onClick={disconnect} variant="outline" className="gap-2">
                    <Power className="w-4 h-4" /> Disconnect
                  </Button>
                )}
              </CardContent>
            </Card>
          </section>
          {/* Drive Controls - Joystick only */}
          <section className="flex flex-row grow">
            <Card className="grow border-0 shadow-xl bg-card/70 backdrop-blur-sm">
              <CardContent className="h-full">
                <div className="h-full grid grid-cols-1 place-items-center gap-6">
                  <div className="text-center space-y-1">
                    <h2 className="text-xl font-semibold">Sport Joystick</h2>
                    <p className="text-sm text-muted-foreground">Steer and throttle with a single control</p>
                  </div>
                  {/* Central Joystick */}
                  <div className="animate-enter">
                    <Joystick
                      size={250}
                      onChange={handleJoystickChange}
                    />
                  </div>
                  {/* Live readouts */}
                  <div className="flex items-center gap-4 text-sm">
                    <div className="px-3 py-1 rounded-md border bg-secondary/50">
                      <span className="text-muted-foreground">Throttle:</span>{" "}
                      <span className="font-mono">{throttle.toFixed(2)}</span>
                    </div>
                    <div className="px-3 py-1 rounded-md border bg-secondary/50">
                      <span className="text-muted-foreground">Steering:</span>{" "}
                      <span className="font-mono">{steering.toFixed(2)}</span>
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>
          </section>
          {/* Aux controls */}
          <section>
            <Card className="border-0 shadow-lg bg-card/60 backdrop-blur-sm">
              <CardContent>
                <div className="flex flex-row items-center justify-center gap-6">
                  <Button
                    size="lg"
                    variant={headlights ? "default" : "outline"}
                    className="w-1/2 gap-2 px-6 cursor-pointer"
                    onClick={() => setHeadlights(!headlights)}
                  >
                    <Lightbulb className="w-5 h-5 text-foreground" aria-hidden="true" />
                    Headlights
                  </Button>

                  <Button
                    size="lg"
                    variant={horn ? "default" : "outline"}
                    className="w-1/2 gap-2 px-6 cursor-pointer"
                    onMouseDown={() => setHorn(true)}
                    onMouseUp={() => setHorn(false)}
                    onTouchStart={() => setHorn(true)}
                    onTouchEnd={() => setHorn(false)}
                    aria-pressed={horn}
                    aria-label="Press and hold for horn"
                  >
                    <Volume2 className="w-5 h-5" /> Horn
                  </Button>
                </div>
              </CardContent>
            </Card>
          </section>
        </div>
      </main>
    </div>
  );
};
export default Index;
