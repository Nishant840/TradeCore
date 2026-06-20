const { nextOrderId } = require("./orderIdGenerator")

function registerRoutes(fastify, engineClient){
    fastify.post('/order', async (request, reply) => {
        const { userId, side, type, price, quantity } = request.body;

        if(!userId || !side || !type || !quantity){
            return reply.code(400).send({
                error: "Missing required fields"
            });
        }

        if(side !== "BUY" && side !== "SELL"){
            return reply.code(400).send({
                error: "side must be BUY or SELL"
            });
        }

        if(type !== "LIMIT" && type !== "MARKET"){
            return reply.code(400).send({
                error: "type must be LIMIT or MARKET"
            });
        }

        if(type === "LIMIT" && (price === undefined || price <= 0)){
            return reply.code(400).send({
                error: "LIMIT orders require a positive price"
            });
        }

        const orderId = nextOrderId();

        try{
            engineClient.sendOrder({
                orderId,
                userId,
                side,
                type,
                price: type==="MARKET"?0:price,
                quantity,
            });
        }
        catch(err){
            return reply.code(503).send({
                error: "Engine unavailable"
            });
        }

        return reply.code(202).send({
            status: "accepted",
            orderId,
        });
    });

    fastify.delete('/order/:id', async (request, reply) => {
        const orderId = parseInt(request.params.id, 10);

        if(Number.isNaN(orderId)){
            return reply.code(400).send({
                error: "Invalid order id"
            });
        }

        try{
            engineClient.sendCancel(orderId);
        }
        catch(err){
            return reply.code(503).send({
                error: "Engine unavailable"
            });
        }

        return reply.code(202).send({
            status: "cancel_submitted",
            orderId
        });
    })
}

module.exports = { registerRoutes };