import { useState, useEffect, useRef } from "react";
import { invoke } from "@tauri-apps/api/core";
import { listen } from "@tauri-apps/api/event";
import "./App.css";

interface TouchData {
  index: number | null;
  id: number;
  x: number;
  y: number;
  time: number;
}

const CANVAS_WIDTH = 1500;
const CANVAS_HEIGHT = 1000;
const LOWER_SECTION_HEIGHT = 200;
const SLOTS_COUNT = 5;

function App() {
  const [connectionStatus, setConnectionStatus] = useState(false);
  const [touchSlots, setTouchSlots] = useState<(TouchData | null)[]>([null, null, null, null, null]);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const lastDrawTime = useRef<number>(0);
  const animationFrameId = useRef<number | null>(null);

  // Canvas çizim fonksiyonu
  const drawCanvas = () => {

    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Canvas'ı temizle
    ctx.fillStyle = '#2c2c2cff';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // bottom line
    const lowerSectionY = canvas.height - canvas.height * LOWER_SECTION_HEIGHT / CANVAS_HEIGHT;
    ctx.beginPath();
    ctx.setLineDash([5, 8]);
    ctx.moveTo(0, lowerSectionY);
    ctx.lineTo(canvas.width, lowerSectionY);
    ctx.stroke();
    ctx.beginPath();

    ctx.setLineDash([5, 8]);
    ctx.moveTo(canvas.width / 2, lowerSectionY);
    ctx.lineTo(canvas.width / 2, canvas.height);
    ctx.stroke();

    const radius = canvas.width / 30;
    // Her slot için çizim yap
    for (let i = 0; i < SLOTS_COUNT; i++) {
      const item = touchSlots[i];

      if (item && item.id > 0) {
        const cx = item.x / CANVAS_WIDTH * canvas.width;
        const cy = item.y / CANVAS_HEIGHT * canvas.height;
        // Ana daire
        ctx.beginPath();
        ctx.arc(cx, cy, radius, 0, Math.PI * 2);
        ctx.fillStyle = 'red';
        ctx.fill();

        // // Yazılar
        ctx.fillStyle = 'white';
        ctx.textAlign = 'center';
        ctx.font = 'bold 8px system-ui';
        ctx.fillText(`${item.index}`, cx, cy + 2);
      }
    }
  };

  // Throttled draw function
  const throttledDraw = () => {
    const now = Date.now();
    const timeSinceLastDraw = now - lastDrawTime.current;

    // Minimum 16ms between draws (~60fps)
    if (timeSinceLastDraw >= 16) {
      if (animationFrameId.current) {
        cancelAnimationFrame(animationFrameId.current);
      }

      animationFrameId.current = requestAnimationFrame(() => {
        drawCanvas();
        lastDrawTime.current = Date.now();
        animationFrameId.current = null;
      });
    }
  };

  // TCP bağlantısını başlat
  const startConnection = async () => {
    try {
      await listen<boolean>("tcp-connected", () => {
        setConnectionStatus(true);
      });
      await invoke("start_tcp_connection");
    } catch (error) {
      console.error("Bağlantı hatası:", error);
      setConnectionStatus(false);
    }
  };

  useEffect(() => {
    throttledDraw();
  }, [touchSlots]);

  useEffect(() => {
    const setupListeners = async () => {
      await listen<string>("tcp-data", (event) => {
        const frame = event.payload;
        const index = parseInt(frame.slice(0, 1), 10);
        const id = parseInt(frame.slice(1, 6), 10);
        const x = parseInt(frame.slice(6, 10), 10);
        const y = parseInt(frame.slice(10, 14), 10);
        const time = parseInt(frame.slice(14), 10);
        let data: TouchData = {
          id,
          x,
          y,
          time,
          index,
        }
        if (data.index !== null && data.index >= 0 && data.index < 5) {
          setTouchSlots(prev => {
            const newSlots = [...prev];
            newSlots[data.index!] = data;
            return newSlots;
          });
        }
      });

      await listen<string>("tcp-error", (event) => {
        console.log("TCP Error:", event.payload);
        setConnectionStatus(false);
      });
    };

    setupListeners();
    startConnection();
  }, []);

  return (
    <main className="container-fluid p-4" style={{ height: '100dvh' }}>
      <div className="row">
        <div className="col d-flex justify-content-center" style={{
        }}>

          {
            connectionStatus ? <canvas
              ref={canvasRef}
              style={{
                maxWidth: CANVAS_WIDTH / 2,
                aspectRatio: CANVAS_WIDTH / CANVAS_HEIGHT,
                color: 'inherit',
                border: '2px solid var(--color)',
                display: 'block',
                width: '100%',
                height: '100%',
              }}
            /> : <button onClick={startConnection} style={{}}>
              Baglantı Kur
            </button>
          }
        </div>
      </div>

      {/* <h3>Son Mesaj / Seçili Slot</h3>
      <div style={{
        display: 'grid',
        gridTemplateColumns: 'repeat(5, 1fr)',
        gap: '16px',
        marginTop: '20px'
      }}>
        {touchSlots.map((slot, index) => (
          <div
            key={index}
            style={{
              backgroundColor: '#ddd',
              aspectRatio: '1/1',
              padding: '8px',
              borderRadius: '6px',
              maxHeight: '200px',
              overflow: 'auto',
              fontSize: '12px',
              fontFamily: 'monospace'
            }}
          >
            {slot && slot.id > 0 ? (
              <pre>{JSON.stringify(slot, null, 2)}</pre>
            ) : (
              <div style={{ color: '#666' }}>Slot {index}</div>
            )}
          </div>
        ))}
      </div> */}
    </main>
  );
}

export default App;
