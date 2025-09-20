// OBINexus PID Manager - Process tracking and death watch
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define PID_FILE "/var/run/polycall.pid"
#define PID_FILE_ALT "/tmp/polycall.pid"
#define MAX_PID_LEN 16

typedef struct {
    pid_t pid;
    time_t start_time;
    char process_name[256];
} ProcessInfo;

static ProcessInfo g_process_info = {0};

// Write PID to file
int write_pid_file(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        // Try alternate location
        fp = fopen(PID_FILE_ALT, "w");
        if (!fp) {
            fprintf(stderr, "Failed to create PID file: %s\n", strerror(errno));
            return -1;
        }
        filename = PID_FILE_ALT;
    }
    
    pid_t pid = getpid();
    fprintf(fp, "%d\n", pid);
    fclose(fp);
    
    g_process_info.pid = pid;
    g_process_info.start_time = time(NULL);
    
    printf("[PID Manager] Process %d logged to %s\n", pid, filename);
    return 0;
}

// Check if process is running
int check_process_alive(pid_t pid) {
    return kill(pid, 0) == 0 ? 1 : 0;
}

// Read PID from file
pid_t read_pid_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fp = fopen(PID_FILE_ALT, "r");
        if (!fp) {
            return -1;
        }
    }
    
    char buf[MAX_PID_LEN];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    return (pid_t)atoi(buf);
}

// Remove PID file on exit
void remove_pid_file(void) {
    unlink(PID_FILE);
    unlink(PID_FILE_ALT);
    printf("[PID Manager] PID file removed\n");
}

// Death watch handler
static void death_watch_handler(int sig) {
    (void)sig;
    printf("[Death Watch] Process %d received signal %d\n", getpid(), sig);
    remove_pid_file();
    exit(0);
}

// Initialize death watch
void init_death_watch(void) {
    signal(SIGTERM, death_watch_handler);
    signal(SIGINT, death_watch_handler);
    signal(SIGHUP, death_watch_handler);
    
    // Register cleanup on normal exit
    atexit(remove_pid_file);
    
    printf("[Death Watch] Initialized for process %d\n", getpid());
}

// Check for stale PID file
int check_stale_pid(void) {
    pid_t old_pid = read_pid_file(PID_FILE);
    if (old_pid == -1) {
        old_pid = read_pid_file(PID_FILE_ALT);
    }
    
    if (old_pid > 0) {
        if (check_process_alive(old_pid)) {
            fprintf(stderr, "Another instance (PID %d) is already running\n", old_pid);
            return 1;
        } else {
            printf("[PID Manager] Removing stale PID file for process %d\n", old_pid);
            remove_pid_file();
        }
    }
    
    return 0;
}

// Initialize PID management
int pid_manager_init(const char* process_name) {
    strncpy(g_process_info.process_name, process_name, 
            sizeof(g_process_info.process_name) - 1);
    
    // Check for existing process
    if (check_stale_pid()) {
        return -1;
    }
    
    // Write new PID file
    if (write_pid_file(PID_FILE) < 0) {
        return -1;
    }
    
    // Initialize death watch
    init_death_watch();
    
    return 0;
}
