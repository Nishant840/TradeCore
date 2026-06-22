const fastify = require('fastify')({ logger: true });
const websocketPlugin = require('@fastify/websocket');

const EngineClient = require('./engineClient')
const { registerRoutes } = require('./routes');
const { insertTrade } = require('./persistence');

const wsClients = new Set();

async function buildServer(){
    await fastify.register(websocketPlugin);

    const engineClient = new EngineClient('localhost', 9000);

    engineClient.on('connected', () => {
        fastify.log.info('Connected to matching engine');
    });

    engineClient.on('disconnected', () => {
        fastify.log.warn('Disconnected from matching engine');
    });

    engineClient.on('message', (msg) => {
        if(msg.event === 'trade'){
            insertTrade(msg);
        }

        const payload = JSON.stringify(msg);

        for(const client of wsClients){
            if(client.readyState === client.OPEN){
                client.send(payload);
            }
        }
    });

    engineClient.connect();

    registerRoutes(fastify, engineClient);

    fastify.get('/ws', {websocket: true}, (connection, request) => {
        const socket = connection.socket || connection;

        wsClients.add(socket);
        fastify.log.info(`WebSocket client connected. Total: ${wsClients.size}`);

        socket.on('close', () => {
            wsClients.delete(socket);
            fastify.log.info(`WebSocket client disconnected. Total: ${wsClients.size}`);
        });
    });

    fastify.get('/health', async () => {
        return {
            status: "ok"
        };
    });

    return fastify;
}

module.exports = { buildServer };