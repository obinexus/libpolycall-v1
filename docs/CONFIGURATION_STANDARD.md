# LibPolyCall v1.0.1 Configuration Standard

LibPolyCall configuration has three ordered layers:

1. `Polycallfile`
2. `Polycallrc`
3. `Polycallrc.<language>`

Later layers override earlier `key=value` settings. Server topology remains
owned by `Polycallfile`.

## Polycallfile

`Polycallfile` is the writable project definition layer. It defines project
topology, language servers, network state, and project-wide settings.

The existing format is unchanged:

```text
server node 8080:8084
server python 3001:8084
network start
workspace_root=/opt/polycall
log_directory=/var/log/polycall
```

Server mappings use `host_port:target_port`. Both ports must be between 1 and
65535. Duplicate server languages are invalid.

## Polycallrc

`Polycallrc` is the global read-only runtime layer. "Read-only" describes its
role in the configuration contract: the runtime consumes this file and does
not use it as writable project state.

It contains runtime defaults shared by every language:

```text
log_level=info
max_connections=1000
tls_enabled=true
cert_file=/etc/polycall/cert.pem
key_file=/etc/polycall/key.pem
```

## Language Overrides

`Polycallrc.<language>` is the language-specific read-only runtime override
layer. Canonical v1.0.1 names include:

```text
Polycallrc.node
Polycallrc.python
Polycallrc.java
Polycallrc.go
```

Example:

```text
port=8080:8084
server_type=node
workspace=/opt/polycall/node
supports_completion=true
timeout=30
```

`server_type` must match the filename language. `workspace` is required.
Examples are available in `config/examples/`.

## Legacy Compatibility

`.polycallrc` remains supported as a fallback when the requested canonical
`Polycallrc.<language>` file is absent. Loading it emits a migration warning.

Migrate a legacy file with:

```sh
polycall config migrate .polycallrc Polycallrc.node
```

The compatibility alias below is also accepted:

```sh
polycall config rc migrate .polycallrc Polycallrc.node
```

Migration validates the source and refuses to overwrite an existing
destination.

## CLI

```sh
polycall config load
polycall config validate
polycall config show
polycall config rc show node
polycall config rc validate node
```

`config load` and `config show` optionally accept a language to apply the
third layer:

```sh
polycall config load node
polycall config show node
```

`config validate <path>` validates one explicit file.

Validation behavior:

- malformed syntax fails
- invalid or out-of-range port mappings fail
- missing required fields fail
- invalid booleans and positive integers fail
- unknown keys produce warnings without failing
- `tls_enabled=true` requires `cert_file` and `key_file`

The XML files under `config/` are legacy schemas and are not part of this text
configuration loader. XML remains a serialization/schema concern rather than
the canonical writable project definition.

## Build Hygiene

The root Makefile records compiler path, compiler version, compiler target,
platform, architecture target, and compile flags in `build/.toolchain`.
Changing that fingerprint removes incompatible objects before compilation.

Every build also performs a relocatable-link check. Corrupt or foreign-format
objects are deleted and rebuilt automatically.

```sh
make clean
make rebuild
```

`make rebuild` removes compiled objects, dependency files, generated
libraries, and the `bin/polycall` executable before rebuilding all outputs.
