#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>  // Include JSON library (https://github.com/nlohmann/json)

using json = nlohmann::json;

std::string UNIVERSE_ID = "7639730222";
std::string GAME_URL = "https://www.roblox.com/games/100339443227710/Sky";
std::string DISCORD_WEBHOOK = "https://discord.com/api/webhooks/1401037060722393231/jKvPKzHPWvKg78MdJ58ySb99Pgyq0X-3HSE2YSZRA3yJSw4i61YPM4F4lB8mIevqvuHH";

std::string lastMessageId = "";
std::string previousState = "";
std::string lastPrintState = "";

std::string now() {
    time_t t = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&t));
    return std::string(buf);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

void deleteLastMessage() {
    if (lastMessageId.empty()) return;

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = DISCORD_WEBHOOK + "/messages/" + lastMessageId;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK)
            std::cout << "[" << now() << "] Deleted previous message.\n";
        else
            std::cout << "[" << now() << "] Failed to delete message.\n";

        curl_easy_cleanup(curl);
    }

    lastMessageId = "";
}

void sendMessage(const std::string& content) {
    deleteLastMessage();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string responseStr;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string payload = "{\"content\":\"" + content + "\"}";
        std::string url = DISCORD_WEBHOOK + "?wait=true";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::cout << "[" << now() << "] Webhook sent: " << content << "\n";

            try {
                auto j = json::parse(responseStr);
                lastMessageId = j["id"];
            } catch (...) {
                std::cout << "[" << now() << "] Failed to parse webhook response.\n";
            }
        } else {
            std::cout << "[" << now() << "] Failed to send webhook.\n";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void checkPlayers() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://games.roblox.com/v1/games?universeIds=" + UNIVERSE_ID;
        std::string responseStr;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            try {
                auto j = json::parse(responseStr);
                int count = j["data"][0]["playing"];

                if (count > 0 && lastPrintState != "online") {
                    std::cout << "[" << now() << "] Polled player count: " << count << "\n";
                    lastPrintState = "online";
                } else if (count == 0 && lastPrintState != "offline") {
                    std::cout << "[" << now() << "] Polled player count: 0\n";
                    lastPrintState = "offline";
                }

                if (count > 0 && previousState != "online") {
                    sendMessage("🎮 Someone just joined **Sky**!\nCurrently **" + std::to_string(count) + "** player(s) in-game.\n🔗 <" + GAME_URL + ">");
                    previousState = "online";
                } else if (count == 0 && previousState != "offline") {
                    sendMessage("No players are online.");
                    previousState = "offline";
                }

            } catch (...) {
                std::cout << "[" << now() << "] JSON parse error.\n";
            }
        } else {
            std::cout << "[" << now() << "] HTTP error while checking players.\n";
        }

        curl_easy_cleanup(curl);
    }
}

int main() {
    std::cout << "🟢 Roblox Sky Join Notifier is running!\n";
    while (true) {
        checkPlayers();
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }
    return 0;
}
