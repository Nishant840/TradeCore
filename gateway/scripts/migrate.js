require('dotenv').config();
const fs = require('fs');
const path = require('path');
const { pool } = require('../src/db');

async function migrate() {
    const sql = fs.readFileSync(path.join(__dirname, '../src/schema.sql'), 'utf-8');

    try {
        await pool.query(sql);
        console.log('Migration successful — orders and trades tables ready');
    } catch (err) {
        console.error('Migration failed:', err);
        process.exit(1);
    } finally {
        await pool.end();
    }
}

migrate();