#include <sleepy_discord/sleepy_discord.h>
#include <unordered_map>

class MyBot : public SleepyDiscord::DiscordClient {
public:
    using SleepyDiscord::DiscordClient::DiscordClient;

    std::unordered_map<int, std::string> shuffles = {
        {3, "a3 s a2 s a1"},
        {4, "a3 s a2 s a4 s a1"},
        {5, "a3 s a4 s a2 s a5 s a1"},
        {6, "a2 s a4 s a3 s a1 s a5 s a6"},
        {7, "a4 s a5 s a1 s a7 s a2 s a3 s a6"},
        {8, "a3 s a2 s a4 s a8 s a6 s a5 s a7 s a1"},
        {9, "a5 s a1 s a7 s a3 s a4 s a9 s a2 s a6 s a8"}
    };

    std::unordered_map<std::string, std::string> addresses = {
        {"LuaVM_load", "0xBC85F0"},
        {"GetLuaState", "0xBC3DD0"},
        {"FireClickDetector", "0x1C34F00"},
        {"LuaO_NilObject", "0x4CD0028"},
        {"Luau_execute", "0x273D6E0"},
        {"RaiseEventInvocation", "0x14C2960"}
    };

    std::unordered_map<std::string, std::string> offsets = {
        {"PropertyDescriptor", "0x3C0"},
        {"IsCoreScript", "0x1A0"},
        {"RequireBypass", "0x7E8"}
    };

    std::unordered_map<std::string, std::string> hyperion = {
        {"SetInsertFuncO", "0xce9e90"},
        {"WhitelistedPagesO", "0x26b720"},
        {"BitMap", "0x28add8"},
        {"PageHashXor", "0xd252ed77"},
        {"ByteHashXor", "0x99"}
    };

    void onMessage(SleepyDiscord::Message message) override {
        std::string content = message.content;

        if (startsWith(content, "!shuffle")) {
            int num = std::stoi(content.substr(9));
            auto it = shuffles.find(num);
            if (it != shuffles.end()) {
                sendMessage(message.channelID, "**Shuffle " + std::to_string(num) + "**: `" + it->second + "`");
            } else {
                sendMessage(message.channelID, "Shuffle not found.");
            }
        }

        else if (startsWith(content, "!address")) {
            std::string key = content.substr(9);
            auto it = addresses.find(key);
            if (it != addresses.end()) {
                sendMessage(message.channelID, "**" + key + "** = `" + it->second + "`");
            } else {
                sendMessage(message.channelID, "Address not found.");
            }
        }

        else if (startsWith(content, "!offset")) {
            std::string key = content.substr(8);
            auto it = offsets.find(key);
            if (it != offsets.end()) {
                sendMessage(message.channelID, "**" + key + "** = `" + it->second + "`");
            } else {
                sendMessage(message.channelID, "Offset not found.");
            }
        }

        else if (content == "!lua") {
            std::string code =
                "```cpp\n"
                "lua_State* GetLuaState(uintptr_t scriptContext) {\n"
                "  uintptr_t encryptedState = scriptContext + Offsets::GlobalState + Offsets::GlobalStateStart + Offsets::EncryptedState;\n"
                "  uint32_t low = *(uint32_t*)(encryptedState) - (uint32_t)encryptedState;\n"
                "  uint32_t high = *(uint32_t*)(encryptedState + 4) - (uint32_t)encryptedState;\n"
                "  uintptr_t luaState = ((uint64_t)high << 32) | low;\n"
                "  return (lua_State*)luaState;\n"
                "}```";
            sendMessage(message.channelID, code);
        }

        else if (content == "!hyperion") {
            std::string response = "**🔐 Hyperion Addresses**\n";
            for (const auto& [key, val] : hyperion) {
                response += "**" + key + "** = `" + val + "`\n";
            }
            sendMessage(message.channelID, response);
        }
    }

private:
    bool startsWith(const std::string& str, const std::string& prefix) {
        return str.compare(0, prefix.size(), prefix) == 0;
    }
};

int main() {
    std::string token = "YOUR_BOT_TOKEN_HERE";  // Replace with your bot token
    MyBot bot(token, SleepyDiscord::USER_CONTROLED_THREADS);
    bot.run();
    return 0;
}
