import requests
import time

print("🟢 Roblox Join Notifier is running!")

UNIVERSE_ID = "7639730222"
GAME_URL = "https://www.roblox.com/games/100339443227710/Sky"
DISCORD_WEBHOOK = "webhook"

last_message_id = None
previous_state = None  # "online" or "offline""
last_print_state = None  # For console output xd

def now():
    return time.strftime('%H:%M:%S')

def delete_last_message():
    global last_message_id
    if last_message_id:
        try:
            delete_url = f"{DISCORD_WEBHOOK}/messages/{last_message_id}"
            response = requests.delete(delete_url)
            if response.status_code in [200, 204]:
                print(f"[{now()}] Deleted previous message.")
            else:
                print(f"[{now()}] Failed to delete message: {response.status_code} {response.text}")
        except Exception as e:
            print(f"[{now()}] Error deleting message: {e}")
        last_message_id = None

def send_message(content):
    global last_message_id
    delete_last_message()
    try:
        response = requests.post(DISCORD_WEBHOOK + "?wait=true", json={"content": content})
        if response.status_code in [200, 204]:
            print(f"[{now()}] Webhook sent: {content}")
            try:
                last_message_id = response.json().get("id")
                if last_message_id is None:
                    print(f"[{now()}] Warning: Couldn't retrieve message ID.")
            except Exception as e:
                print(f"[{now()}] Error parsing webhook response: {e}")
                last_message_id = None
        else:
            print(f"[{now()}] Webhook error: {response.status_code} {response.text}")
    except Exception as e:
        print(f"[{now()}] Error sending webhook: {e}")

def check_players():
    global previous_state, last_print_state

    url = f"https://games.roblox.com/v1/games?universeIds={UNIVERSE_ID}"

    try:
        response = requests.get(url)
        if response.status_code == 429:
            print(f"[{now()}] Rate limited. Waiting 30 seconds.")
            time.sleep(30)
            return

        response.raise_for_status()
        data = response.json()
        count = data['data'][0]['playing']

        if count > 0:
            if last_print_state != "online":
                print(f"[{now()}] Polled player count: {count}")
                last_print_state = "online"
        else:
            if last_print_state != "offline":
                print(f"[{now()}] Polled player count: 0")
                last_print_state = "offline"

        if count > 0:
            if previous_state != "online":
                send_message(f"🎮 Someone just joined **Sky**!\nCurrently **{count}** player(s) in-game.\n🔗 <{GAME_URL}>")
                previous_state = "online"
        else:
            if previous_state != "offline":
                send_message("No players are online.")
                previous_state = "offline"

    except Exception as e:
        print(f"[{now()}] Error: {e}")

while True:
    check_players()
    time.sleep(6)
