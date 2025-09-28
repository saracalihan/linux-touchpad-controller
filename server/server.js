const net = require("net");
const WebSocket = require("ws");

/**
 * Anlık slot verileri
 */
const slots = [{}, {}, {}, {}, {}];

/**
 * Slotların son dokunulmuş halleri
 */
const oldSlotsLive = [{}, {}, {}, {}, {}];

// USER CONSTANTS
const WIDTH = 1500;
const HEIGHT = 1000;

// SYSTEM CONSTANTS
const DOUBLE_TAP_MS = 500;
const DOUBLE_TAP_AREA = 100;
const LOWER_SECTION_HEIGHT = 100;

const COMMAND_LEFT_CLICK = "1000231";
const COMMAND_RIGHT_CLICK = "1000232";

const Events = {
  doubleTap: "doubleTap",
  rightClick: "rightClick",
}

const tcpClient = net.createConnection({ host: "127.0.0.1", port: 8081 }, () => {
  console.log("TCP sunucusuna bağlandı (8080)");
});

const wss = new WebSocket.Server({ port: 9090 }, () => {
  console.log("WebSocket sunucusu 9090 portunda çalışıyor");
});

tcpClient.on("data", (data) => {
  const frame = data.toString();

  try {
    const index = parseInt(frame.slice(0, 1), 10);
    const id = parseInt(frame.slice(1, 6), 10);
    const x = parseInt(frame.slice(6, 10), 10);
    const y = parseInt(frame.slice(10, 14), 10);
    const time = parseInt(frame.slice(14), 10);

    // reverse
    const obj = { index, id, y: x, x: y, time };

    // console.log(obj);

    if (obj.id != -1) {
      oldSlotsLive[obj.index] = slots[obj.index];
    }
    slots[obj.index] = obj;
    const json = JSON.stringify(obj);
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(json);
      }
    });
    runEvents(obj);
  } catch (err) {
    console.error("Veri parse hatası:", err.message);
  }
});

const runEvents = (slot) => {
  let oldSlot = oldSlotsLive[slot.index];

  if (oldSlot) {
    if (slot.id != -1) {
      // double tap
      if (Conditions.doubleTap(slot, oldSlot)) {
        // right click ( double tap on righ down corder )
        if(Conditions.onBottom(slot) && Conditions.onRight(slot)){
          tcpClient.write(COMMAND_RIGHT_CLICK);
          console.log("[COMMAND]: right click");
        } else {
          tcpClient.write(COMMAND_LEFT_CLICK);
          console.log("[COMMAND]: double tap");
        }
      }
    }
  }
}

const Conditions = {
  doubleTap(slot, oldSlot){ return oldSlot.id != slot.id && Math.abs(slot.x - oldSlot.x) < DOUBLE_TAP_AREA && Math.abs(slot.y - oldSlot.y) < 100 && slot.time - oldSlot.time <= DOUBLE_TAP_MS },
  onBottom: (slot) => slot.y >= HEIGHT - LOWER_SECTION_HEIGHT,
  onRight: (slot) => slot.x >= WIDTH / 2,
  onLeft: (slot) => slot.x < WIDTH / 2,
}

tcpClient.on("error", (err) => {
  console.error("TCP hatası:", err);
});

tcpClient.on("close", () => {
  console.log("TCP bağlantısı kapandı, tekrar bağlanmayı deneyebilirsin...");
  setTimeout(connectTcp, 2000);
});

wss.on("connection", (ws) => {
  console.log("Yeni WebSocket client bağlandı");
  ws.send(JSON.stringify({ msg: "WebSocket'e hoş geldin!" }));
});
