const axios = require('axios');

console.log("🟢 Roblox Sky Join Notifier is running!"); // change to wtv u want

const UNIVERSE_ID = "put universe-game id here";
const GAME_URL = "put game url here";
const DISCORD_WEBHOOK = "put webhook here";

let lastMessageId = null;
let previousState = null;
let lastPrintState = null;

function now() {
    return new Date().toLocaleTimeString();
}

async function deleteLastMessage() {
    if (!lastMessageId) return;

    try {
        const url = `${DISCORD_WEBHOOK}/messages/${lastMessageId}`;
        const response = await axios.delete(url);
        console.log(`[${now()}] Deleted previous message.`);
    } catch (err) {
        console.log(`[${now()}] Error deleting message: ${err}`);
    }

    lastMessageId = null;
}

async function sendMessage(content) {
    await deleteLastMessage();

    try {
        const response = await axios.post(`${DISCORD_WEBHOOK}?wait=true`, { content });
        console.log(`[${now()}] Webhook sent: ${content}`);
        lastMessageId = response.data.id || null;
    } catch (err) {
        console.log(`[${now()}] Error sending webhook: ${err}`);
    }
}

async function checkPlayers() {
    try {
        const url = `https://games.roblox.com/v1/games?universeIds=${UNIVERSE_ID}`;
        const response = await axios.get(url);

        const count = response.data.data[0].playing;

        if (count > 0) {
            if (lastPrintState !== "online") {
                console.log(`[${now()}] Polled player count: ${count}`);
                lastPrintState = "online";
            }
        } else {
            if (lastPrintState !== "offline") {
                console.log(`[${now()}] Polled player count: 0`);
                lastPrintState = "offline";
            }
        }

        if (count > 0 && previousState !== "online") {
            await sendMessage(`🎮 Someone just joined **Sky**!\nCurrently **${count}** player(s) in-game.\n🔗 <${GAME_URL}>`);
            previousState = "online";
        } else if (count === 0 && previousState !== "offline") {
            await sendMessage("No players are online.");
            previousState = "offline";
        }

    } catch (err) {
        console.log(`[${now()}] Error: ${err}`);
    }
}

setInterval(checkPlayers, 6000);
