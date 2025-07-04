#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string UNIVERSE_ID = "7639730222";
const std::string GAME_URL = "https://www.roblox.com/games/100339443227710/Sky";
const std::string DISCORD_WEBHOOK = "webhook";

std::string last_message_id;
std::string previous_state;
std::string last_print_state;

std::string now() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    char buffer[9];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    return std::string(buffer);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

void delete_last_message() {
    if (!last_message_id.empty()) {
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string delete_url = DISCORD_WEBHOOK + "/messages/" + last_message_id;
            std::string response;

            curl_easy_setopt(curl, CURLOPT_URL, delete_url.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (res == CURLE_OK && (response_code == 200 || response_code == 204)) {
                std::cout << "[" << now() << "] Deleted previous message.\n";
            } else {
                std::cout << "[" << now() << "] Failed to delete message: " << response_code << " " << response << "\n";
            }
            curl_easy_cleanup(curl);
        } else {
            std::cout << "[" << now() << "] Error initializing CURL for delete.\n";
        }
        last_message_id.clear();
    }
}

void send_message(const std::string& content) {
    delete_last_message();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;
        json payload = {{"content", content}};
        std::string json_str = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, (DISCORD_WEBHOOK + "?wait=true").c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res == CURLE_OK && (response_code == 200 || response_code == 204)) {
            std::cout << "[" << now() << "] Webhook sent: " << content << "\n";
            try {
                json response_json = json::parse(response);
                last_message_id = response_json.value("id", "");
                if (last_message_id.empty()) {
                    std::cout << "[" << now() << "] Warning: Couldn't retrieve message ID.\n";
                }
            } catch (const std::exception& e) {
                std::cout << "[" << now() << "] Error parsing webhook response: " << e.what() << "\n";
                last_message_id.clear();
            }
        } else {
            std::cout << "[" << now() << "] Webhook error: " << response_code << " " << response << "\n";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cout << "[" << now() << "] Error initializing CURL for webhook.\n";
    }
}

void check_players() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "https://games.roblox.com/v1/games?universeIds=" + UNIVERSE_ID;
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (res == CURLE_OK) {
            if (response_code == 429) {
                std::cout << "[" << now() << "] Rate limited. Waiting 30 seconds.\n";
                std::this_thread::sleep_for(std::chrono::seconds(30));
                curl_easy_cleanup(curl);
                return;
            }

            try {
                json data = json::parse(response);
                int count = data["data"][0]["playing"].get<int>();

                if (count > 0) {
                    if (last_print_state != "online") {
                        std::cout << "[" << now() << "] Polled player count: " << count << "\n";
                        last_print_state = "online";
                    }
                } else {
                    if (last_print_state != "offline") {
                        std::cout << "[" << now() << "] Polled player count: 0\n";
                        last_print_state = "offline";
                    }
                }

                if (count > 0) {
                    if (previous_state != "online") {
                        send_message("🎮 Someone just joined **Sky**!\nCurrently **" + 
                                     std::to_string(count) + 
                                     "** player(s) in-game.\n🔗 <" + GAME_URL + ">");
                        previous_state = "online";
                    }
                } else {
                    if (previous_state != "offline") {
                        send_message("No players are online.");
                        previous_state = "offline";
                    }
                }
            } catch (const std::exception& e) {
                std::cout << "[" << now() << "] Error parsing response: " << e.what() << "\n";
            }
        } else {
            std::cout << "[" << now() << "] CURL error: " << curl_easy_strerror(res) << "\n";
        }
        curl_easy_cleanup(curl);
    } else {
        std::cout << "[" << now() << "] Error initializing CURL.\n";
    }
}

int main() {
    std::cout << "🟢 Roblox Join Notifier is running!\n";
    curl_global_init(CURL_GLOBAL_ALL);

    while (true) {
        check_players();
        std::this_thread::sleep_for(std::chrono::seconds(6));
    }

    curl_global_cleanup();
    return 0;
}
