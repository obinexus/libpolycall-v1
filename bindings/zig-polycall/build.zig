//! build.zig — zig-polycall package build script
//! Compatible with Zig 0.12 / 0.13.
//!
//! As a dependency, consumers add to their build.zig.zon:
//!   .zig_polycall = .{
//!       .url = "https://github.com/obinexus/zig-polycall/archive/<COMMIT>.tar.gz",
//!       .hash = "<ZON_HASH>",
//!   },
//!
//! Then in their build.zig:
//!   const polycall_dep = b.dependency("zig_polycall", .{
//!       .target  = target,
//!       .optimize = optimize,
//!   });
//!   exe.root_module.addImport("polycall", polycall_dep.module("polycall"));
//!   exe.linkLibrary(polycall_dep.artifact("polycall"));

const std = @import("std");

pub fn build(b: *std.Build) void {
    // ----------------------------------------------------------------
    // Standard options — exposed so consumers can forward them via
    // b.dependency("zig_polycall", .{ .target = target, .optimize = optimize })
    // ----------------------------------------------------------------
    const target   = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // ----------------------------------------------------------------
    // Path to the libpolycall-v2 C library source tree.
    // When used as an in-tree dependency (bindings/zig inside the
    // libpolycall-v2 repo) we resolve relative to this build.zig.
    // When consumed as a standalone package, override with:
    //   -Dlibpolycall_dir=/path/to/libpolycall-v2
    // ----------------------------------------------------------------
    const libpolycall_dir = b.option(
        []const u8,
        "libpolycall_dir",
        "Path to the libpolycall-v2 source root (default: ../..)",
    ) orelse "../..";

    const core_include = b.pathJoin(&.{ libpolycall_dir, "core/include" });

    // ----------------------------------------------------------------
    // Static C library — compile libpolycall from source.
    // If the project ships a pre-built .a, replace this with
    //   exe.addObjectFile(b.path("libpolycall.a"));
    // ----------------------------------------------------------------
    const libpolycall = b.addStaticLibrary(.{
        .name    = "polycall",
        .target  = target,
        .optimize = optimize,
    });

    libpolycall.addIncludePath(b.path(core_include));
    libpolycall.linkLibC();

    // Add all C source files under core/src/
    const core_src = b.pathJoin(&.{ libpolycall_dir, "core/src" });
    libpolycall.addCSourceFiles(.{
        .root = b.path(core_src),
        .files = &.{
            // Core context
            "polycall.c",
            // State machine
            "polycall_state_machine.c",
            // Protocol
            "polycall_protocol.c",
            // Network layer
            "network.c",
            // Micro-services
            "polycall_micro.c",
            // Tokenizer / parser
            "polycall_tokenizer.c",
            "polycall_parser.c",
        },
        .flags = &.{
            "-std=c11",
            "-Wall",
            "-Wextra",
            "-fPIC",
            // Silence POSIX warnings on Linux
            "-D_POSIX_C_SOURCE=200809L",
        },
    });

    // Expose the compiled static lib as a package artifact.
    b.installArtifact(libpolycall);

    // ----------------------------------------------------------------
    // Zig module — the public API surface.
    // ----------------------------------------------------------------
    const polycall_module = b.addModule("polycall", .{
        .root_source_file = b.path("src/root.zig"),
        .target   = target,
        .optimize = optimize,
    });
    polycall_module.addIncludePath(b.path(core_include));

    // ----------------------------------------------------------------
    // Example executable
    // ----------------------------------------------------------------
    const example = b.addExecutable(.{
        .name             = "basic_client",
        .root_source_file = b.path("examples/basic_client.zig"),
        .target           = target,
        .optimize         = optimize,
    });
    example.root_module.addImport("polycall", polycall_module);
    example.linkLibrary(libpolycall);
    example.linkLibC();

    const run_example = b.addRunArtifact(example);
    const run_step = b.step("run-example", "Run the basic_client example");
    run_step.dependOn(&run_example.step);

    b.installArtifact(example);

    // ----------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------
    const unit_tests = b.addTest(.{
        .root_source_file = b.path("src/root.zig"),
        .target           = target,
        .optimize         = optimize,
    });
    unit_tests.root_module.addIncludePath(b.path(core_include));
    unit_tests.linkLibrary(libpolycall);
    unit_tests.linkLibC();

    const run_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_tests.step);

    // ----------------------------------------------------------------
    // Check step (syntax + type-check only, no link)
    // ----------------------------------------------------------------
    const check = b.addTest(.{
        .root_source_file = b.path("src/root.zig"),
        .target           = target,
        .optimize         = optimize,
    });
    check.root_module.addIncludePath(b.path(core_include));

    const check_step = b.step("check", "Type-check without linking");
    check_step.dependOn(&check.step);
}
