add_rules("mode.debug", "mode.release")

add_requires("argparse")
add_requires("cpp-httplib", {configs = {ssl = true}})
add_requires("openssl")
add_requires("sqlitecpp")
add_requires("toml++")

add_defines("USE_SSL")

target("common")
    set_kind("headeronly")
    add_includedirs("src/common", {public = true})

target("hw3s")
    set_kind("binary")
    set_languages("c++23")
    add_files("src/server/*.cpp")
    add_packages("cpp-httplib", "openssl", "sqlitecpp")
    add_deps("common")

target("hw3c")
    set_kind("binary")
    set_languages("c++23")
    add_files("src/client/*.cpp")
    add_packages("argparse", "cpp-httplib", "openssl", "toml++")
    add_deps("common")
