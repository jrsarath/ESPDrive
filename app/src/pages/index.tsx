import { useEffect, useRef, useState } from "react";
import { Button } from "@/components/ui/button";
import { Card, CardContent } from "@/components/ui/card";
import { Switch } from "@/components/ui/switch";
import { Bluetooth, Lightbulb, Volume2, Power } from "lucide-react";
import Joystick from "@/components/controls/JoyStick";
import { createBluetoothTransport, Transport } from "@/lib/transport";
import { toast } from "sonner";

const Index = () => {
  const lastCmdRef = useRef<{ throttle: number; steering: number }>({ throttle: 0, steering: 0 });

  const [transport, setTransport] = useState<Transport | null>(null);
  const [status, setStatus] = useState("Disconnected");
  const [headlights, setHeadlights] = useState(false);
  const [horn, setHorn] = useState(false);
  const [throttle, setThrottle] = useState(0);
  const [steering, setSteering] = useState(0);

  const isConnected = status === "Connected";

  useEffect(() => {
    document.title = "ESP Drive – Sport Mode";
  }, []);
  // Send commands at 20Hz
  useEffect(() => {
    const interval = setInterval(() => {
      if (!transport?.connected) return;
      const { throttle, steering } = lastCmdRef.current;
      transport.send({ throttle, steering, headlights, horn });
    }, 50);
    return () => clearInterval(interval);
  }, [transport, headlights, horn]);

  const connectBluetooth = async () => {
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


  return (
    <div className="min-h-screen bg-gradient-to-b from-background to-primary/10 flex flex-col">
      <header className="px-6 pt-6">
        <div className="max-w-4xl mx-auto flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold tracking-tight bg-gradient-to-r from-foreground to-foreground/60 bg-clip-text text-transparent">
              RC Controller
            </h1>
            <p className="text-sm text-muted-foreground">Sport mode</p>
          </div>

          <div
            className={`flex items-center gap-2 px-3 py-1.5 rounded-full text-sm font-medium border ${
              isConnected
                ? "text-primary border-primary/30 bg-primary/10"
                : "text-destructive border-destructive/30 bg-destructive/10"
            }`}
            aria-live="polite"
          >
            <span
              className={`inline-block size-2 rounded-full ${
                isConnected ? "bg-primary" : "bg-destructive"
              }`}
            />
            {status}
          </div>
        </div>
      </header>

      <main className="flex flex-col px-6 py-10 grow">
        <div className="flex flex-col grow space-y-6">
          {/* Connection Panel */}
          <section>
            <Card className="border-0 shadow-lg bg-card/60 backdrop-blur-sm">
              <CardContent className="flex items-center justify-between">
                <div className="flex items-center gap-3">
                  <div className="p-2 rounded-lg bg-primary/10">
                    <Bluetooth className="w-5 h-5 text-primary" />
                  </div>
                  <div>
                    <h2 className="font-semibold leading-none">Bluetooth Connection</h2>
                    <p className="text-sm text-muted-foreground">Connect to your RC device</p>
                  </div>
                </div>
                {!isConnected ? (
                  <Button onClick={connectBluetooth} className="gap-2">
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
                      size={220}
                      onChange={({ x, y }) => {
                        const s = Math.max(-1, Math.min(1, x));
                        const t = Math.max(-1, Math.min(1, -y));
                        setSteering(s);
                        setThrottle(t);
                        lastCmdRef.current.steering = s;
                        lastCmdRef.current.throttle = t;
                      }}
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
                    <Lightbulb className={`w-5 h-5 ${headlights ? "text-foreground" : "text-primary"}`} aria-hidden="true" />
                    <div className="flex flex-row grow justify-between">
                      <span className="font-medium text-sm">Headlights</span>
                      <Switch checked={headlights} onCheckedChange={setHeadlights} aria-label="Toggle headlights" />
                    </div>
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
