const { buildServer } = require("./src/server")

async function main(){
    const server = await buildServer();

    try {
        await server.listen({
            port: 4000,
            host: '0.0.0.0'
        });
        console.log('Gateway listening on http://localhost:4000');
    }
    catch (err){
        console.error('Failed to start gateway:', err);
        process.exit(1);
    }
}

main();