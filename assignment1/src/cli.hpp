#pragma once

#include <boost/program_options.hpp>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "boost/program_options/value_semantic.hpp"
#include "rand.hpp"

struct KeygenOption {
    // std::size_t seed;
    std::string output;
    bool force;
};

struct EncryptOption {
    std::optional<std::size_t> encryptSeed;  // nullopt for decryption
    std::optional<std::string> keyPath;
    std::string pkey;
    std::string input;
    std::string output;
    bool enableDigest;
    bool force;
};

using ParsedOption = std::variant<KeygenOption, EncryptOption>;

namespace detail {

using namespace std::literals;

template <typename T>
struct Tag {};

inline ParsedOption vm2struct(boost::program_options::variables_map vm) {
    auto cmd = vm["command"].as<std::string>();

    auto getOptional = [&]<typename T = std::string>(const std::string& key,
                                                     Tag<T>)
                           ->std::optional<T> {
        if (auto it = vm.find(key); it != vm.end()) {
            return it->second.as<T>();
        } else {
            return std::nullopt;
        }
    };

    if (cmd == "keygen"s) {
        return KeygenOption{// .seed = getOptional("seed"s,
                            // Tag<std::size_t>{}).value_or(randomInt()),
                            .output = vm["output"].as<std::string>(),
                            .force = vm["force"].as<bool>()};
    } else {
        // encrypt or decrypt
        auto encryptSeed =
            getOptional("seed"s, Tag<std::size_t>{}).or_else([&] {
                return cmd == "encrypt" ? std::optional{randomInt()}
                                        : std::nullopt;
            });
        auto input = vm["input"].as<std::string>();
        return EncryptOption{
            .encryptSeed = encryptSeed,
            .keyPath = getOptional("key-path"s, Tag<std::string>{}),
            .pkey = vm["pkey"].as<std::string>(),
            .input = input,
            .output =
                getOptional("output"s, Tag<std::string>{}).value_or(input),
            .enableDigest = !vm["no-verify"].as<bool>(),
            .force = vm["force"].as<bool>()};
    }
}

}  // namespace detail

// clang-format off

inline ParsedOption parseCommandLine(int argc, char** argv) {
    namespace po = boost::program_options;
    using namespace std::literals;

    // Subcommand CLI parsing
    // https://stackoverflow.com/a/23098581

    po::options_description global("Global options");
    global.add_options()
        ("help", po::bool_switch(), "Show help message")
        ("force", po::bool_switch(), "Force overwrite existing")
        ("command", po::value<std::string>(), "Command to execute")
        ("subargs", po::value<std::vector<std::string>>(), "Arguments for command");
    
    po::positional_options_description pos;
    pos.add("command", 1)
        .add("subargs", -1);
    
    auto parsed = po::command_line_parser(argc, argv)
        .options(global)
        .positional(pos)
        .allow_unregistered()
        .run();

    po::variables_map vm;
    po::store(parsed, vm);    

    if (!vm.contains("command")) {
        std::cout << std::format(R"(encbox, Assignment #1 of UCAS course "Crypto Technology Practice".
Copyright (c) Guyutongxue 2023

Usage: {} command args...

Available Commands:
  keygen         Generate a public-private key pair.
  encrypt        Encrypt a folder.
  decrypt        Decrypt a folder.
  
Use --help after command for detailed description.)", argv[0]) << std::endl;
        exit(0);
    }

    auto cmd = vm["command"].as<std::string>();

    po::options_description keygenDesc("Keygen options");
    keygenDesc.add_options()
        // ("seed", po::value<std::size_t>(), "Seed for key generation")
        ("output", po::value<std::string>()->default_value("id_key"s), 
            "Output private key path. Public key filename will be the same with .pub extension");

    po::options_description encryptDesc("Encrypt options");
    encryptDesc.add_options()
        ("pkey", po::value<std::string>()->default_value("id_key.pub"s), "Public key path")
        ("input", po::value<std::string>(), "Input folder")
        ("output", po::value<std::string>(), "Output destination. Defaulted to overwrite original input folder")
        ("key-path", po::value<std::string>(), "Path for storing encrypted encryption key. Defaulted to ${outputFolder}/.encbox.key")
        ("no-verify", po::bool_switch(), "Do not generate file digest for verifying")
        ("seed", po::value<std::size_t>(), "Fix seed for generating key. ONLY FOR DEBUGGING PURPOSES!");

    po::options_description decryptDesc("Decrypt options");
    decryptDesc.add_options()
        ("pkey", po::value<std::string>()->default_value("id_key"s), "Private key path")
        ("input", po::value<std::string>(), "Input encrypted folder")
        ("output", po::value<std::string>(), "Output destination. Defaulted to overwrite original input folder")
        ("key-path", po::value<std::string>(), "Path for storing encrypted encryption key. Defaulted to ${outputFolder}/.encbox.key")
        ("no-verify", po::bool_switch(), "Do not verify file digest");
    
    std::map<std::string, po::options_description> subDescMap{
        {"encrypt"s, encryptDesc},
        {"decrypt"s, decryptDesc},
        {"keygen"s, keygenDesc}
    };

    if (auto it = subDescMap.find(cmd); it != subDescMap.end()) {
        auto opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin()); // remove command name
        auto parser = po::command_line_parser(opts).options(it->second);
        po::positional_options_description pos;
        if (cmd == "encrypt"s || cmd == "decrypt"s) {
            pos.add("input", 1);
            parser.positional(pos);
        }
        po::store(parser.run(), vm);

        if (vm["help"].as<bool>()) {
            std::cout << it->second << std::endl;
            exit(0);
        }
        vm.erase("subargs");
        return detail::vm2struct(std::move(vm));
    } else {
        throw std::runtime_error("Unknown command: " + cmd);
    }
}
