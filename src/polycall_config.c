#include "polycall_config.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POLYCALL_CONFIG_MAX_SERVERS 32
#define POLYCALL_CONFIG_MAX_VALUES 128
#define POLYCALL_CONFIG_KEY_SIZE 64
#define POLYCALL_CONFIG_VALUE_SIZE 256
#define POLYCALL_CONFIG_PATH_SIZE 512
#define POLYCALL_CONFIG_LINE_SIZE 1024

typedef enum {
    CONFIG_LAYER_PROJECT,
    CONFIG_LAYER_RUNTIME,
    CONFIG_LAYER_LANGUAGE
} ConfigLayer;

typedef struct {
    char language[32];
    unsigned int host_port;
    unsigned int target_port;
} ConfigServer;

typedef struct {
    char key[POLYCALL_CONFIG_KEY_SIZE];
    char value[POLYCALL_CONFIG_VALUE_SIZE];
} ConfigValue;

typedef struct {
    ConfigServer servers[POLYCALL_CONFIG_MAX_SERVERS];
    size_t server_count;
    ConfigValue values[POLYCALL_CONFIG_MAX_VALUES];
    size_t value_count;
    bool network_enabled;
    bool network_seen;
    unsigned int warnings;
} PolycallConfig;

typedef struct {
    char paths[3][POLYCALL_CONFIG_PATH_SIZE];
    size_t count;
    bool used_legacy;
} LoadedSources;

static const char* const PROJECT_KEYS[] = {
    "network_timeout",
    "max_connections",
    "log_directory",
    "workspace_root",
    "auto_discover",
    "discovery_interval",
    "tls_enabled",
    "cert_file",
    "key_file",
    "max_memory_per_service",
    "max_cpu_per_service",
    "enable_metrics",
    "metrics_port"
};

static const char* const RUNTIME_KEYS[] = {
    "port",
    "server_type",
    "workspace",
    "log_level",
    "max_connections",
    "supports_diagnostics",
    "supports_completion",
    "supports_formatting",
    "max_memory",
    "timeout",
    "allow_remote",
    "require_auth",
    "network_timeout",
    "log_directory",
    "workspace_root",
    "auto_discover",
    "discovery_interval",
    "tls_enabled",
    "cert_file",
    "key_file",
    "max_memory_per_service",
    "max_cpu_per_service",
    "enable_metrics",
    "metrics_port"
};

static const char* const BOOLEAN_KEYS[] = {
    "auto_discover",
    "tls_enabled",
    "enable_metrics",
    "supports_diagnostics",
    "supports_completion",
    "supports_formatting",
    "allow_remote",
    "require_auth"
};

static const char* const POSITIVE_INTEGER_KEYS[] = {
    "network_timeout",
    "max_connections",
    "discovery_interval",
    "max_cpu_per_service",
    "metrics_port",
    "timeout"
};

static char* trim(char* text) {
    char* end;

    while (*text && isspace((unsigned char)*text)) {
        text++;
    }

    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return text;
}

static bool file_exists(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}

static bool string_in_list(
    const char* value,
    const char* const* values,
    size_t value_count
) {
    size_t i;
    for (i = 0; i < value_count; i++) {
        if (strcmp(value, values[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_known_key(const char* key, ConfigLayer layer) {
    if (layer == CONFIG_LAYER_PROJECT) {
        return string_in_list(
            key,
            PROJECT_KEYS,
            sizeof(PROJECT_KEYS) / sizeof(PROJECT_KEYS[0])
        );
    }
    return string_in_list(
        key,
        RUNTIME_KEYS,
        sizeof(RUNTIME_KEYS) / sizeof(RUNTIME_KEYS[0])
    );
}

static bool parse_positive_integer(const char* value, unsigned long* result) {
    char* end = NULL;
    unsigned long parsed;

    if (!value || !*value || *value == '-') {
        return false;
    }

    errno = 0;
    parsed = strtoul(value, &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed == 0) {
        return false;
    }

    if (result) {
        *result = parsed;
    }
    return true;
}

static bool parse_port_mapping(
    const char* value,
    unsigned int* host_port,
    unsigned int* target_port
) {
    char buffer[64];
    char* separator;
    unsigned long host;
    unsigned long target;

    if (!value || strlen(value) >= sizeof(buffer)) {
        return false;
    }

    strcpy(buffer, value);
    separator = strchr(buffer, ':');
    if (!separator || strchr(separator + 1, ':')) {
        return false;
    }

    *separator = '\0';
    if (!parse_positive_integer(buffer, &host) ||
        !parse_positive_integer(separator + 1, &target) ||
        host > 65535 || target > 65535) {
        return false;
    }

    if (host_port) {
        *host_port = (unsigned int)host;
    }
    if (target_port) {
        *target_port = (unsigned int)target;
    }
    return true;
}

static const char* config_get(const PolycallConfig* config, const char* key) {
    size_t i;
    for (i = 0; i < config->value_count; i++) {
        if (strcmp(config->values[i].key, key) == 0) {
            return config->values[i].value;
        }
    }
    return NULL;
}

static bool config_set(
    PolycallConfig* config,
    const char* key,
    const char* value
) {
    size_t i;

    for (i = 0; i < config->value_count; i++) {
        if (strcmp(config->values[i].key, key) == 0) {
            snprintf(
                config->values[i].value,
                sizeof(config->values[i].value),
                "%s",
                value
            );
            return true;
        }
    }

    if (config->value_count >= POLYCALL_CONFIG_MAX_VALUES) {
        return false;
    }

    snprintf(
        config->values[config->value_count].key,
        sizeof(config->values[config->value_count].key),
        "%s",
        key
    );
    snprintf(
        config->values[config->value_count].value,
        sizeof(config->values[config->value_count].value),
        "%s",
        value
    );
    config->value_count++;
    return true;
}

static bool validate_value(
    const char* path,
    unsigned int line_number,
    const char* key,
    const char* value
) {
    unsigned long number;

    if (strcmp(key, "port") == 0) {
        if (!parse_port_mapping(value, NULL, NULL)) {
            fprintf(
                stderr,
                "%s:%u: invalid port mapping '%s' (expected 1-65535:1-65535)\n",
                path,
                line_number,
                value
            );
            return false;
        }
    }

    if (string_in_list(
            key,
            BOOLEAN_KEYS,
            sizeof(BOOLEAN_KEYS) / sizeof(BOOLEAN_KEYS[0])
        ) &&
        strcmp(value, "true") != 0 &&
        strcmp(value, "false") != 0) {
        fprintf(
            stderr,
            "%s:%u: '%s' must be true or false\n",
            path,
            line_number,
            key
        );
        return false;
    }

    if (string_in_list(
            key,
            POSITIVE_INTEGER_KEYS,
            sizeof(POSITIVE_INTEGER_KEYS) /
                sizeof(POSITIVE_INTEGER_KEYS[0])
        ) &&
        !parse_positive_integer(value, &number)) {
        fprintf(
            stderr,
            "%s:%u: '%s' must be a positive integer\n",
            path,
            line_number,
            key
        );
        return false;
    }

    if (strcmp(key, "metrics_port") == 0 &&
        (!parse_positive_integer(value, &number) || number > 65535)) {
        fprintf(
            stderr,
            "%s:%u: metrics_port must be between 1 and 65535\n",
            path,
            line_number
        );
        return false;
    }

    return true;
}

static bool add_server(
    PolycallConfig* config,
    const char* path,
    unsigned int line_number,
    const char* language,
    const char* mapping
) {
    size_t i;
    unsigned int host_port;
    unsigned int target_port;

    if (!*language || strlen(language) >= sizeof(config->servers[0].language)) {
        fprintf(stderr, "%s:%u: invalid server language\n", path, line_number);
        return false;
    }

    if (!parse_port_mapping(mapping, &host_port, &target_port)) {
        fprintf(
            stderr,
            "%s:%u: invalid server port mapping '%s'\n",
            path,
            line_number,
            mapping
        );
        return false;
    }

    for (i = 0; i < config->server_count; i++) {
        if (strcmp(config->servers[i].language, language) == 0) {
            fprintf(
                stderr,
                "%s:%u: duplicate server language '%s'\n",
                path,
                line_number,
                language
            );
            return false;
        }
    }

    if (config->server_count >= POLYCALL_CONFIG_MAX_SERVERS) {
        fprintf(stderr, "%s:%u: too many server definitions\n", path, line_number);
        return false;
    }

    snprintf(
        config->servers[config->server_count].language,
        sizeof(config->servers[config->server_count].language),
        "%s",
        language
    );
    config->servers[config->server_count].host_port = host_port;
    config->servers[config->server_count].target_port = target_port;
    config->server_count++;
    return true;
}

static bool parse_config_file(
    PolycallConfig* config,
    const char* path,
    ConfigLayer layer
) {
    FILE* file;
    char raw_line[POLYCALL_CONFIG_LINE_SIZE];
    unsigned int line_number = 0;

    file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Unable to open configuration file: %s\n", path);
        return false;
    }

    while (fgets(raw_line, sizeof(raw_line), file)) {
        char* line;
        char* comment;
        char* equals;

        line_number++;
        if (!strchr(raw_line, '\n') && !feof(file)) {
            fprintf(stderr, "%s:%u: line is too long\n", path, line_number);
            fclose(file);
            return false;
        }

        comment = strchr(raw_line, '#');
        if (comment) {
            *comment = '\0';
        }
        line = trim(raw_line);
        if (!*line) {
            continue;
        }

        if (strncmp(line, "server", 6) == 0 &&
            isspace((unsigned char)line[6])) {
            char language[32];
            char mapping[64];
            char extra[2];

            if (layer != CONFIG_LAYER_PROJECT ||
                sscanf(line, "server %31s %63s %1s", language, mapping, extra) != 2 ||
                !add_server(config, path, line_number, language, mapping)) {
                if (layer != CONFIG_LAYER_PROJECT) {
                    fprintf(
                        stderr,
                        "%s:%u: server definitions belong in Polycallfile\n",
                        path,
                        line_number
                    );
                } else if (sscanf(
                               line,
                               "server %31s %63s %1s",
                               language,
                               mapping,
                               extra
                           ) != 2) {
                    fprintf(
                        stderr,
                        "%s:%u: expected 'server <language> <host>:<target>'\n",
                        path,
                        line_number
                    );
                }
                fclose(file);
                return false;
            }
            continue;
        }

        if (strncmp(line, "network", 7) == 0 &&
            isspace((unsigned char)line[7])) {
            char action[16];
            char extra[2];

            if (layer != CONFIG_LAYER_PROJECT ||
                sscanf(line, "network %15s %1s", action, extra) != 1 ||
                (strcmp(action, "start") != 0 &&
                 strcmp(action, "stop") != 0)) {
                fprintf(
                    stderr,
                    "%s:%u: expected 'network start' or 'network stop' in Polycallfile\n",
                    path,
                    line_number
                );
                fclose(file);
                return false;
            }
            config->network_enabled = strcmp(action, "start") == 0;
            config->network_seen = true;
            continue;
        }

        equals = strchr(line, '=');
        if (equals) {
            char* key;
            char* value;

            if (strchr(equals + 1, '=')) {
                fprintf(stderr, "%s:%u: malformed key=value entry\n", path, line_number);
                fclose(file);
                return false;
            }
            *equals = '\0';
            key = trim(line);
            value = trim(equals + 1);
            if (!*key || !*value || strlen(key) >= POLYCALL_CONFIG_KEY_SIZE ||
                strlen(value) >= POLYCALL_CONFIG_VALUE_SIZE) {
                fprintf(stderr, "%s:%u: malformed key=value entry\n", path, line_number);
                fclose(file);
                return false;
            }
            if (!is_known_key(key, layer)) {
                fprintf(
                    stderr,
                    "%s:%u: warning: unknown key '%s'\n",
                    path,
                    line_number,
                    key
                );
                config->warnings++;
            }
            if (!validate_value(path, line_number, key, value) ||
                !config_set(config, key, value)) {
                if (config->value_count >= POLYCALL_CONFIG_MAX_VALUES) {
                    fprintf(stderr, "%s:%u: too many configuration values\n", path, line_number);
                }
                fclose(file);
                return false;
            }
            continue;
        }

        fprintf(stderr, "%s:%u: malformed configuration syntax\n", path, line_number);
        fclose(file);
        return false;
    }

    if (ferror(file)) {
        fprintf(stderr, "Failed while reading configuration file: %s\n", path);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static bool validate_required(
    const PolycallConfig* config,
    bool require_project_fields,
    const char* language
) {
    const char* tls_enabled;
    const char* server_type;
    bool valid = true;

    if (require_project_fields) {
        if (!config_get(config, "workspace_root")) {
            fprintf(stderr, "Missing required field: workspace_root\n");
            valid = false;
        }
        if (!config_get(config, "log_directory")) {
            fprintf(stderr, "Missing required field: log_directory\n");
            valid = false;
        }
    }

    tls_enabled = config_get(config, "tls_enabled");
    if (tls_enabled && strcmp(tls_enabled, "true") == 0) {
        if (!config_get(config, "cert_file")) {
            fprintf(stderr, "Missing required field: cert_file (tls_enabled=true)\n");
            valid = false;
        }
        if (!config_get(config, "key_file")) {
            fprintf(stderr, "Missing required field: key_file (tls_enabled=true)\n");
            valid = false;
        }
    }

    if (language) {
        if (!config_get(config, "workspace")) {
            fprintf(stderr, "Missing required field: workspace\n");
            valid = false;
        }
        server_type = config_get(config, "server_type");
        if (!server_type) {
            fprintf(stderr, "Missing required field: server_type\n");
            valid = false;
        } else if (strcmp(server_type, language) != 0) {
            fprintf(
                stderr,
                "server_type '%s' does not match Polycallrc.%s\n",
                server_type,
                language
            );
            valid = false;
        }
    }

    return valid;
}

static bool valid_language(const char* language) {
    const unsigned char* cursor = (const unsigned char*)language;
    if (!language || !*language) {
        return false;
    }
    while (*cursor) {
        if (!isalnum(*cursor) && *cursor != '-' && *cursor != '_') {
            return false;
        }
        cursor++;
    }
    return true;
}

static bool record_source(LoadedSources* sources, const char* path) {
    if (sources->count >= sizeof(sources->paths) / sizeof(sources->paths[0])) {
        return false;
    }
    snprintf(
        sources->paths[sources->count],
        sizeof(sources->paths[sources->count]),
        "%s",
        path
    );
    sources->count++;
    return true;
}

static bool load_hierarchy(
    PolycallConfig* config,
    const char* language,
    LoadedSources* sources
) {
    char language_path[POLYCALL_CONFIG_PATH_SIZE];

    if (!file_exists("Polycallfile")) {
        fprintf(stderr, "Missing project configuration: Polycallfile\n");
        return false;
    }
    if (!parse_config_file(config, "Polycallfile", CONFIG_LAYER_PROJECT) ||
        !record_source(sources, "Polycallfile")) {
        return false;
    }

    if (file_exists("Polycallrc")) {
        if (!parse_config_file(config, "Polycallrc", CONFIG_LAYER_RUNTIME) ||
            !record_source(sources, "Polycallrc")) {
            return false;
        }
    }

    if (language) {
        if (!valid_language(language)) {
            fprintf(stderr, "Invalid language name: %s\n", language);
            return false;
        }
        snprintf(language_path, sizeof(language_path), "Polycallrc.%s", language);
        if (file_exists(language_path)) {
            if (!parse_config_file(config, language_path, CONFIG_LAYER_LANGUAGE) ||
                !record_source(sources, language_path)) {
                return false;
            }
        } else if (file_exists(".polycallrc")) {
            fprintf(
                stderr,
                "warning: Legacy .polycallrc detected. "
                "Use Polycallrc.<language> for v1.0.1.\n"
            );
            if (!parse_config_file(config, ".polycallrc", CONFIG_LAYER_LANGUAGE) ||
                !record_source(sources, ".polycallrc")) {
                return false;
            }
            sources->used_legacy = true;
        } else {
            fprintf(stderr, "Missing runtime override: %s\n", language_path);
            return false;
        }
    }

    return validate_required(config, true, language);
}

static void show_config(const PolycallConfig* config) {
    size_t i;

    for (i = 0; i < config->server_count; i++) {
        printf(
            "server %s %u:%u\n",
            config->servers[i].language,
            config->servers[i].host_port,
            config->servers[i].target_port
        );
    }
    if (config->network_seen) {
        printf("network %s\n", config->network_enabled ? "start" : "stop");
    }
    for (i = 0; i < config->value_count; i++) {
        printf("%s=%s\n", config->values[i].key, config->values[i].value);
    }
}

static ConfigLayer infer_layer(const char* path, const char** language) {
    const char* name = strrchr(path, '/');
    const char* windows_name = strrchr(path, '\\');
    const char* suffix;

    if (!name || (windows_name && windows_name > name)) {
        name = windows_name;
    }
    name = name ? name + 1 : path;

    if (strcmp(name, "Polycallfile") == 0) {
        return CONFIG_LAYER_PROJECT;
    }
    suffix = strstr(name, "Polycallrc.");
    if (suffix == name && suffix[11] != '\0') {
        *language = suffix + 11;
        return CONFIG_LAYER_LANGUAGE;
    }
    if (strcmp(name, ".polycallrc") == 0) {
        return CONFIG_LAYER_LANGUAGE;
    }
    return CONFIG_LAYER_RUNTIME;
}

static int validate_one(const char* path) {
    PolycallConfig config = {0};
    const char* language = NULL;
    ConfigLayer layer = infer_layer(path, &language);

    if (!parse_config_file(&config, path, layer) ||
        !validate_required(
            &config,
            layer == CONFIG_LAYER_PROJECT,
            language
        )) {
        return 1;
    }

    printf("%s is valid", path);
    if (config.warnings) {
        printf(" with %u warning(s)", config.warnings);
    }
    printf("\n");
    return 0;
}

static int load_and_report(const char* language, bool show) {
    PolycallConfig config = {0};
    LoadedSources sources = {0};
    size_t i;

    if (!load_hierarchy(&config, language, &sources)) {
        return 1;
    }

    if (show) {
        show_config(&config);
    } else {
        printf("Loaded configuration in order:\n");
        for (i = 0; i < sources.count; i++) {
            printf("%u. %s\n", (unsigned int)(i + 1), sources.paths[i]);
        }
        printf("Configuration is valid");
        if (config.warnings) {
            printf(" with %u warning(s)", config.warnings);
        }
        printf("\n");
    }
    return 0;
}

static int show_rc(const char* language, bool validate_only) {
    PolycallConfig config = {0};
    char path[POLYCALL_CONFIG_PATH_SIZE];
    const char* selected_path;

    if (!valid_language(language)) {
        fprintf(stderr, "Invalid language name: %s\n", language ? language : "");
        return 1;
    }

    snprintf(path, sizeof(path), "Polycallrc.%s", language);
    selected_path = path;
    if (!file_exists(selected_path)) {
        if (!file_exists(".polycallrc")) {
            fprintf(stderr, "Missing runtime override: %s\n", path);
            return 1;
        }
        selected_path = ".polycallrc";
        fprintf(
            stderr,
            "warning: Legacy .polycallrc detected. "
            "Use Polycallrc.<language> for v1.0.1.\n"
        );
    }

    if (!parse_config_file(&config, selected_path, CONFIG_LAYER_LANGUAGE) ||
        !validate_required(&config, false, language)) {
        return 1;
    }

    if (validate_only) {
        printf("%s is valid for %s\n", selected_path, language);
    } else {
        show_config(&config);
    }
    return 0;
}

static int migrate_config(const char* source, const char* destination) {
    PolycallConfig config = {0};
    const char* language = NULL;
    ConfigLayer destination_layer;
    FILE* input;
    FILE* output;
    char buffer[4096];
    size_t read_count;

    if (file_exists(destination)) {
        fprintf(stderr, "Refusing to overwrite existing file: %s\n", destination);
        return 1;
    }

    destination_layer = infer_layer(destination, &language);
    if (destination_layer != CONFIG_LAYER_LANGUAGE || !language ||
        !valid_language(language)) {
        fprintf(
            stderr,
            "Migration destination must be Polycallrc.<language>\n"
        );
        return 1;
    }

    if (!parse_config_file(&config, source, CONFIG_LAYER_LANGUAGE) ||
        !validate_required(&config, false, language)) {
        return 1;
    }

    input = fopen(source, "rb");
    if (!input) {
        fprintf(stderr, "Unable to open migration source: %s\n", source);
        return 1;
    }
    output = fopen(destination, "wb");
    if (!output) {
        fprintf(stderr, "Unable to create migration destination: %s\n", destination);
        fclose(input);
        return 1;
    }

    while ((read_count = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (fwrite(buffer, 1, read_count, output) != read_count) {
            fprintf(stderr, "Failed to write migration destination: %s\n", destination);
            fclose(input);
            fclose(output);
            remove(destination);
            return 1;
        }
    }

    if (ferror(input) || fclose(output) != 0) {
        fprintf(stderr, "Failed to migrate configuration to: %s\n", destination);
        fclose(input);
        remove(destination);
        return 1;
    }
    fclose(input);

    printf("Migrated %s to %s\n", source, destination);
    return 0;
}

static void print_config_usage(void) {
    printf("PolyCall configuration commands:\n");
    printf("  polycall config load [language]\n");
    printf("  polycall config validate [path]\n");
    printf("  polycall config show [language]\n");
    printf("  polycall config rc show <language>\n");
    printf("  polycall config rc validate <language>\n");
    printf("  polycall config migrate <source> <Polycallrc.language>\n");
}

int polycall_config_cli(int argc, char* argv[]) {
    if (argc < 2 || strcmp(argv[1], "config") != 0) {
        return -1;
    }

    if (argc < 3) {
        print_config_usage();
        return 1;
    }

    if (strcmp(argv[2], "load") == 0) {
        if (argc > 4) {
            print_config_usage();
            return 1;
        }
        return load_and_report(argc == 4 ? argv[3] : NULL, false);
    }

    if (strcmp(argv[2], "validate") == 0) {
        if (argc == 3) {
            return load_and_report(NULL, false);
        }
        if (argc == 4) {
            return validate_one(argv[3]);
        }
        print_config_usage();
        return 1;
    }

    if (strcmp(argv[2], "show") == 0) {
        if (argc > 4) {
            print_config_usage();
            return 1;
        }
        return load_and_report(argc == 4 ? argv[3] : NULL, true);
    }

    if (strcmp(argv[2], "migrate") == 0 && argc == 5) {
        return migrate_config(argv[3], argv[4]);
    }

    if (strcmp(argv[2], "rc") == 0) {
        if (argc == 6 && strcmp(argv[3], "migrate") == 0) {
            return migrate_config(argv[4], argv[5]);
        }
        if (argc != 5) {
            print_config_usage();
            return 1;
        }
        if (strcmp(argv[3], "show") == 0) {
            return show_rc(argv[4], false);
        }
        if (strcmp(argv[3], "validate") == 0) {
            return show_rc(argv[4], true);
        }
    }

    print_config_usage();
    return 1;
}
