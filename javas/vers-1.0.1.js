const axios = require('axios');

console.log("🟢 Roblox Sky Join Notifier is running!");

const UNIVERSE_ID = "7639730222";
const GAME_URL = "https://www.roblox.com/games/100339443227710/Sky";
const DISCORD_WEBHOOK = "https://discord.com/api/webhooks/1401037060722393231/jKvPKzHPWvKg78MdJ58ySb99Pgyq0X-3HSE2YSZRA3yJSw4i61YPM4F4lB8mIevqvuHH";

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
