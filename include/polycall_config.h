#ifndef POLYCALL_CONFIG_H
#define POLYCALL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Handle the "polycall config ..." command family.
 * Returns -1 when argv is not a configuration command.
 */
int polycall_config_cli(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#endif /* POLYCALL_CONFIG_H */
