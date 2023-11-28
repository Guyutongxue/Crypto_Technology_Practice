add_rules("mode.debug", "mode.release")

add_requires("gmssl", "asn1c")

rule("asn1c_p")
    set_extensions(".asn1")
    before_buildcmd_file(function (target, batchcmds, sourcefile_asn1, opt)

        -- get asn1c
        import("lib.detect.find_tool")
        local asn1c = assert(find_tool("asn1c"), "asn1c not found!")

        -- asn1 to *.c sourcefiles
        local sourcefile_dir = path.join(target:autogendir(), "rules", "asn1c")
        batchcmds:show_progress(opt.progress, "${color.build.object}compiling.asn1c %s", sourcefile_asn1)
        batchcmds:mkdir(sourcefile_dir)
        batchcmds:vrunv(asn1c.program, {path(sourcefile_asn1):absolute()}, {curdir = sourcefile_dir})

        -- add includedirs
        target:add("includedirs", sourcefile_dir)
    end)
    on_buildcmd_file(function (target, batchcmds, sourcefile_asn1, opt)
        local sourcefile_dir = path.join(target:autogendir(), "rules", "asn1c")

        -- compile *.c
        for _, sourcefile in ipairs(os.files(path.join(sourcefile_dir, "*.c|converter-*.c"))) do
            local objectfile = target:objectfile(sourcefile)
            batchcmds:compile(sourcefile, objectfile, {configs = {includedirs = sourcefile_dir}})
            table.insert(target:objectfiles(), objectfile)
        end

        -- add deps
        batchcmds:add_depfiles(sourcefile_asn1)
        batchcmds:set_depcache(target:dependfile(sourcefile_asn1))
    end)

target("assignment2")
    set_license("GPL-3.0")
    set_languages("c++23")
    add_packages("gmssl", "asn1c")
    add_rules("asn1c_p")
    add_files("src/*.cpp", "src/*.asn1")
