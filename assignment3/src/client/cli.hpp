#pragma once

#include <argparse/argparse.hpp>
#include <variant>

#include "config.hpp"

struct RegisterOption {
    Config config;
};

struct EncryptOption {
    Config config;
    std::string input;
    std::string output;
    bool force;
};

struct DecryptOption {
    Config config;
    std::string input;
    std::string output;
    bool force;
};

using Option = std::variant<RegisterOption, EncryptOption, DecryptOption>;

inline Option parseCommandLine(int argc, char** argv) {
    argparse::ArgumentParser program("hw3c", "0.1.0");

    argparse::ArgumentParser registerCmd("register");
    registerCmd.add_description("Register a user.");
    registerCmd.add_argument("-c", "--config")
        .help("Specify config file path")
        .default_value(DEFAULT_CONFIG_PATH)
        .required();

    argparse::ArgumentParser encryptCmd("encrypt");
    encryptCmd.add_description("Encrypt a file.");
    encryptCmd.add_argument("-c", "--config")
        .help("Specify key file path")
        .default_value(DEFAULT_CONFIG_PATH)
        .required();
    encryptCmd.add_argument("-i", "--input").help("Input file path").required();
    encryptCmd.add_argument("-o", "--output").help("Output file path");
    encryptCmd.add_argument("-f", "--force")
        .help("Force overwrite exist file")
        .default_value(false)
        .implicit_value(true);

    argparse::ArgumentParser decryptCmd("decrypt");
    decryptCmd.add_description("Decrypt a file.");
    decryptCmd.add_argument("-c", "--config")
        .help("Specify key file path")
        .default_value(DEFAULT_CONFIG_PATH)
        .required();
    decryptCmd.add_argument("-i", "--input").help("Input file path").required();
    decryptCmd.add_argument("-o", "--output").help("Output file path");
    decryptCmd.add_argument("-f", "--force")
        .help("Force overwrite exist file")
        .default_value(false)
        .implicit_value(true);

    program.add_subparser(registerCmd);
    program.add_subparser(encryptCmd);
    program.add_subparser(decryptCmd);

    program.parse_args(argc, argv);

    if (program.is_subcommand_used("register")) {
        auto configPath = registerCmd.get("-c");
        return RegisterOption{
            .config = loadConfig(configPath),
        };
    }
    if (program.is_subcommand_used("encrypt")) {
        auto configPath = encryptCmd.get("-c");
        auto input = encryptCmd.get("-i");
        auto output = encryptCmd.present("-o").value_or(input);
        return EncryptOption{
            .config = loadConfig(configPath),
            .input = input,
            .output = output,
            .force = encryptCmd.get<bool>("-f"),
        };
    }
    if (program.is_subcommand_used("decrypt")) {
        auto configPath = decryptCmd.get("-c");
        auto input = decryptCmd.get("-i");
        auto output = decryptCmd.present("-o").value_or(input);
        return DecryptOption{
            .config = loadConfig(configPath),
            .input = input,
            .output = output,
            .force = decryptCmd.get<bool>("-f"),
        };
    }
    throw std::runtime_error("no subcommand specified");
}