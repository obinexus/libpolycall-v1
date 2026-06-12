# zig-polycall

Zig bindings for [libpolycall-v2](https://github.com/obinexus/libpolycall) — the OBINexus polyglot protocol runtime.

Provides idiomatic Zig wrappers over the full libpolycall-v2 C API: core context management, finite state machine, protocol sessions, network endpoints, and micro-service isolation.

---

## Requirements

| Tool | Version |
|------|---------|
| Zig  | 0.12 or 0.13 |
| libpolycall-v2 | v2.x (compiled as a static library) |

---

## Installation

Add the package to your project's `build.zig.zon`:

```bash
zig fetch --save \
  https://github.com/obinexus/zig-polycall/archive/<TAG>.tar.gz
  https://github.com/obinexus/zig-polycall/archive/refs/tags/v0.2.0.zip
```

Replace `<TAG>` with a specific release tag or commit SHA — **never use `#HEAD`**, as the hash changes with every push and will break reproducible builds.

Then in your `build.zig`:

```zig
const polycall_dep = b.dependency("zig_polycall", .{
    .target  = target,
    .optimize = optimize,
});

exe.root_module.addImport("polycall", polycall_dep.module("polycall"));
exe.linkLibrary(polycall_dep.artifact("polycall"));
```

---

## Quick start

```zig
const std     = @import("std");
const polycall = @import("polycall");

pub fn main() !void {
    // 1. Core context
    std.debug.print("version: {s}\n", .{polycall.PolyCall.version()});

    var ctx = try polycall.PolyCall.init();
    defer ctx.deinit();

    // 2. State machine
    var sm = try polycall.StateMachine.init(ctx.handle, null);
    defer sm.deinit();

    try sm.addState("idle",    null, null, false);
    try sm.addState("running", null, null, false);
    try sm.addTransition("start", 0, 1, null, null);
    try sm.executeTransition("start");

    // 3. Protocol checksum utility (no live socket needed)
    const payload = "hello polycall";
    const csum = polycall.Protocol.calculateChecksum(payload);
    std.debug.print("checksum: 0x{x:0>8}\n", .{csum});
}
```

Run the bundled example:

```bash
zig build run-example -Dlibpolycall_dir=/path/to/libpolycall-v2
```

---

## Module layout

```
src/
├── root.zig          ← public entry point  (@import("polycall"))
├── types.zig         ← all C types as Zig extern equivalents
├── c.zig             ← raw extern function declarations
├── core.zig          ← PolyCall  — context lifecycle & flags
├── state_machine.zig ← StateMachine — states, transitions, snapshots
├── protocol.zig      ← Protocol  — handshake, auth, send/receive
├── network.zig       ← Endpoint / Program — TCP/UDP socket layer
└── micro.zig         ← MicroService / Banking — service isolation
```

### PolyCall — core context

```zig
var ctx = try polycall.PolyCall.init();
defer ctx.deinit();

try ctx.setFlag(polycall.Flags.debug);
std.debug.print("init: {}\n", .{ctx.isInitialized()});
```

### StateMachine

```zig
var sm = try polycall.StateMachine.init(ctx.handle, null);
defer sm.deinit();

try sm.addState("idle",       null, null, false);
try sm.addState("connecting", null, null, false);
try sm.addState("ready",      null, null, true);

try sm.addTransition("connect",   0, 1, null, null);
try sm.addTransition("handshook", 1, 2, null, null);

try sm.executeTransition("connect");
try sm.executeTransition("handshook");

const snap = try sm.createSnapshot(sm.currentState());
try sm.restoreSnapshot(&snap);
```

### Protocol

```zig
// Assumes you already have a NetworkEndpoint configured.
var proto = try polycall.Protocol.init(ctx.handle, &endpoint.inner, &config);
defer proto.deinit();

try proto.startHandshake();
try proto.authenticate("my-token");
try proto.send(.command, "RUN job42", polycall.ProtocolFlags.reliable);
```

### Network

```zig
var ep = polycall.Endpoint{ .inner = std.mem.zeroes(c.NetworkEndpoint) };
ep.inner.port     = 8080;
ep.inner.protocol = .tcp;
ep.inner.role     = .server;
try ep.init();
defer ep.close();
```

### MicroService

```zig
var svc = try polycall.MicroService.init("credit-score", 9090);
defer svc.deinit();

try svc.start();
// handle requests …
try svc.stop();
```

---

## Build options

| Option | Default | Description |
|--------|---------|-------------|
| `-Dlibpolycall_dir=<path>` | `../..` | Path to the libpolycall-v2 source root |
| `-Dtarget=<triple>` | native | Cross-compilation target |
| `-Doptimize=<mode>` | Debug | `Debug` / `ReleaseSafe` / `ReleaseFast` / `ReleaseSmall` |

```bash
zig build                          # build everything
zig build test                     # run unit tests
zig build check                    # type-check without linking
zig build run-example              # run examples/basic_client.zig
```

---

## Error handling

Every fallible operation returns a Zig error union from `polycall.Error`:

```zig
var ctx = polycall.PolyCall.init() catch |err| {
    std.debug.print("init failed: {}\n", .{err});
    return err;
};
```

The full error set:

```
InvalidParameters  OutOfMemory      NotInitialized  AlreadyInitialized
Network            Timeout          Unknown
InvalidState       InvalidTransition  MaxStatesReached  MaxTransitionsReached
InvalidContext     IntegrityCheckFailed  StateLocked  VersionMismatch
```

---

## Toolchain context

zig-polycall is part of the OBINexus build stack:

libpolycall-v2 sits at the protocol boundary layer, providing the runtime that all language bindings (Go, Java, Node, Rust, Zig) communicate through.

---

## License

Same as libpolycall-v2. See [LICENSE](LICENSE).

---

## Contributing — #NoGhosting policy

OBINexus operates under a structured milestone-based development model. If you open an issue or pull request, please follow through. Abandoned contributions without notice violate the project's collaboration contract. See the main libpolycall-v2 repository for the full policy.
