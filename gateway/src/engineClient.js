const net = require('net')
const EventEmitter = require('events')

class EngineClient extends EventEmitter {
    constructor(host, port){
        super();
        this.host   = host;
        this.port   = port;
        this.socket = null;
        this.buffer = '';
        this.pendingRequests = new Map();
    }

    connect() {
        this.socket = net.createConnection(this.port, this.host, ()=>{
            console.log(`Connected to engine at ${this.host}:${this.port}`);
            this.emit('connected');
        });

        this.socket.on('data', (chunk) => {
            this.buffer += chunk.toString();

            let newlineIndex;
            while((newlineIndex = this.buffer.indexOf('\n')) !== -1){
                const line  = this.buffer.slice(0,newlineIndex);
                this.buffer = this.buffer.slice(newlineIndex+1);

                if(line.length === 0) continue;

                try{
                    const message = JSON.parse(line);
                    this._handleMessage(message);
                }
                catch (err){
                    console.error('Failed to parse engine message:', err.message);
                }
            }
        });

        this.socket.on('error', (err) => {
            console.error('Engine socket error:', err.message);
        });

        this.socket.on('close', ()=>{
            console.log('Engine connection closed');
            this.emit('disconnected');
        });
    }

    _handleMessage(message){
        if(message.requestId && this.pendingRequests.has(message.requestId)){
            const { resolve, timeoutHandle } = this.pendingRequests.get(message.requestId);
            clearTimeout(timeoutHandle);
            this.pendingRequests.delete(message.requestId);
            resolve(message);
            return;
        }

        this.emit('message', message);
    }

    sendOrder(order){
        const message = {
            cmd: 'new_order',
            orderId: order.orderId,
            userId: order.userId,
            side: order.side,
            type: order.type,
            price: order.price,
            quantity: order.quantity,
        };
        this._send(message);
    }

    sendCancel(orderId){
        const message = {
            cmd: 'cancel',
            orderId: orderId,
        };
        this._send(message);
    }

    requestBook(requestId, depth = 10){
        return new Promise((resolve, reject) => {
            const timeoutHandle = setTimeout(()=>{
                this.pendingRequests.delete(requestId);
                reject(new Error("Engine did not respond in time"));
            }, 2000);
            
            this.pendingRequests.set(requestId, {resolve, timeoutHandle});

            try{
                this._send({
                    cmd: 'get_book',
                    requestId,
                    depth
                });
            }
            catch(err){
                clearTimeout(timeoutHandle);
                this.pendingRequests.delete(requestId);
                reject(err);
            }
        });
    }

    requestMetrics(requestId) {
        return new Promise((resolve, reject) => {
            const timeoutHandle = setTimeout(() => {
                this.pendingRequests.delete(requestId);
                reject(new Error('Engine did not respond in time'));
            }, 2000);

            this.pendingRequests.set(requestId, { resolve, timeoutHandle });

            try {
                this._send({
                    cmd: 'get_metrics',
                    requestId 
                });
            }
            catch(err){
                clearTimeout(timeoutHandle);
                this.pendingRequests.delete(requestId);
                reject(err);
            }
        });
    }

    _send(message){
        if(!this.socket || this.socket.destroyed) {
            throw new Error('Engine socket not connected');
        }
        const line = JSON.stringify(message) + '\n';
        this.socket.write(line);
    }
}

module.exports = EngineClient;