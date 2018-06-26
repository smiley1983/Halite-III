#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>

#include "Config.hpp"
#include "Constants.hpp"
#include "Generator.hpp"
#include "Halite.hpp"
#include "Logging.hpp"

#include "version.hpp"

#include "nlohmann/json.hpp"
#include "tclap/CmdLine.h"

/** The platform-specific path separator. */
#ifdef _WIN32
constexpr auto SEPARATOR = '\\';
#else
constexpr auto SEPARATOR = '/';
#endif

int main(int argc, char *argv[]) {
    auto &constants = hlt::Constants::get_mut();
    hlt::Config config{};

    using namespace TCLAP;
    CmdLine cmd("Halite Game Environment", ' ', HALITE_VERSION);
    SwitchArg timeout_switch("", "no-timeout", "Causes game environment to ignore bot timeouts.", cmd, false);
    SwitchArg no_replay_switch("", "no-replay", "Turns off the replay generation.", cmd, false);
    SwitchArg print_constants_switch("", "print-constants", "Print out the default constants and exit.", cmd, false);
    SwitchArg no_compression_switch("", "no-compression", "Disables compression for output files.", cmd, false);
    ValueArg<unsigned long> players_arg("n", "players", "Create a map that will accommodate n players.", false, 1,
                                        "positive integer", cmd);
    ValueArg<hlt::dimension_type> width_arg("", "width", "The width of the map.", false,
                                                 constants.DEFAULT_MAP_WIDTH,
                                                 "positive integer", cmd);
    ValueArg<hlt::dimension_type> height_arg("", "height", "The height of the map.", false,
                                                  constants.DEFAULT_MAP_HEIGHT,
                                                  "positive integer", cmd);
    ValueArg<unsigned int> seed_arg("s", "seed", "The seed for the map generator.", false, 0, "positive integer", cmd);
    ValueArg<std::string> replay_arg("i", "replay-directory", "The path to directory for replay output.", false, ".",
                                     "path to directory", cmd);
    ValueArg<std::string> constants_arg("c", "constants-file", "JSON file containing runtime constants to use.", false,
                                        "", "path to file", cmd);
    MultiArg<std::string> override_args("o", "override-names", "Overrides player-sent names.", false, "name strings",
                                        cmd);
    MultiSwitchArg verbosity_arg("v", "verbosity", "Increase the logging verbosity level.", cmd);
    UnlabeledMultiArg<std::string> command_args("bot-commands", "Start commands for bots.", true, "path strings", cmd);

    cmd.parse(argc, argv);

    // If requested, print constants and exit
    if (print_constants_switch.getValue()) {
        std::cout << nlohmann::json(constants).dump() << std::endl;
        return 0;
    }

    // Update the game constants
    if (constants_arg.isSet()) {
        std::ifstream constants_file(constants_arg.getValue());
        nlohmann::json constants_json;
        constants_file >> constants_json;
        from_json(constants_json, constants);
    }

    // TODO: set game ID

    // Set the random seed
    config.seed = static_cast<unsigned int>(time(nullptr));
    std::srand(config.seed); // For all non-seeded randomness
    if (seed_arg.getValue() != 0) {
        config.seed = seed_arg.getValue();
    }

    // Get the map parameters
    auto map_width = width_arg.getValue();
    auto map_height = height_arg.getValue();
    auto n_players = players_arg.getValue();

    auto verbosity = verbosity_arg.getValue();
    verbosity++; // One more level than specified verbosity
    verbosity = verbosity > Logging::NUM_LEVELS ? 0 : Logging::NUM_LEVELS - verbosity;
    Logging::set_level(static_cast<Logging::Level>(verbosity));

    // Read the player bot commands
    auto bot_commands = command_args.getValue();
    if (bot_commands.size() > constants.MAX_PLAYERS) {
        std::cerr << "Error: too many players (max is " << constants.MAX_PLAYERS << ")" << std::endl;
        return 1;
    } else if (bot_commands.size() > n_players) {
        n_players = bot_commands.size();
        if (players_arg.isSet()) {
            std::cerr << "Warning: overriding the specified number of players." << std::endl;
        }
    }
    hlt::mapgen::MapParameters map_param{hlt::mapgen::MapType::Basic, config.seed, map_width, map_height, n_players};

    // TODO: override names

    net::NetworkingConfig networking_config{};
    networking_config.ignore_timeout = timeout_switch.getValue();

    std::list<hlt::Player> players;
    hlt::PlayerFactory player_factory;
    for (const auto &command : bot_commands) {
        players.push_back(player_factory.new_player(command));
    }

    std::string replay_directory = replay_arg.getValue();
    if (replay_directory.back() != SEPARATOR) replay_directory.push_back(SEPARATOR);

    hlt::Halite game(config, map_param, networking_config, players);
    game.run_game();

    // Output replay file for visualizer
    if (!no_replay_switch.getValue()){
        // Output gamefile. First try the replays folder; if that fails, just use the straight filename.
        std::stringstream filename_buf;
        // While compilers like G++4.8 report C++11 compatibility, they do not
        // support std::put_time, so we have to use strftime instead.
        auto time = std::time(nullptr);
        auto localtime = std::localtime(&time);
        static constexpr size_t MAX_DATE_STRING_LENGTH = 25;
        char time_string[MAX_DATE_STRING_LENGTH];
        std::strftime(time_string, MAX_DATE_STRING_LENGTH, "%Y%m%d-%H%M%S%z", localtime);
        filename_buf << "replay-" << std::string(time_string);
        filename_buf << "-" << game.replay_struct.map_generator_seed;
        filename_buf << "-" << game.game_map.width;
        filename_buf << "-" << game.game_map.height << ".hlt";
        auto filename = filename_buf.str();
        std::string output_filename = replay_directory + "Replays/" + filename;
        bool enable_compression = !no_compression_switch.getValue();
        try {
            game.replay_struct.output(output_filename, enable_compression);
        }
        catch (std::runtime_error& e) {
            output_filename = replay_directory + filename;
            game.replay_struct.output(output_filename, enable_compression);
        }
        std::stringstream replay_message;
        replay_message << "Map seed was " << game.replay_struct.map_generator_seed << std::endl
                       << "Opening a file at " << output_filename << std::endl;
        Logging::log(replay_message.str());
    }

    return 0;
}
