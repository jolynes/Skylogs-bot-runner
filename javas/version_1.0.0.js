import axios, { AxiosResponse } from 'axios';

console.log("🟢 Roblox Join Notifier is running!");

const UNIVERSE_ID: string = "7639730222";
const GAME_URL: string = "https://www.roblox.com/games/100339443227710/Sky";
const DISCORD_WEBHOOK: string = "webhook";

let lastMessageId: string | null = null;
let previousState: "online" | "offline" | null = null;
let lastPrintState: "online" | "offline" | null = null;

const now = (): string => {
    return new Date().toLocaleTimeString('en-US', { hour12: false });
};

const deleteLastMessage = async (): Promise<void> => {
    if (lastMessageId) {
        try {
            const deleteUrl: string = `${DISCORD_WEBHOOK}/messages/${lastMessageId}`;
            const response: AxiosResponse = await axios.delete(deleteUrl);
            if (response.status === 200 || response.status === 204) {
                console.log(`[${now()}] Deleted previous message.`);
            } else {
                console.log(`[${now()}] Failed to delete message: ${response.status} ${JSON.stringify(response.data)}`);
            }
        } catch (error: any) {
            console.log(`[${now()}] Error deleting message: ${error.message}`);
        }
        lastMessageId = null;
    }
};

const sendMessage = async (content: string): Promise<void> => {
    await deleteLastMessage();
    try {
        const response: AxiosResponse = await axios.post(`${DISCORD_WEBHOOK}?wait=true`, { content });
        if (response.status === 200 || response.status === 204) {
            console.log(`[${now()}] Webhook sent: ${content}`);
            try {
                lastMessageId = response.data.id || null;
                if (!lastMessageId) {
                    console.log(`[${now()}] Warning: Couldn't retrieve message ID.`);
                }
            } catch (error: any) {
                console.log(`[${now()}] Error parsing webhook response: ${error.message}`);
                lastMessageId = null;
            }
        } else {
            console.log(`[${now()}] Webhook error: ${response.status} ${JSON.stringify(response.data)}`);
        }
    } catch (error: any) {
        console.log(`[${now()}] Error sending webhook: ${error.message}`);
    }
};

const checkPlayers = async (): Promise<void> => {
    const url: string = `https://games.roblox.com/v1/games?universeIds=${UNIVERSE_ID}`;

    try {
        const response: AxiosResponse = await axios.get(url);
        if (response.status === 429) {
            console.log(`[${now()}] Rate limited. Waiting 30 seconds.`);
            await new Promise(resolve => setTimeout(resolve, 30000));
            return;
        }

        const count: number = response.data.data[0].playing;

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

        if (count > 0) {
            if (previousState !== "online") {
                await sendMessage(
                    `🎮 Someone just joined **Sky**!\nCurrently **${count}** player(s) in-game.\n🔗 <${GAME_URL}>`
                );
                previousState = "online";
            }
        } else {
            if (previousState !== "offline") {
                await sendMessage("No players are online.");
                previousState = "offline";
            }
        }
    } catch (error: any) {
        console.log(`[${now()}] Error: ${error.message}`);
    }
};

const main = async (): Promise<void> => {
    while (true) {
        await checkPlayers();
        await new Promise(resolve => setTimeout(resolve, 6000));
    }
};

main().catch(error => console.error(`[${now()}] Main loop error: ${error.message}`));
