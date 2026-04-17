import { WebSocketServer } from 'ws';

const PORT = 3001;
const wss = new WebSocketServer({ port: PORT, host: '0.0.0.0' });

console.log(`\x1b[32m[Robot Relay]\x1b[0m WebSocket server listening on 0.0.0.0:${PORT}`);
console.log(`\x1b[33m[Robot Relay]\x1b[0m Ensuring visibility for mobile/remote devices...`);

// Active master tracking
wss.on('connection', (ws, req) => {
  const remoteIp = req.socket.remoteAddress;
  console.log(`[Link] New device connected: ${remoteIp}`);

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);
      
      // Broadcast to all OTHER clients
      wss.clients.forEach((client) => {
        if (client !== ws && client.readyState === 1) {
          client.send(JSON.stringify(data));
        }
      });
    } catch (e) {
      console.error('[Error] Invalid message format');
    }
  });

  ws.on('close', () => {
    console.log(`[Link] Device disconnected: ${remoteIp}`);
  });
});
