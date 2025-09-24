const net = require("net");
const WebSocket = require("ws");

const tcpClient = net.createConnection({ host: "127.0.0.1", port: 8080 }, () => {
  console.log("TCP sunucusuna bağlandı (8080)");
});

const wss = new WebSocket.Server({ port: 9090 }, () => {
  console.log("WebSocket sunucusu 9090 portunda çalışıyor");
});

tcpClient.on("data", (data) => {
  const frame = data.toString().trim();

  try {
    const index = parseInt(frame.slice(0, 1), 10);
    const id = parseInt(frame.slice(1, 6), 10);
    const x = parseInt(frame.slice(6, 10), 10);
    const y = parseInt(frame.slice(10, 14), 10);
    const time = parseInt(frame.slice(14), 10);

    // reverse 
    const obj = { index, id, y:x, x:y, time };
    console.log(obj);

    const json = JSON.stringify(obj);
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(json);
      }
    });
  } catch (err) {
    console.error("Veri parse hatası:", err.message);
  }
});

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
