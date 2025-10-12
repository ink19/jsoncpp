add_requires("boost[json,pfr]", "gtest")

add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
set_languages("c++23")

add_headerfiles("include")

target("test")
    set_kind("binary")
    set_group("tests")
    add_files("test/test.cpp")
    add_packages("gtest", "boost")
    add_includedirs("include")
