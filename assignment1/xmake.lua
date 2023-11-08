add_rules("mode.debug", "mode.release")

add_requires("gmssl")
add_requires("boost", {configs = {program_options = true}})

target("encbox")
    add_packages("gmssl", "boost")
    set_languages("c++23")
    add_files("src/*.cpp")
