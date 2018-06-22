#ifndef NETWORKING_H
#define NETWORKING_H

#include <map>
#include <unordered_map>
#include <utility>

#include "Command.hpp"
#include "Connection.hpp"
#include "NetworkingConfig.hpp"
#include "Player.hpp"

#ifdef _WIN32
#include "WinConnection.hpp"
#else

#include "UnixConnection.hpp"

#endif

namespace hlt {
class Halite;
}

namespace net {

/** The type of platform-specific connections. */
#ifdef _WIN32
using OSConnection = WinConnection;
#else
using OSConnection = UnixConnection;
#endif

/** Networking support suite for Halite. */
class Networking final {
private:
    ConnectionFactory<OSConnection> connection_factory; /**< The platform-specific connection factory. */
    Connections connections{};    /**< The current network connections. */
    NetworkingConfig config;      /**< The networking configuration. */
    const hlt::Halite *game; /**< The current game. Raw pointer because Networking should always be owned by a game. */
    std::mutex connections_mutex; /**< Mutex used to protect the connections map. */

public:
    /**
     * Launch the bot for a player, send the initial game information to the player, and update its name.
     * Safe to invoke from multiple threads on different players.
     *
     * @param player The player to communicate with.
     */
    void initialize_player(hlt::Player &player);

    /**
     * Handle the networking for a single frame, obtaining a command from the player if there is one.
     * Safe to invoke from multiple threads on different players.
     *
     * @param player The player to communicate with.
     * @return The command from the player.
     */
    hlt::Command handle_frame(const hlt::Player &player);

    /**
     * Initialize Networking from configuration and Halite game.
     * @param config The configuration.
     * @param game The Halite game.
     */
    explicit Networking(NetworkingConfig config, const hlt::Halite *game) :
            connection_factory(config), config(config), game(game) {};
};

}

#endif