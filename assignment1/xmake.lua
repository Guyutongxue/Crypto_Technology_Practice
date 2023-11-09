add_rules("mode.debug", "mode.release")

add_requires("gmssl")
add_requires("boost", {configs = {program_options = true}})

target("encbox")
    add_packages("gmssl", "boost")
    set_languages("c++23")
    -- set_policy("build.c++.modules", true)
    -- Seems a bug in xmake package, see
    -- https://github.com/xmake-io/xmake-repo/pull/2813
    if is_plat("windows") then
        add_syslinks("advapi32")
    end
    add_files("src/*.cpp")
