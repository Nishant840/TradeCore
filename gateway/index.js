const { buildServer } = require("./src/server")

async function main(){
    const server = await buildServer();

    try {
        const port = process.env.PORT || 4000;
        await server.listen({
            port: port,
            host: '0.0.0.0'
        });
        console.log(`Gateway listening on http://localhost:${port}`);
    }
    catch (err){
        console.error('Failed to start gateway:', err);
        process.exit(1);
    }
}

main();