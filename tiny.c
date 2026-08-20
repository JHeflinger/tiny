/*
* author: Jason Heflinger
* description: Tiny GCC project manager
*/

#define VERSION 1
#define MAJOR_RELEASE 2
#define MINOR_RELEASE 3

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>

#ifdef __linux__
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <dirent.h>
#elif __WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
    #define NOVIRTUALKEYCODES // VK_*
    #define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
    #define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
    #define NOSYSMETRICS      // SM_*
    #define NOMENUS           // MF_*
    #define NOICONS           // IDI_*
    #define NOKEYSTATES       // MK_*
    #define NOSYSCOMMANDS     // SC_*
    #define NORASTEROPS       // Binary and Tertiary raster ops
    #define NOSHOWWINDOW      // SW_*
    #define OEMRESOURCE       // OEM Resource values
    #define NOATOM            // Atom Manager routines
    #define NOCLIPBOARD       // Clipboard routines
    #define NOCOLOR           // Screen colors
    #define NOCTLMGR          // Control and Dialog routines
    #define NODRAWTEXT        // DrawText() and DT_*
    #define NOGDI             // All GDI defines and routines
    #define NOKERNEL          // All KERNEL defines and routines
    #define NOUSER            // All USER defines and routines
    #define NOMB              // MB_* and MessageBox()
    #define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
    #define NOMETAFILE        // typedef METAFILEPICT
    #define NOMSG             // typedef MSG and associated routines
    #define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
    #define NOSCROLL          // SB_* and scrolling routines
    #define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
    #define NOSOUND           // Sound driver routines
    #define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
    #define NOWH              // SetWindowsHook and WH_*
    #define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
    #define NOCOMM            // COMM driver routines
    #define NOKANJI           // Kanji support stuff.
    #define NOHELP            // Help engine interface.
    #define NOPROFILER        // Profiler interface.
    #define NODEFERWINDOWPOS  // DeferWindowPos routines
    #define NOMCX             // Modem Configuration Extensions
    #include <windows.h>
    #include <direct.h>
#elif __APPLE__
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <pthread.h>
    #include <limits.h>
    #include <unistd.h>
    #include <dirent.h>
#else
    #error "Unsupported operating system detected!"
#endif

#define print(...) {printf(__VA_ARGS__);printf("\n");}
#define crash(...) {printf("\033[31m[ERROR]\033[0m "); print(__VA_ARGS__);exit(1);}
#define warn(...) {printf("\033[33m[WARNING]\033[0m "); print(__VA_ARGS__);}
#define PATHLEN 4096

#ifdef __linux__
    #define PATH_SEP '/'
    #define cwd(buffer) getcwd(buffer, sizeof(buffer))
    #define makedir(dir) (!mkdir(dir, 0755))
    #define TINY_THREAD_RETURN_TYPE void*
    #define TINY_THREAD_PARAMETER_TYPE void*
    #define TINY_CREATE_THREAD(thread, func, parameters) pthread_create(&thread, NULL, (void* (*)(void*))func, parameters)
    #define TINY_WAIT_THREAD(thread) pthread_join(thread, NULL)
    #define TINY_CREATE_MUTEX(mutex) pthread_mutex_init(&mutex, NULL);
    #define TINY_LOCK_MUTEX(mutex) pthread_mutex_lock(&mutex)
    #define TINY_RELEASE_MUTEX(mutex) pthread_mutex_unlock(&mutex)
    #define TINY_CREATE_COND(cond) pthread_cond_init(&cond, NULL);
    #define TINY_WAIT_COND(cond, mutex) pthread_cond_wait(&cond, &mutex)
    #define TINY_SIGNAL_COND(cond) pthread_cond_signal(&cond)
    #define TINY_BROADCAST_COND(cond) pthread_cond_broadcast(&cond)
#elif __WIN32
    #define PATH_SEP '\\'
    #define cwd(buffer) _getcwd(buffer, sizeof(buffer))
    #define makedir(dir) (!_mkdir(dir))
    #define TINY_THREAD_RETURN_TYPE DWORD WINAPI
    #define TINY_THREAD_PARAMETER_TYPE LPVOID
    #define TINY_CREATE_THREAD(thread, func, parameters) { thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)parameters, 0, NULL); }
    #define TINY_WAIT_THREAD(thread) {WaitForSingleObject(thread, INFINITE); CloseHandle(thread);}
    #define TINY_CREATE_MUTEX(mutex) InitializeCriticalSection(&mutex);
    #define TINY_LOCK_MUTEX(mutex) EnterCriticalSection(&mutex)
    #define TINY_RELEASE_MUTEX(mutex) LeaveCriticalSection(&mutex)
    #define TINY_CREATE_COND(cond) InitializeConditionVariable(&cond);
    #define TINY_WAIT_COND(cond, mutex) SleepConditionVariableCS(&cond, &mutex, INFINITE);
    #define TINY_SIGNAL_COND(cond) WakeConditionVariable(&cond)
    #define TINY_BROADCAST_COND(cond) WakeAllConditionVariable(&cond)
#elif __APPLE__
    #define PATH_SEP '/'
    #define cwd(buffer) getcwd(buffer, sizeof(buffer))
    #define makedir(dir) (!mkdir(dir, 0755))
    #define TINY_THREAD_RETURN_TYPE void*
    #define TINY_THREAD_PARAMETER_TYPE void*
    #define TINY_CREATE_THREAD(thread, func, parameters) pthread_create(&thread, NULL, (void* (*)(void*))func, parameters)
    #define TINY_WAIT_THREAD(thread) pthread_join(thread, NULL)
    #define TINY_CREATE_MUTEX(mutex) pthread_mutex_init(&mutex, NULL);
    #define TINY_LOCK_MUTEX(mutex) pthread_mutex_lock(&mutex)
    #define TINY_RELEASE_MUTEX(mutex) pthread_mutex_unlock(&mutex)
    #define TINY_CREATE_COND(cond) pthread_cond_init(&cond, NULL);
    #define TINY_WAIT_COND(cond, mutex) pthread_cond_wait(&cond, &mutex)
    #define TINY_SIGNAL_COND(cond) pthread_cond_signal(&cond)
    #define TINY_BROADCAST_COND(cond) pthread_cond_broadcast(&cond)
#else
    #error "Unsupported operating system detected!"
#endif

typedef void (*FileHandler)(const char*);

typedef enum {
    NONE = 0,
    PROD = 1 << 0,
    AUDIT = 1 << 1,
    FAST = 1 << 2,
    DEBUG = 1 << 3,
    RECOMPILE_VENDORS = 1 << 4,
    RUN = 1 << 5,
    CLEAN = 1 << 6
} BuildFlags;

typedef struct {
    char str[PATHLEN];
    void* next;
} PathList;

typedef struct {
    char header[PATHLEN];
    PathList* links;
    PathList* secondaries;
} HeaderLink;

typedef struct {
    HeaderLink* link;
    void* next;
} HeaderLinkList;

typedef struct {
    char* command;
    char destination[PATHLEN];
    char file[PATHLEN];
    int index;
    int basename_ptr;
    int sourcei;
} ThreadParameters;

typedef struct {
    char name[PATHLEN];
    char url[PATHLEN];
    char path[PATHLEN];
    int integrated;
} Module;

typedef struct {
    Module module;
    void* next;
} ModuleList;

#ifdef __linux__
    typedef pthread_t TINY_THREAD;
    typedef pthread_mutex_t TINY_MUTEX;
    typedef pthread_cond_t TINY_COND;
#elif __WIN32
    typedef HANDLE TINY_THREAD;
    typedef CRITICAL_SECTION TINY_MUTEX;
    typedef CONDITION_VARIABLE TINY_COND;
#elif __APPLE__
    typedef pthread_t TINY_THREAD;
    typedef pthread_mutex_t TINY_MUTEX;
    typedef pthread_cond_t TINY_COND;
#else
    #error "Unsupported operating system detected!"
#endif

void run_build();
int make_symlink(const char* src, const char* dest);
int copytree(const char* src, const char* dest);
void rmtree(const char* path);
char* generate_quote(const char* s);
int runcmd(const char* cmd);
int threadcount();
int dexists(const char* dir);
int fexists(const char* file);
void walkdir(const char* path, FileHandler func);
void walkfiles(const char* path, FileHandler func);
uint64_t mtime();
void dissect_time_elapsed(uint64_t time, int* hours, int* minutes, float* seconds);
void integrate_modules();
void dissect_module(const char* str);
void download_module(const char* name, const char* url, const char* path);
int rmakedir(const char* dir);
void modulelist_add(ModuleList** list, Module module);
void modulelist_delete(ModuleList* list);
void pathlist_add(PathList** list, const char* path);
void pathlist_delete(PathList* list);
size_t pathlist_len(PathList* list);
void pathlist_construct(PathList* list, char* output);
void clean_header_links();
void clean_source_links();
int header_link_exists(const char* header);
void add_header_link(const char* header, const char* link);
void add_source_link(const char* header, const char* link);
void add_header_secondary(const char* header, const char* link);
int functionline(const char* line);
int functionimplline(const char* line);
int vardeclared(const char* line);
void easyc_audit(const char* file);
void syntax_audit(const char* file);
void copyfile(const char* src, const char* dst);
int filecmp(const char* path1, const char* path2);
void affirmdir(const char* dir);
void affirm_to_cache(const char* dir);
void add_to_sources(const char* file);
void verify_header(const char* file);
void accumulate_header(const char* file);
void async_compile_progress_update(int index, int action, const char* name);
void async_compile(void* params);
void compile_source(const char* file);
void parseflag(char* flag, int blacklistable);
void configure(const char* prepath, const char* path);
void affirm_projects();
void initialize(int argc, char* argv[]);
void compile_vendors();
void calculate_dependencies();
void compile_objects();
void compile_executable();
void get_in_depth_headers(const char* dive_header, HeaderLinkList* update_header);
void audit();
void port_folder(const char* path);

size_t s_start_time = 0;
BuildFlags s_flags = NONE;
BuildFlags s_unflags = NONE;
char s_main_file_name[PATHLEN];
int s_found_main = 0;
int s_sources_up_to_date = 1;
int s_main_up_to_date = 1;
int s_vulnerabilities = 0;
char s_main_file_path[PATHLEN] = { 0 };
char s_cwd[PATHLEN] = { 0 };
PathList* s_projects = NULL;
PathList* s_includes = NULL;
PathList* s_links = NULL;
PathList* s_raws = NULL;
PathList* s_defines = NULL;
PathList* s_libs = NULL;
PathList* s_sources = NULL;
PathList* s_objects = NULL;
PathList* s_changed_headers = NULL;
HeaderLinkList* s_header_links = NULL;
HeaderLinkList* s_source_links = NULL;
TINY_THREAD* s_threads = NULL;
int* s_active_threads = NULL;
TINY_MUTEX s_mutex;
int s_sourcei = 0;
int s_easymemory_detected = 0;
ModuleList* s_modules = NULL;
char** s_copy_argsv = NULL;
int s_copy_argsc = 0;
int s_max_argsc = 0;

#ifdef __linux__
    void run_build() {
        affirmdir("build/env");
        pid_t pid = fork();
        if (pid < 0) {
            crash("Fork error");
        }
        if (pid == 0) {
            if (chdir("build/env") != 0) {
                crash("Unable to enter build environment");
            }
            s_copy_argsv[s_copy_argsc] = NULL;
            execv("../bin.exe", s_copy_argsv);
            crash("Unable to run executable");
        }
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            crash("Unable to wait on executable cleanup");
        }
    }

    int make_symlink(const char* src, const char* dest) {
        return symlink(src, dest) == 0;
    }

    int copytree(const char* src, const char* dest) {
        char* sq = generate_quote(src);
        char* dq = generate_quote(dest);
        if (!sq || !dq) return 0;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "cp -R %s %s", sq, dq);
        int copy_ok = runcmd(cmd) == 0;
        free(sq);
        free(dq);
        return copy_ok;
    }

    void rmtree(const char* path) {
        char* q = generate_quote(path);
        if (!q) return;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "rm -rf %s >/dev/null 2>&1", q);
        system(cmd);
        free(q);
    }

    char* generate_quote(const char* s) {
        size_t len = strlen(s);
        char* out = (char*)calloc(len * 4 + 3, sizeof(char));
        if (!out) return NULL;
        size_t o = 0;
        out[o++] = '\'';
        for (size_t i = 0; i < len; i++) {
            if (s[i] == '\'') {
                out[o++] = '\'';
                out[o++] = '\\';
                out[o++] = '\'';
                out[o++] = '\'';
            } else {
                out[o++] = s[i];
            }
        }
        out[o++] = '\'';
        out[o] = '\0';
        return out;
    }

    int runcmd(const char *cmd) {
        int rc = system(cmd);
        if (rc == -1) return -1;
        if (WIFEXITED(rc)) return WEXITSTATUS(rc);
        return -1;
    }

    int threadcount() {
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
    }

    int dexists(const char* dir) {
        struct stat statbuf;
        if (stat(dir, &statbuf) != 0) {
            return 0;
        }
        return S_ISDIR(statbuf.st_mode);
    }

    int fexists(const char* file) {
        struct stat statbuf;
        if (stat(file, &statbuf) != 0) {
            return 0;
        }
        return !S_ISDIR(statbuf.st_mode);
    }

    void walkdir(const char* path, FileHandler func) {
        DIR *dir = opendir(path);
        if (!dir) {
            crash("Unable to open directory \"%s\"", path);
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat statbuf;
            if (stat(full_path, &statbuf) != 0) {
                crash("Stat call failed");
            }
            if (S_ISDIR(statbuf.st_mode)) {
                func(full_path);
                walkdir(full_path, func);
            }
        }
        closedir(dir);
    }

    void walkfiles(const char* path, FileHandler func) {
        DIR *dir = opendir(path);
        if (!dir) {
            crash("Unable to open directory \"%s\"", path);
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat statbuf;
            if (stat(full_path, &statbuf) != 0) {
                crash("Stat call failed");
            }
            if (!S_ISDIR(statbuf.st_mode)) {
                func(full_path);
            } else {
                walkfiles(full_path, func);
            }
        }
        closedir(dir);
    }

    uint64_t mtime() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    }
#elif __WIN32
    void run_build() {
        affirmdir("build/env");
        char command_line[PATHLEN * 2] = { 0 };
        int offset = 0;
        for (int i = 0; i < s_copy_argsc; i++) {
            offset += snprintf(command_line + offset, sizeof(command_line) - offset, "\"%s%s\"%c", i == 0 ? "build/" : "", s_copy_argsv[i], i == s_copy_argsc - 1 ? '\0' : ' ');
        }
        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        if (!CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, "build/env", &si, &pi)) {
            crash("CreateProcess failed");
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    int make_symlink(const char* src, const char* dest) {
        DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
        DWORD attrs = GetFileAttributesA(src + 6);
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            crash("GetFileAttributes failed on dir \"%s\"", src + 6);
        }
        if (attrs & FILE_ATTRIBUTE_DIRECTORY)
            flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
        return CreateSymbolicLinkA(dest, src, flags);
    }

    int copytree(const char* src, const char* dest) {
        char* sq = generate_quote(src);
        char* dq = generate_quote(dest);
        if (!sq || !dq) return 0;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "robocopy %s %s /E /NFL /NDL /NJH /NJS /NC /NS >NUL 2>&1", sq, dq);
        int rc = runcmd(cmd);
        int copy_ok = (rc >= 0 && rc < 8);
        free(sq);
        free(dq);
        return copy_ok;
    }

    void rmtree(const char* path) {
        char* q = generate_quote(path);
        if (!q) return;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "rmdir /S /Q %s >NUL 2>&1", q);
        system(cmd);
        free(q);
    }


    char* generate_quote(const char* s) {
        size_t len = strlen(s);
        char* out = (char*)calloc(len * 4 + 3, sizeof(char));
        if (!out) return NULL;
        size_t o = 0;
        out[o++] = '\"';
        for (size_t i = 0; i < len; i++) {
            if (s[i] == '\"') {
                out[o++] = '\\';
                out[o++] = '\"';
            } else if (s[i] == '\\') {
                out[o++] = '\\';
                out[o++] = '\\';
            }
            else {
                out[o++] = s[i];
            }
        }
        out[o++] = '\"';
        out[o] = '\0';
        return out;
    }

    int runcmd(const char *cmd) {
        int rc = system(cmd);
        return rc;
    }

    int threadcount() {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return (int)sysinfo.dwNumberOfProcessors;
    }

    int dexists(const char* dir) {
        struct _stat statbuf;
        if (_stat(dir, &statbuf) != 0) {
            return 0;
        }
        return (statbuf.st_mode & _S_IFDIR) != 0;
    }

    int fexists(const char* file) {
        struct _stat statbuf;
        if (_stat(file, &statbuf) != 0) {
            return 0;
        }
        return (statbuf.st_mode & _S_IFDIR) == 0;
    }

    void walkdir(const char* path, FileHandler func) {
        char search_path[MAX_PATH];
        snprintf(search_path, MAX_PATH, "%s/*", path);
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA(search_path, &find_data);
        if (hFind == INVALID_HANDLE_VALUE) {
            crash("Unable to open directory \"%s\"", path);
        }
        do {
            const char *name = find_data.cFileName;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            char full_path[MAX_PATH];
            snprintf(full_path, MAX_PATH, "%s/%s", path, name);
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                func(full_path);
                walkdir(full_path, func);
            }
        } while (FindNextFileA(hFind, &find_data) != 0);
        FindClose(hFind);
    }

    void walkfiles(const char* path, FileHandler func) {
        char search_path[MAX_PATH];
        snprintf(search_path, MAX_PATH, "%s/*", path);
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA(search_path, &find_data);
        if (hFind == INVALID_HANDLE_VALUE) {
            crash("Unable to open directory \"%s\"", path);
        }
        do {
            const char *name = find_data.cFileName;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            char full_path[MAX_PATH];
            snprintf(full_path, MAX_PATH, "%s/%s", path, name);
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                func(full_path);
            } else {
                walkfiles(full_path, func);
            }
        } while (FindNextFileA(hFind, &find_data) != 0);
        FindClose(hFind);
    }

    uint64_t mtime() {
        return (uint64_t)GetTickCount64();
    }
#elif __APPLE__
    void run_build() {
        affirmdir("build/env");
        pid_t pid = fork();
        if (pid < 0) {
            crash("Fork error");
        }
        if (pid == 0) {
            if (chdir("build/env") != 0) {
                crash("Unable to enter build environment");
            }
            s_copy_argsv[s_copy_argsc] = NULL;
            execv("../bin.exe", s_copy_argsv);
            crash("Unable to run executable");
        }
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            crash("Unable to wait on executable cleanup");
        }
    }

    int make_symlink(const char* src, const char* dest) {
        return symlink(src, dest) == 0;
    }

    int copytree(const char* src, const char* dest) {
        char* sq = generate_quote(src);
        char* dq = generate_quote(dest);
        if (!sq || !dq) return 0;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "cp -R %s %s", sq, dq);
        int copy_ok = runcmd(cmd) == 0;
        free(sq);
        free(dq);
        return copy_ok;
    }

    void rmtree(const char* path) {
        char* q = generate_quote(path);
        if (!q) return;
        char cmd[PATHLEN];
        snprintf(cmd, PATHLEN, "rm -rf %s >/dev/null 2>&1", q);
        system(cmd);
        free(q);
    }

    char* generate_quote(const char* s) {
        size_t len = strlen(s);
        char* out = (char*)calloc(len * 4 + 3, sizeof(char));
        if (!out) return NULL;
        size_t o = 0;
        out[o++] = '\'';
        for (size_t i = 0; i < len; i++) {
            if (s[i] == '\'') {
                out[o++] = '\'';
                out[o++] = '\\';
                out[o++] = '\'';
                out[o++] = '\'';
            } else {
                out[o++] = s[i];
            }
        }
        out[o++] = '\'';
        out[o] = '\0';
        return out;
    }

    int runcmd(const char *cmd) {
        int rc = system(cmd);
        if (rc == -1) return -1;
        if (WIFEXITED(rc)) return WEXITSTATUS(rc);
        return -1;
    }

    int threadcount() {
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
    }

    int dexists(const char* dir) {
        struct stat statbuf;
        if (stat(dir, &statbuf) != 0) {
            return 0;
        }
        return S_ISDIR(statbuf.st_mode);
    }

    int fexists(const char* file) {
        struct stat statbuf;
        if (stat(file, &statbuf) != 0) {
            return 0;
        }
        return !S_ISDIR(statbuf.st_mode);
    }

    void walkdir(const char* path, FileHandler func) {
        DIR *dir = opendir(path);
        if (!dir) {
            crash("Unable to open directory \"%s\"", path);
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat statbuf;
            if (stat(full_path, &statbuf) != 0) {
                crash("Stat call failed");
            }
            if (S_ISDIR(statbuf.st_mode)) {
                func(full_path);
                walkdir(full_path, func);
            }
        }
        closedir(dir);
    }

    void walkfiles(const char* path, FileHandler func) {
        DIR *dir = opendir(path);
        if (!dir) {
            crash("Unable to open directory \"%s\"", path);
        }
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            struct stat statbuf;
            if (stat(full_path, &statbuf) != 0) {
                crash("Stat call failed");
            }
            if (!S_ISDIR(statbuf.st_mode)) {
                func(full_path);
            } else {
                walkfiles(full_path, func);
            }
        }
        closedir(dir);
    }

    uint64_t mtime() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t)(tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    }
#else
    #error "Unsupported operating system detected!"
#endif

void dissect_time_elapsed(uint64_t time, int* hours, int* minutes, float* seconds) {
    time = mtime() - time;
    *hours = (int)(time / 3600000);
    *minutes = (int)((time - (*hours * 3600000)) / 60000);
    *seconds = (((float)time) - (*hours * 3600000) - (*minutes * 60000)) / 1000.0f;
}

void integrate_modules() {
    ModuleList* curr = s_modules;
    if (curr != NULL) {
        print("Integrating modules...");
        uint64_t timer = mtime();
        while (!curr->module.integrated) {
            char fpbuffer[PATHLEN] = { 0 };
            char mbuffer[PATHLEN] = { 0 };
            snprintf(fpbuffer, PATHLEN, "build/modules/%s", curr->module.name);
            if (!dexists(fpbuffer)) {
                snprintf(fpbuffer, PATHLEN, "build/modules/%s/", curr->module.name);
                print("Downloading module \"%s\"...", curr->module.name);
                download_module(curr->module.name, curr->module.url, curr->module.path);
            }
            snprintf(fpbuffer, PATHLEN, "build/modules/%s/", curr->module.name);
            snprintf(mbuffer, PATHLEN, "build/modules/%s/.tinymodule", curr->module.name);
            if (fexists(mbuffer)) {
                configure(fpbuffer, mbuffer);
            } else {
                crash("No .tinymodule file detected for module \"%s\" - this is required to configure a module", curr->module.name);
            }
            curr->module.integrated = 1;
            if (curr->next) {
                curr = (ModuleList*)(curr->next);
            } else {
                curr = s_modules;
            }
        }
        int hours, minutes;
        float seconds;
        dissect_time_elapsed(timer, &hours, &minutes, &seconds);
        print("\033[32mFinished\033[0m integrating modules in %d:%d:%.3f", hours, minutes, seconds);
    }
}

void dissect_module(const char* str) {
    Module module = { 0 };
    char buffer[PATHLEN] = { 0 };
    size_t bi = 0;
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') {
            buffer[bi] = '\0';
            bi = 0;
            if (module.name[0] == '\0') {
                strcpy(module.name, buffer);
            } else if (module.url[0] == '\0') {
                strcpy(module.url, buffer);
            } else {
                crash("Too many arguments detected when trying to dissect module \"%s\" - \"%s\"", module.name, str);
            }
        } else {
            buffer[bi] = str[i];
            bi++;
        }
    }
    buffer[bi] = '\0';
    if (module.name[0] == '\0') {
        crash("Too few arguments detected when trying to dissect module \"%s\" - \"%s\"", str, str);
    } else if (module.url[0] == '\0') {
        warn("No path argument detected when trying to dissect module \"%s\" - defaulting to root directory", module.name);
        strcpy(module.url, buffer);
        strcpy(module.path, ".");
    } else {
        strcpy(module.path, buffer);
    }
    ModuleList* curr = s_modules;
    int found = 0;
    while (curr != NULL) {
        if (strcmp(curr->module.name, module.name) == 0) {
            found = 1;
            break;
        }
        curr = (ModuleList*)(curr->next);
    }
    if (!found) {
        modulelist_add(&s_modules, module);
    }
}

void download_module(const char* name, const char* url, const char* path) {
    const char* folder = (path && *path) ? path : ".";
    const char* tmpdir = "build/tmp";
    if (dexists(tmpdir)) {
        rmtree(tmpdir);
    }
    affirmdir(tmpdir);
    affirmdir("build/modules");
    char* q_url = generate_quote(url);
    char* q_tmp = generate_quote(tmpdir);
    char* q_folder = generate_quote(folder);
    char cmd[PATHLEN * 3] = { 0 };
    if (!q_url || !q_tmp || !q_folder) {
        crash("Invalid arguments to module detected - [NAME: \"%s\" | URL: \"%s\" | PATH: \"%s\"]", name, url, path);
    }
    if (strcmp(folder, ".") == 0) {
        snprintf(cmd, sizeof(cmd), "git clone --quiet --depth 1 --filter=blob:none %s %s", q_url, q_tmp);
        if (runcmd(cmd) != 0) {
            crash("Unable to download module \"%s\" via git (bad URL, auth, git, or network)", name);
        }
    } else {
        snprintf(cmd, sizeof(cmd), "git clone --quiet --no-checkout --depth 1 --filter=blob:none --sparse %s %s", q_url, q_tmp);
        if (runcmd(cmd) != 0) {
            crash("Unable to download module \"%s\" via git (bad URL, auth, git, or network)", name);
        }
        snprintf(cmd, sizeof(cmd), "cd %s && git sparse-checkout set %s", q_tmp, q_folder);
        if (runcmd(cmd) != 0) {
            crash("Unable to download module \"%s\" via git - sprase-checkout failed", name);
        }
        snprintf(cmd, sizeof(cmd), "cd %s && git checkout --quiet", q_tmp);
        if (runcmd(cmd) != 0) {
            crash("Unable to download module \"%s\" via git - materialization failed", name);
        }
    }
    char src_path[PATHLEN] = { 0 };
    snprintf(src_path, sizeof(src_path), "%s%c%s", tmpdir, PATH_SEP, folder);
    if (!dexists(src_path)) {
        crash("Module-specified folder \"%s\" not found in module \"%s\"", path, name);
    }
    char dest_path[PATHLEN] = { 0 };
    snprintf(dest_path, sizeof(dest_path), "build/modules/%s", name);
    if (!copytree(src_path, dest_path)) {
        crash("Failed to copy over module-critical data for module \"%s\"", name);
    }
    free(q_url);
    free(q_tmp);
    free(q_folder);
    rmtree(tmpdir);
}

int rmakedir(const char* dir) {
    char *p = strdup(dir);
    if (!p) return 0;
    for (char *s = p + 1; *s; s++) {
        if (*s == '/') {
            *s = '\0';
            if (!makedir(p) && errno != EEXIST) {
                free(p);
                return 0;
            }
            *s = '/';
        }
    }
    if (!makedir(p) && errno != EEXIST) {
        free(p);
        return 0;
    }
    free(p);
    return 1;
}

void modulelist_add(ModuleList** list, Module module) {
    ModuleList* new = calloc(1, sizeof(ModuleList));
    new->next = NULL;
    new->module = module;
    if (*list == NULL) {
        *list = new;
        return;
    }
    ModuleList* current = *list;
    new->next = current;
    *list = new;
}

void modulelist_delete(ModuleList* list) {
    if (list == NULL) return;
    ModuleList* n = (ModuleList*)list->next;
    free(list);
    modulelist_delete(n);
}

void pathlist_add(PathList** list, const char* path) {
    PathList* new = calloc(1, sizeof(PathList));
    new->next = NULL;
    strncpy(new->str, path, PATHLEN);
    if (*list == NULL) {
        *list = new;
        return;
    }
    PathList* current = *list;
    new->next = current;
    *list = new;
}

void pathlist_delete(PathList* list) {
    if (list == NULL) return;
    PathList* n = (PathList*)list->next;
    free(list);
    pathlist_delete(n);
}

size_t pathlist_len(PathList* list) {
    if (list == NULL) return 0;
    return 1 + pathlist_len(list->next);
}

void pathlist_construct(PathList* list, char* output) {
    size_t outcounter = 0;
    PathList* curr = list;
    while (curr != NULL) {
        strcpy(output + outcounter, curr->str);
        outcounter += strlen(curr->str);
        output[outcounter] = ' ';
        outcounter++;
        output[outcounter] = '\0';
        curr = (PathList*)curr->next;
    }
}

void clean_header_links() {
    while (s_header_links) {
        pathlist_delete(s_header_links->link->links);
        pathlist_delete(s_header_links->link->secondaries);
        free(s_header_links->link);
        HeaderLinkList* old = s_header_links;
        s_header_links = (HeaderLinkList*)s_header_links->next;
        free(old);
    }
}

void clean_source_links() {
    while (s_source_links) {
        pathlist_delete(s_source_links->link->links);
        pathlist_delete(s_source_links->link->secondaries);
        free(s_source_links->link);
        HeaderLinkList* old = s_source_links;
        s_source_links = (HeaderLinkList*)s_source_links->next;
        free(old);
    }
}

int header_link_exists(const char* header) {
    HeaderLinkList* curr = s_header_links;
    while (curr) {
        if (strcmp(curr->link->header, header) == 0) {
            return 1;
        }
        curr = (HeaderLinkList*)curr->next;
    }
    return 0;
}

void add_header_link(const char* header, const char* link) {
    HeaderLinkList* curr = s_header_links;
    while (curr) {
        if (strcmp(curr->link->header, header) == 0) {
            pathlist_add(&(curr->link->links), link);
            return;
        }
        curr = (HeaderLinkList*)curr->next;
    }
    HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
    new->link = calloc(1, sizeof(HeaderLink));
    strcpy(new->link->header, header);
    pathlist_add(&(new->link->links), link);
    new->next = s_header_links;
    s_header_links = new;
}

void add_source_link(const char* header, const char* link) {
    HeaderLinkList* curr = s_source_links;
    while (curr) {
        if (strcmp(curr->link->header, header) == 0) {
            pathlist_add(&(curr->link->links), link);
            return;
        }
        curr = (HeaderLinkList*)curr->next;
    }
    HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
    new->link = calloc(1, sizeof(HeaderLink));
    strcpy(new->link->header, header);
    pathlist_add(&(new->link->links), link);
    new->next = s_source_links;
    s_source_links = new;
}

void add_header_secondary(const char* header, const char* link) {
    HeaderLinkList* curr = s_header_links;
    while (curr) {
        if (strcmp(curr->link->header, header) == 0) {
            pathlist_add(&(curr->link->secondaries), link);
            return;
        }
        curr = (HeaderLinkList*)curr->next;
    }
    HeaderLinkList* new = calloc(1, sizeof(HeaderLinkList));
    new->link = calloc(1, sizeof(HeaderLink));
    strcpy(new->link->header, header);
    pathlist_add(&(new->link->secondaries), link);
    new->next = s_header_links;
    s_header_links = new;
}

int functionline(const char* line) {
    if (strstr(line, "#define")) return 0;
    int sfound = 0;
    int typefound = 0;
    int p1found = 0;
    int p2found = 0;
    int i = 0;
    while (1) {
        if (line[i] == '\0') return 0;
        if (!sfound && line[i] != ' ') sfound = 1;
        if (sfound && !typefound && line[i] == ' ') typefound = 1;
        else if (typefound && line[i] == '(' && !p1found) p1found = 1;
        else if (p1found && line[i] == '(') return 0;
        else if (p1found && line[i] == ')') p2found = 1;
        else if (p2found && line[i] == ';') return 1;
        else if (p2found) return 0;
        i++;
    }
    return 0;
}

int functionimplline(const char* line) {
    if (strstr(line, "#define")) return 0;
    int sfound = 0;
    int typefound = 0;
    int p1found = 0;
    int p2found = 0;
    int i = 0;
    while (1) {
        if (line[i] == '\0') return 0;
        if (!sfound && line[i] != ' ') sfound = 1;
        if (sfound && !typefound && line[i] == ' ') typefound = 1;
        else if (typefound && line[i] == '(' && !p1found) p1found = 1;
        else if (p1found && line[i] == '(') return 0;
        else if (p1found && line[i] == ')') p2found = 1;
        else if (p2found && line[i] == '{') return 1;
        else if (p2found && line[i] != ' ') return 0;
        i++;
    }
    return 0;
}

int vardeclared(const char* line) {
    if (strstr(line, "typedef ") == line) return 0;
    if (strstr(line, "{") && !strstr(line, "=")) return 0;
    int i = 0;
    while (1) {
        if (line[i] == ' ') return 1;
        if (line[i] == '(' ||
            line[i] == ')' ||
            line[i] == '#' ||
            line[i] == '{' ||
            line[i] == '}') return 0;
        i++;
    }
    return 0;
}

void easyc_audit(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    int slen = strlen(file);
    int header = (slen > 2 && (file[slen - 1] == 'h' && file[slen - 2] == '.'));
    int source = (slen > 2 && (file[slen - 1] == 'c' && file[slen - 2] == '.'));
    if (!header && !source) {
        return;
    }
    FILE* fp = fopen(file, "r");
    if (!fp) {
        crash("Failed to open file");
    }
    char line[PATHLEN * 2] = { 0 };
    int linecount = 0;
    while (fgets(line, sizeof(line), fp)) {
        linecount++;
        if (strstr(line,"malloc(") || strstr(line, "calloc(") || strstr(line, "free(")) {
            print("Detected unmonitored memory operation in \"%s\" on line %d", file, linecount);
            s_vulnerabilities++;
        }
    }
    fclose(fp);
}

void syntax_audit(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    int slen = strlen(file);
    int header = (slen > 2 && (file[slen - 1] == 'h' && file[slen - 2] == '.'));
    int source = (slen > 2 && (file[slen - 1] == 'c' && file[slen - 2] == '.'));
    if (!header && !source && !strstr(file, ".tinymodule")) {
        print("Detected abnormal file type in project: \"%s\"", file);
        s_vulnerabilities++;
        return;
    }
    int basename_ptr = 0;
    for (int i = slen; i > 0; i--) {
        if (file[i] == '/' || file[i] == '\\') {
            basename_ptr = i + 1;
            break;
        }
    }
    FILE* fp = fopen(file, "r");
    if (!fp) {
        crash("Failed to open file");
    }
    char line[PATHLEN * 2] = { 0 };
    int linecount = 0;
    int prev_empty = 0;
    int header_closed = 0;
    while (fgets(line, sizeof(line), fp)) {
        linecount++;
        int linelen = strlen(line);
        if (linelen >= PATHLEN) {
            print("Detected excessively long line in \"%s\" on line %d", file, linecount);
            s_vulnerabilities++;
        }
        int empty_line = 1;
        for (int i = 0; i < linelen; i++) {
            if (line[i] != '\n' && line[i] != ' ' && line[i] != '\t' && line[i] != '\r') {
                empty_line = 0;
                break;
            }
        }
        if (empty_line && prev_empty) {
            print("Detected excessive whitespace in \"%s\" on line %d", file, linecount);
            s_vulnerabilities++;
            prev_empty = 0;
        } else if (empty_line) {
            prev_empty = 1;
        } else {
            prev_empty = 0;
        }
        if (linecount == 1 && header) {
            char hbuf[PATHLEN] = { 0 };
            snprintf(hbuf, PATHLEN, "#ifndef %s", file + basename_ptr);
            int hlen = strlen(hbuf);
            for (int i = 8; i < hlen; i++) {
                if (hbuf[i] >= 'a' && hbuf[i] <= 'z') hbuf[i] = hbuf[i] + ('A' - 'a');
                else if (hbuf[i] == '.') hbuf[i] = '_';
            }
            char t = line[hlen];
            line[hlen] = '\0';
            if (strcmp(hbuf, line) != 0) {
                print("Detected missing or incorrect header guard in \"%s\" on line %d", file, linecount);
                s_vulnerabilities++;
            }
            line[hlen] = t;
        } else if (linecount == 2 && header) {
            char hbuf[PATHLEN] = { 0 };
            snprintf(hbuf, PATHLEN, "#define %s", file + basename_ptr);
            int hlen = strlen(hbuf);
            for (int i = 8; i < hlen; i++) {
                if (hbuf[i] >= 'a' && hbuf[i] <= 'z') hbuf[i] = hbuf[i] + ('A' - 'a');
                else if (hbuf[i] == '.') hbuf[i] = '_';
            }
            char t = line[hlen];
            line[hlen] = '\0';
            if (strcmp(hbuf, line) != 0) {
                print("Detected missing or incorrect header guard in \"%s\" on line %d", file, linecount);
                s_vulnerabilities++;
            }
            line[hlen] = t;
        }
        if (header) {
            char endbuf[7] = { 0 };
            if (linelen >= 6) memcpy(endbuf, line, 6);
            if (linelen >= 6 && strcmp("#endif", endbuf) == 0) header_closed = 1;
            else if (!empty_line) header_closed = 0;
        }
        if (header) {
            if (functionline(line)) {
                char implbuf[PATHLEN] = { 0 };
                char badimplbuf[PATHLEN] = { 0 };
                for (int i = 0; i < PATHLEN; i++) {
                    if (line[i] == ';') {
                        implbuf[i] = ' ';
                        implbuf[i + 1] = '{';
                        implbuf[i + 2] = '\0';
                        badimplbuf[i] = '{';
                        badimplbuf[i + 1] = '\0';
                        break;
                    } else {
                        implbuf[i] = line[i];
                        badimplbuf[i] = line[i];
                    }
                }
                int implfound = 0;
                int badimplfound = 0;
                char srcfilepath[PATHLEN] = { 0 };
                strcpy(srcfilepath, file);
                srcfilepath[slen - 1] = 'c';
                if (fexists(srcfilepath)) {
                    FILE* sfp = fopen(srcfilepath, "r");
                    if (!sfp) {
                        crash("Unable to open file");
                    }
                    char srcline[PATHLEN * 2] = { 0 };
                    int srclc = 0;
                    int srcbilc = 0;
                    while (fgets(srcline, sizeof(srcline), sfp)) {
                        srclc++;
                        if (strstr(srcline, implbuf)) implfound = 1;
                        if (strstr(srcline, badimplbuf)) {
                            badimplfound = 1;
                            srcbilc = srclc;
                        }
                    }
                    if (!implfound) {
                        if (badimplfound) {
                            print("The function implementation in \"%s\" on line %d is improperly formatted - please have a space in between the function and the curly brace.", srcfilepath, srcbilc);
                            s_vulnerabilities++;
                        } else {
                            print("Unable to find an implementation for the function on line %d of header \"%s\" in the corresponding source file", linecount, file);
                            s_vulnerabilities++;
                        }
                    }
                    fclose(sfp);
                } else {
                    print("Unable to find a corresponding source for the header \"%s\"", file);
                    s_vulnerabilities++;
                }
            }
        }
        if (header) {
            if (strstr(line, "#include")) {
                if (strstr(line, "easymemory.h")) s_easymemory_detected = 1;
                char ibuf[PATHLEN] = { 0 };
                int ind = 0;
                int toggle = 0;
                for (int i = 0; i < linelen; i++) {
                    if (line[i] == '\"' || line[i] == '<' || line[i] == '>') toggle = !toggle;
                    else if (toggle) {
                        ibuf[ind] = line[i];
                        ind++;
                        if (line[i] == '/' || line[i] == '\\') {
                            ind = 0;
                            memset(ibuf, 0, PATHLEN);
                        }
                    }
                }
                add_header_link(file + basename_ptr, ibuf);
            }
        }
        if (source) {
            if (line[0] != 0 && line[0] != '\n' && line[0] != '\r' && line[0] != ' ' && line[0] != '\t') {
                if (!strstr(line, "static") &&
                    !strstr(line, "extern") &&
                    !strstr(line, "#") &&
                    !(strstr(line, "//") == line)) {
                    if (!strstr(line, "(") && (strstr(line, "=") || vardeclared(line))) {
                        print("The global variable detected in \"%s\" on line %d is not translation protected - please make it static.", file, linecount);
                        s_vulnerabilities++;
                    }
                    if (functionimplline(line) && !strstr(line, "int main(")) {
                        char implbuf[PATHLEN] = { 0 };
                        strcpy(implbuf, line);
                        int implcorrect = 0;
                        for (int i = 0; i < PATHLEN; i++) {
                            if (line[i] == '{') {
                                if (i > 0 && line[i - 1] == ' ') {
                                    implbuf[i - 1] = ';';
                                    implbuf[i] = 0;
                                } else {
                                    implbuf[i] = ';';
                                    implbuf[i + 1] = 0;
                                }
                                implcorrect = 1;
                            }
                        }
                        if (implcorrect) {
                            char hfilepath[PATHLEN] = { 0 };
                            strcpy(hfilepath, file);
                            hfilepath[slen - 1] = 'h';
                            int needs_static = 1;
                            if (fexists(hfilepath)) {
                                FILE* hfp = fopen(hfilepath, "r");
                                if (!hfp) {
                                    crash("Unable to open file");
                                }
                                char hline[PATHLEN * 2] = { 0 };
                                while (fgets(hline, sizeof(hline), hfp)) {
                                    if (strstr(hline, implbuf)) {
                                        needs_static = 0;
                                        break;
                                    }
                                }
                                fclose(hfp);
                            }
                            if (needs_static) {
                                print("The global function detected in \"%s\" on line %d is not translation protected - please make it static.", file, linecount);
                                s_vulnerabilities++;
                            }
                        }
                    }
                }
            }
            if (strstr(line, "#include")) {
                if (strstr(line, "easymemory.h")) s_easymemory_detected = 1;
                char ibuf[PATHLEN] = { 0 };
                int ind = 0;
                int toggle = 0;
                for (int i = 0; i < linelen; i++) {
                    if (line[i] == '\"' || line[i] == '<' || line[i] == '>') toggle = !toggle;
                    else if (toggle) {
                        ibuf[ind] = line[i];
                        ind++;
                        if (line[i] == '/' || line[i] == '\\') {
                            ind = 0;
                            memset(ibuf, 0, PATHLEN);
                        }
                    }
                }
                add_source_link(file + basename_ptr, ibuf);
            }
        }
    }
    if (header && !header_closed) {
        print("Detected missing #endif to close header guard in \"%s\"", file);
        s_vulnerabilities++;
    }
    if (linecount == 0) {
        print("Detected empty file \"%s\"", file);
        s_vulnerabilities++;
    }
    fclose(fp);
}

void copyfile(const char* src, const char* dst) {
    FILE* srcFile = fopen(src, "rb");
    if (!srcFile) {
        crash("Failed to open source file");
    }
    FILE *dstFile = fopen(dst, "wb");
    if (!dstFile) {
        fclose(srcFile);
        crash("Failed to open destination file \"%s\"", dst);
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        if (fwrite(buffer, 1, bytes, dstFile) != bytes) {
            fclose(srcFile);
            fclose(dstFile);
            crash("Critical write error during copying");
        }
    }
    fclose(srcFile);
    fclose(dstFile);
}

int filecmp(const char* path1, const char* path2) {
    FILE* f1 = fopen(path1, "rb");
    FILE* f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        fclose(f1);
        fclose(f2);
        crash("failure during opening files");
    }
    struct stat stat1, stat2;
    if (stat(path1, &stat1) != 0 || stat(path2, &stat2) != 0) {
        fclose(f1);
        fclose(f2);
        crash("failure during stating files");
    }
    if (stat1.st_size != stat2.st_size) {
        fclose(f1);
        fclose(f2);
        return 0;
    }
    char buf1[4096], buf2[4096];
    size_t bytes1, bytes2;
    int equal = 1;
    while ((bytes1 = fread(buf1, 1, 4096, f1)) > 0 &&
           (bytes2 = fread(buf2, 1, 4096, f2)) > 0) {
        if (bytes1 != bytes2 || memcmp(buf1, buf2, bytes1) != 0) {
            equal = 0;
            break;
        }
    }
    fclose(f1);
    fclose(f2);
    return equal;
}

void affirmdir(const char* dir) {
    if (!dexists(dir) && !rmakedir(dir)) {
        crash("Unable to affirm directory %s", dir);
    }
}

void affirm_to_cache(const char* dir) {
    if (strstr(dir, "build/cache") == &(dir[0]) || strstr(dir, "./build/cache") == &(dir[0])) return;
    char buffer[PATHLEN] = { 0 };
    snprintf(buffer, PATHLEN, "build/cache/%s", dir);
    affirmdir(buffer);
}

void add_to_sources(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    size_t slen = strlen(file);
    if (slen > 2 && file[slen - 1] == 'c' && file[slen - 2] == '.')
        pathlist_add(&s_sources, file);
}

void verify_header(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    size_t slen = strlen(file);
    if (slen > 2 && (file[slen - 1] != 'h' || file[slen - 2] != '.')) return;
    int basename_ptr = 0;
    for (int i = slen; i > 0; i--) {
        if (file[i] == '/' || file[i] == '\\') {
            basename_ptr = i + 1;
            break;
        }
    }
    char destination[PATHLEN] = { 0 };
    snprintf(destination, PATHLEN, "build/cache/%s", file);
    if (!fexists(destination)) {
        pathlist_add(&s_changed_headers, file + basename_ptr);
        copyfile(file, destination);
    } else {
        if (!filecmp(file, destination)) {
            pathlist_add(&s_changed_headers, file + basename_ptr);
            copyfile(file, destination);
        }
    }
}

void accumulate_header(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    size_t slen = strlen(file);
    if (slen > 2 && (file[slen - 1] != 'h' || file[slen - 2] != '.')) return;
    int basename_ptr = 0;
    for (int i = slen; i > 0; i--) {
        if (file[i] == '/' || file[i] == '\\') {
            basename_ptr = i + 1;
            break;
        }
    }
    PathList* curr = s_changed_headers;
    while (curr != NULL) {
        if (strcmp(file + basename_ptr, curr->str) == 0) return;
        curr = (PathList*)curr->next;
    }
    FILE* fp = fopen(file, "r");
    if (!fp) {
        crash("Unable to open file \"%s\"", file);
    }
    char line[PATHLEN * 2] = { 0 };
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "#include") != NULL) {
            curr = s_changed_headers;
            while (curr != NULL) {
                if (strstr(line, curr->str) != NULL) {
                    pathlist_add(&s_changed_headers, file + basename_ptr);
                    fclose(fp);
                    return;
                }
                curr = (PathList*)curr->next;
            }
        }
    }
    fclose(fp);
}

void async_compile_progress_update(int index, int action, const char* name) {
    if (action == 0) {
        print("- [%s] \033[33m(compiling...)\033[0m", name);
    } else {
        print("\033[%dA\033[2K- [%s] \033[32mOK\033[0m", s_sourcei - index, name);
        printf("\033[%dB", (s_sourcei - index) - 1);
    }
}

void async_compile(void* params) {
    ThreadParameters* tp = (ThreadParameters*)params;
    int result = system(tp->command);
    if (result == 0) {
        copyfile(tp->file, tp->destination);
        TINY_LOCK_MUTEX(s_mutex);
        async_compile_progress_update(tp->sourcei, 1, tp->file + tp->basename_ptr);
        TINY_RELEASE_MUTEX(s_mutex);
    } else {
        TINY_LOCK_MUTEX(s_mutex);
        print("Building source \"%s\" \033[31mfailed\033[0m", tp->file + tp->basename_ptr);
        exit(1);
    }
    TINY_LOCK_MUTEX(s_mutex);
    s_active_threads[tp->index] = 2;
    TINY_RELEASE_MUTEX(s_mutex);
    free(tp->command);
    free(tp);
}

void compile_source(const char* file) {
    if (strstr(file, "build/cache") == &(file[0]) || strstr(file, "./build/cache") == &(file[0])) return;
    size_t slen = strlen(file);
    if (slen > 2 && (file[slen - 1] != 'c' || file[slen - 2] != '.')) return;
    int basename_ptr = 0;
    for (int i = slen; i > 0; i--) {
        if (file[i] == '/' || file[i] == '\\') {
            basename_ptr = i + 1;
            break;
        }
    }
    char destination[PATHLEN] = { 0 };
    snprintf(destination, PATHLEN, "build/cache/%s", file);
    if (strcmp(file + basename_ptr, s_main_file_name) == 0) {
        if (strcmp(s_main_file_path, file) != 0) {
            if (s_found_main) {
                crash("another main file detected: %s", file);
            }
            s_found_main = 1;
            strcpy(s_main_file_path, file);
        }
        if (!fexists(destination) || !filecmp(destination, file)) {
            copyfile(file, destination);
            s_main_up_to_date = 0;
        }
    } else {
        if (fexists(destination)) {
            FILE* fp = fopen(file, "r");
            if (!fp) {
                crash("Unable to open file \"%s\"", file);
            }
            char line[PATHLEN * 2] = { 0 };
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "#include") != NULL) {
                    PathList* curr = s_changed_headers;
                    int breakagain = 0;
                    while (curr != NULL) {
                        if (strstr(line, curr->str) != NULL) {
                            remove(destination);
                            breakagain = 1;
                            break;
                        }
                        curr = (PathList*)curr->next;
                    }
                    if (breakagain) break;
                }
            }
            fclose(fp);
        }
        char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
        char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
        char* rawbuf = calloc(pathlist_len(s_raws), PATHLEN);
        char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
        char* defbuf = calloc(pathlist_len(s_defines), PATHLEN);
        pathlist_construct(s_includes, incbuf);
        pathlist_construct(s_links, linkbuf);
        pathlist_construct(s_raws, rawbuf);
        pathlist_construct(s_libs, libbuf);
        pathlist_construct(s_defines, defbuf);
        char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + PATHLEN, sizeof(char));
        sprintf(
            commandbuf,
            "gcc %s-Wall -Wextra -Wno-unused-parameter -c %s %s%s%s-o %s.o %s %s",
            defbuf,
            file,
            incbuf,
            libbuf,
            linkbuf,
            destination,
            rawbuf,
            s_flags & PROD ? "-O3 -flto -DPROD_BUILD" : "");
        if (!fexists(destination) || !filecmp(file, destination)) {
            s_sources_up_to_date = 0;
            if (s_flags & FAST) {
                int ind = 0;
                while (1) {
                    TINY_LOCK_MUTEX(s_mutex);
                    if (s_active_threads[ind] != 1) {
                        if (s_active_threads[ind] == 2) {
                            TINY_RELEASE_MUTEX(s_mutex);
                            TINY_WAIT_THREAD(s_threads[ind]);
                            TINY_LOCK_MUTEX(s_mutex);
                        }
                        s_active_threads[ind] = 1;
                        ThreadParameters* tp = calloc(1, sizeof(ThreadParameters));
                        tp->command = commandbuf;
                        strcpy(tp->destination, destination);
                        strcpy(tp->file, file);
                        tp->index = ind;
                        tp->basename_ptr = basename_ptr;
                        tp->sourcei = s_sourcei;
                        s_sourcei++;
                        async_compile_progress_update(s_sourcei - 1, 0, file + basename_ptr);
                        TINY_CREATE_THREAD(s_threads[ind], async_compile, tp);
                        TINY_RELEASE_MUTEX(s_mutex);
                        break;
                    }
                    TINY_RELEASE_MUTEX(s_mutex);
                    ind++;
                    if (ind >= threadcount()) ind = 0;
                }
            } else {
                print("- [%s] \033[33m(compiling...)\033[0m", file + basename_ptr);
                int result = system(commandbuf);
                if (result == 0) {
                    print("\033[1A\033[0K- [%s] \033[32mOK\033[0m", file + basename_ptr);
                    copyfile(file, destination);
                } else {
                    print("Building source \"%s\" \033[31mfailed\033[0m", file + basename_ptr);
                    exit(1);
                }
                free(commandbuf);
            }
        }
        char finalbuf[PATHLEN + 2] = { 0 };
        snprintf(finalbuf, PATHLEN + 2, "%s.o", destination);
        pathlist_add(&s_objects, finalbuf);
        free(incbuf);
        free(linkbuf);
        free(rawbuf);
        free(libbuf);
        free(defbuf);
    }
}

void parseflag(char* flag, int blacklistable) {
    if ((s_flags & RUN) && blacklistable) {
        s_copy_argsv[s_copy_argsc] = calloc(strlen(flag) + 1, sizeof(char));
        strcpy(s_copy_argsv[s_copy_argsc], flag);
        s_copy_argsc++;
        return;
    }
    char buffer[PATHLEN] = { 0 };
    int whitelist = 1;
    if (blacklistable) {
        for (size_t i = 0; i < strlen(flag); i++) {
            if (flag[i] == '=') {
                if (strcmp(flag + i + 1, "true") == 0 || strcmp(flag + i + 1, "TRUE") == 0) {
                    whitelist = 1;
                } else if (strcmp(flag + i + 1, "false") == 0 || strcmp(flag + i + 1, "FALSE") == 0) {
                    whitelist = 0;
                } else {
                    crash("Unknown equal argument in flag \"%s\" - \"%s\"", flag, flag + i + 1);
                }
                break;
            }
            buffer[i] = flag[i];
        }
    } else {
        strcpy(buffer, flag);
    }
    if (strcmp("-v", buffer) == 0 || strcmp("-version", buffer) == 0) {
        if (whitelist) {
            print("TINY BUILDER \033[32mv%d.%d.%d\033[0m authored by Jason Heflinger (https://github.com/JHeflinger)", VERSION, MAJOR_RELEASE, MINOR_RELEASE);
            exit(0);
        }
    } else if (strcmp("-p", buffer) == 0 || strcmp("-prod", buffer) == 0) {
        if (whitelist && !(s_unflags & PROD)) {
            print("Optimizing for production build...");
            s_flags |= PROD;
        } else {
            s_unflags |= PROD;
        }
    } else if (strcmp("-a", buffer) == 0 || strcmp("-audit", buffer) == 0) {
        if (whitelist && !(s_unflags & AUDIT)) {
            s_flags |= AUDIT;
        } else {
            s_unflags |= AUDIT;
        }
    } else if (strcmp("-f", buffer) == 0 || strcmp("-fast", buffer) == 0) {
        if (whitelist && !(s_unflags & FAST)) {
            print("Enabling multi-threaded building over %d cores...", threadcount());
            s_threads = calloc(threadcount(), sizeof(TINY_THREAD));
            s_active_threads = calloc(threadcount(), sizeof(int));
            TINY_CREATE_MUTEX(s_mutex);
            s_flags |= FAST;
        } else {
            s_unflags |= FAST;
        }
    } else if (strcmp("-d", buffer) == 0 || strcmp("-debug", buffer) == 0) {
        if (whitelist && !(s_unflags & DEBUG)) {
            s_flags |= DEBUG;
        } else {
            s_unflags |= DEBUG;
        }
    } else if (strcmp("-rv", buffer) == 0 || strcmp("-recompile_vendors", buffer) == 0) {
        if (whitelist && !(s_unflags & RECOMPILE_VENDORS)) {
            s_flags |= RECOMPILE_VENDORS;
        } else {
            s_unflags |= RECOMPILE_VENDORS;
        }
    } else if (strcmp("-r", buffer) == 0 || strcmp("-run", buffer) == 0) {
        if (whitelist && !(s_unflags & RUN)) {
            s_copy_argsv = calloc(s_max_argsc, sizeof(char*));
            s_copy_argsc = 1;
            s_copy_argsv[0] = calloc(strlen("bin.exe") + 1, sizeof(char));
            strcpy(s_copy_argsv[0], "bin.exe");
            s_flags |= RUN;
        } else {
            s_unflags |= RUN;
        }
    } else if (strcmp("-c", buffer) == 0 || strcmp("-clean", buffer) == 0) {
        if (whitelist && !(s_unflags & CLEAN)) {
            if (dexists("build/cache")) {
                print("Cleaning cache...");
                rmtree("build/cache");
                print("Cache has been \033[32msuccessfully cleaned\033[0m!");
            } else {
                print("Cache is \033[32malready clean\033[0m!");
            }
            exit(0);
        } else {
            s_unflags |= CLEAN;
        }
    } else {
        crash("Unknown flag \"%s\" detected", buffer);
    }
}

void configure(const char* prepath, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        crash("Unable to open configuration file \"%s\"", path);
    }
    char line[PATHLEN * 2] = { 0 };
    char precursor[PATHLEN] = { 0 };
    char workbuffer[PATHLEN] = { 0 };
    int linecount = 0;
    while (fgets(line, sizeof(line), file)) {
        for (int i = strlen(line) - 1; i >= 0; i--) {
            if (line[i] == '\n' || line[i] == '\r') {
                line[i] = '\0';
            } else break;
        }
        size_t postcursor = 0;
        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == ' ') {
                precursor[i] = '\0';
                postcursor = i + 1;
                break;
            } else {
                precursor[i] = line[i];
            }
        }
        if (strlen(line) == 0 || line[0] == '#') continue;
        #ifdef __WIN32
            if (strcmp(precursor, "LINUX") == 0) {
                continue;
            }
            if (strcmp(precursor, "MACOS") == 0) {
                continue;
            }
            if (strcmp(precursor, "WINDOWS") == 0) {
                for (size_t i = postcursor; i < strlen(line); i++) {
                    if (line[i] == ' ') {
                        precursor[i - postcursor] = '\0';
                        postcursor = i + 1;
                        break;
                    } else {
                        precursor[i - postcursor] = line[i];
                    }
                }
            }
        #elif __linux__
            if (strcmp(precursor, "WINDOWS") == 0) {
                continue;
            }
            if (strcmp(precursor, "MACOS") == 0) {
                continue;
            }
            if (strcmp(precursor, "LINUX") == 0) {
                for (size_t i = postcursor; i < strlen(line); i++) {
                    if (line[i] == ' ') {
                        precursor[i - postcursor] = '\0';
                        postcursor = i + 1;
                        break;
                    } else {
                        precursor[i - postcursor] = line[i];
                    }
                }
            }
        #elif __APPLE__
            if (strcmp(precursor, "WINDOWS") == 0) {
                continue;
            }
            if (strcmp(precursor, "LINUX") == 0) {
                continue;
            }
            if (strcmp(precursor, "MACOS") == 0) {
                for (size_t i = postcursor; i < strlen(line); i++) {
                    if (line[i] == ' ') {
                        precursor[i - postcursor] = '\0';
                        postcursor = i + 1;
                        break;
                    } else {
                        precursor[i - postcursor] = line[i];
                    }
                }
            }
        #endif
        if (strcmp(precursor, "PROJECT") == 0) {
            snprintf(workbuffer, PATHLEN, "%s%s", prepath, line + postcursor);
            pathlist_add(&s_projects, workbuffer);
        } else if (strcmp(precursor, "MAIN") == 0) {
            snprintf(workbuffer, PATHLEN, "%s%s", prepath, line + postcursor);
            if (fexists(workbuffer)) {
                strcpy(s_main_file_name, workbuffer);
            } else {
                strcpy(s_main_file_name, line + postcursor);
            }
        } else if (strcmp(precursor, "INCLUDE") == 0) {
            snprintf(workbuffer, PATHLEN, "-I\"%s%s\"", prepath, line + postcursor);
            pathlist_add(&s_includes, workbuffer);
        } else if (strcmp(precursor, "LINK") == 0) {
            snprintf(workbuffer, PATHLEN, "-l%s", line + postcursor);
            pathlist_add(&s_links, workbuffer);
        } else if (strcmp(precursor, "LIB") == 0) {
            snprintf(workbuffer, PATHLEN, "-L\"%s%s\"", prepath, line + postcursor);
            pathlist_add(&s_libs, workbuffer);
        } else if (strcmp(precursor, "SOURCE") == 0) {
            snprintf(workbuffer, PATHLEN, "%s%s", prepath, line + postcursor);
            if (dexists(workbuffer)) {
                walkfiles(workbuffer, add_to_sources);
            } else {
                pathlist_add(&s_sources, workbuffer);
            }
        } else if (strcmp(precursor, "FLAG") == 0) {
            char b[PATHLEN] = { 0 };
            sprintf(b, "-%s", line + postcursor);
            parseflag(b, 0);
        } else if (strcmp(precursor, "FRAMEWORK") == 0) {
            snprintf(workbuffer, PATHLEN, "-framework %s", line + postcursor);
            pathlist_add(&s_links, workbuffer);
        } else if (strcmp(precursor, "DEFINE") == 0) {
            snprintf(workbuffer, PATHLEN, "-D\"%s\"", line + postcursor);
            pathlist_add(&s_defines, workbuffer);
        } else if (strcmp(precursor, "RAW") == 0) {
            pathlist_add(&s_raws, line + postcursor);
        } else if (strcmp(precursor, "MODULE") == 0) {
            dissect_module(line + postcursor);
        } else if (strcmp(precursor, "PORT") == 0) {
            snprintf(workbuffer, PATHLEN, "%s%s", prepath, line + postcursor);
            port_folder(workbuffer);
        } else {
            warn("Unknown precursor \"%s\" detected on line %d of \".tinyconf\" - skipping", precursor, linecount);
        }
        memset(line, 0, PATHLEN * 2);
        memset(precursor, 0, PATHLEN);
        memset(workbuffer, 0, PATHLEN);
    }
    fclose(file);
}

void affirm_projects() {
    // default project directory
    if (s_projects == NULL) {
        pathlist_add(&s_projects, "src");
    }

    PathList* curr = s_projects;
    while (curr != NULL) {
        // ensure project directory exists
        if (!dexists(curr->str)) {
            crash("Project directory \"%s/\" does not exist - if this is not your desired location, please specify a different one in \".tinyconf\"", curr->str);
        }

        // set up build directories
        char tbuf[PATHLEN + 12] = { 0 };
        snprintf(tbuf, PATHLEN + 12, "build/cache/%s", curr->str);
        affirmdir(tbuf);

        // set up cache folders
        walkdir(curr->str, affirm_to_cache);

        // set up include directories
        snprintf(tbuf, PATHLEN + 12, "-I\"%s\"", curr->str);
        pathlist_add(&s_includes, tbuf);

        curr = (PathList*)curr->next;
    }
}

void initialize(int argc, char* argv[]) {
    // initialize timer
    s_start_time = mtime();

    // parse flags
    for (int i = 1; i < argc; i++) {
        parseflag(argv[i], 1);
    }

    // set up cwd
    if (cwd(s_cwd) == NULL) {
        crash("Unable to find the current working directory");
    }

    // fallback main file name
    strcpy(s_main_file_name, "main.c");

    // import configuration from .tinyconf
    if (fexists(".tinyconf")) {
        configure("", ".tinyconf");
    }

    // set up build directories
    affirmdir("build");
    affirmdir("build/cache");
    affirmdir("build/vendor");
}

void compile_vendors() {
    if (s_sources == NULL) return;
    if (!fexists("build/vendor/vendor.o") || (s_flags & RECOMPILE_VENDORS)) {
        print("Compiling vendors...");
        FILE* file = fopen("build/vendor/tiny_merged_vendors.c", "w");
        if (!file) {
            crash("Unable to consolidate vendor sources");
        }
        PathList* pl = s_sources;
        while (pl != NULL) {
            char buffer[PATHLEN + 18] = { 0 };
            snprintf(buffer, PATHLEN + 18, "#include \"../../%s\"\n", pl->str);
            fprintf(file, buffer);
            pl = (PathList*)pl->next;
        }
        fclose(file);
        char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
        char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
        char* rawbuf = calloc(pathlist_len(s_raws), PATHLEN);
        char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
        char* defbuf = calloc(pathlist_len(s_defines), PATHLEN);
        pathlist_construct(s_includes, incbuf);
        pathlist_construct(s_links, linkbuf);
        pathlist_construct(s_raws, rawbuf);
        pathlist_construct(s_libs, libbuf);
        pathlist_construct(s_defines, defbuf);
        char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + PATHLEN, sizeof(char));
        sprintf(
            commandbuf,
            "gcc %s-Wall -Wextra -Wno-unused-parameter -c build/vendor/tiny_merged_vendors.c %s%s%s-o build/vendor/vendor.o %s%s",
            defbuf,
            incbuf,
            libbuf,
            linkbuf,
            rawbuf,
            s_flags & PROD ? "-O3 -DPROD_BUILD" : "");
        uint64_t timer = mtime();
        int result = system(commandbuf);
        if (result == 0) {
            int hours, minutes;
            float seconds;
            dissect_time_elapsed(timer, &hours, &minutes, &seconds);
            print("\033[32mFinished\033[0m compiling vendors in %d:%d:%.3f", hours, minutes, seconds);
        } else {
            print("Building vendors \033[31mfailed\033[0m");
        }
        free(commandbuf);
        free(incbuf);
        free(linkbuf);
        free(rawbuf);
        free(libbuf);
        free(defbuf);
    }
    pathlist_add(&s_objects, "build/vendor/vendor.o");
}

void calculate_dependencies() {
    print("Calculating dependency tree...");
    uint64_t timer = mtime();
    PathList* curr = s_projects;
    while (curr != NULL) {
        walkfiles(curr->str, verify_header);
        curr = (PathList*)curr->next;
    }
    if (s_changed_headers == NULL) {
        print("\033[1A\033[0KHeaders are currently \033[32mup to date\033[0m");
        return;
    }
    while (1) {
        PathList* current = s_changed_headers;
        PathList* currproj = s_projects;
        while (currproj != NULL) {
            walkfiles(currproj->str, accumulate_header);
            currproj = (PathList*)currproj->next;
        }
        if (current == s_changed_headers) break;
    }
    int hours, minutes;
    float seconds;
    dissect_time_elapsed(timer, &hours, &minutes, &seconds);
    print("\033[32mFinished\033[0m calculating depdencies in %d:%d:%.3f", hours, minutes, seconds);
}

void compile_objects() {
    print("Compiling sources...");
    uint64_t timer = mtime();
    if (fexists(s_main_file_name)) {
        char destination[PATHLEN] = { 0 };
        int basename_ptr = 0;
        for (int i = strlen(s_main_file_name); i > 0; i--) {
            if (s_main_file_name[i] == '/' || s_main_file_name[i] == '\\') {
                basename_ptr = i + 1;
                break;
            }
        }
        snprintf(destination, PATHLEN, "build/cache/%s", s_main_file_name);
        strcpy(s_main_file_path, s_main_file_name);
        s_found_main = 1;
        if (!fexists(destination) || !filecmp(destination, s_main_file_path)) {
            copyfile(s_main_file_path, destination);
            s_main_up_to_date = 0;
        }
        snprintf(destination, PATHLEN, "%s", s_main_file_name + basename_ptr);
        strcpy(s_main_file_name, destination);
    }
    PathList* curr = s_projects;
    while (curr != NULL) {
        walkfiles(curr->str, compile_source);
        curr = (PathList*)curr->next;
    }
    if (s_flags & FAST) {
        while (1) {
            int all_done = 1;
            for (int i = 0; i < threadcount(); i++) {
                TINY_LOCK_MUTEX(s_mutex);
                if (s_active_threads[i] == 1) all_done = 0;
                TINY_RELEASE_MUTEX(s_mutex);
            }
            if (all_done) break;
        }
    }
    if (s_sources_up_to_date) {
        print("\033[1A\033[0KSources are currently \033[32mup to date\033[0m");
    } else {
        int hours, minutes;
        float seconds;
        dissect_time_elapsed(timer, &hours, &minutes, &seconds);
        print("\033[32mFinished\033[0m compiling sources in %d:%d:%.3f", hours, minutes, seconds);
    }
    if (!s_found_main) {
        crash("unable to compile without a detected \"%s\" file", s_main_file_name);
    }
}

void compile_executable() {
    print("Building executable...");
    uint64_t timer = mtime();
    char* incbuf = calloc(pathlist_len(s_includes), PATHLEN);
    char* linkbuf = calloc(pathlist_len(s_links), PATHLEN);
    char* rawbuf = calloc(pathlist_len(s_raws), PATHLEN);
    char* libbuf = calloc(pathlist_len(s_libs), PATHLEN);
    char* objbuf = calloc(pathlist_len(s_objects), PATHLEN);
    char* defbuf = calloc(pathlist_len(s_defines), PATHLEN);
    pathlist_construct(s_includes, incbuf);
    pathlist_construct(s_links, linkbuf);
    pathlist_construct(s_libs, libbuf);
    pathlist_construct(s_raws, rawbuf);
    pathlist_construct(s_objects, objbuf);
    pathlist_construct(s_defines, defbuf);
    char* commandbuf = calloc(strlen(incbuf) + strlen(linkbuf) + strlen(libbuf) + strlen(objbuf) + PATHLEN, sizeof(char));
    sprintf(
        commandbuf,
        "gcc %s-Wall -Wextra -Wno-unused-parameter %s %s%s%s%s-o build/bin.exe %s%s",
        defbuf,
        s_main_file_path,
        objbuf,
        incbuf,
        libbuf,
        linkbuf,
        rawbuf,
        s_flags & PROD ? "-O3 -DPROD_BUILD" : "");
    if (s_flags & DEBUG) printf("\n\n==========DEBUG COMMAND BUFFER==========\n\n%s\n\n========END DEBUG COMMAND BUFFER========\n\n", commandbuf);
    int result = system(commandbuf);
    if (result == 0) {
        int hours, minutes;
        float seconds;
        dissect_time_elapsed(timer, &hours, &minutes, &seconds);
        print("\033[32mFinished\033[0m compiling executable in %d:%d:%.3f", hours, minutes, seconds);
    } else {
        print("Building executable \033[31mfailed\033[0m");
        exit(1);
    }
    free(commandbuf);
    free(objbuf);
    free(incbuf);
    free(linkbuf);
    free(libbuf);
    free(rawbuf);
}

void get_in_depth_headers(const char* dive_header, HeaderLinkList* update_header) {
    HeaderLinkList* header = s_header_links;
    while (header) {
        if (strcmp(header->link->header, dive_header) == 0) {
            PathList* secondary = header->link->links;
            while (secondary) {
                if (strcmp(secondary->str, update_header->link->header) == 0) {
                    print("\033[31mCritical error\033[0m: recursive include detected from \"%s\" in \"%s\"! Aborting build...", update_header->link->header, dive_header);
                    exit(1);
                }
                add_header_secondary(update_header->link->header, secondary->str);
                if (header_link_exists(secondary->str)) {
                    get_in_depth_headers(secondary->str, update_header);
                    return;
                }
                secondary = (PathList*)secondary->next;
            }
            return;
        }
        header = (HeaderLinkList*)header->next;
    }
}

void audit() {
    print("Auditing project...")
    uint64_t timer = mtime();
    PathList* currproj = s_projects;
    while (currproj != NULL) {
        walkfiles(currproj->str, syntax_audit);
        currproj = (PathList*)currproj->next;
    }
    HeaderLinkList* header = s_header_links;
    while (header) {
        PathList* primary = header->link->links;
        while (primary) {
            if (header_link_exists(primary->str)) {
                get_in_depth_headers(primary->str, header);
            }
            primary = (PathList*)primary->next;
        }
        header = (HeaderLinkList*)header->next;
    }
    header = s_header_links;
    while (header) {
        PathList* primary = header->link->links;
        while (primary) {
            PathList* secondary = header->link->secondaries;
            while (secondary) {
                if (strcmp(primary->str, secondary->str) == 0) {
                    if (header_link_exists(primary->str)) {
                        print("Useless include detected from \"%s\" - \"%s\" is not needed", header->link->header, primary->str);
                    } else {
                        print("Useless include detected from \"%s\" - <%s> is not needed", header->link->header, primary->str);
                    }
                    s_vulnerabilities++;
                    break;
                }
                secondary = (PathList*)secondary->next;
            }
            primary = (PathList*)primary->next;
        }
        header = (HeaderLinkList*)header->next;
    }
    HeaderLinkList* source = s_source_links;
    while (source) {
        char buff[PATHLEN] = { 0 };
        strcpy(buff, source->link->header);
        buff[strlen(source->link->header) - 1] = 'h';
        PathList* inc = source->link->links;
        while (inc) {
            HeaderLinkList* curr = s_header_links;
            while (curr) {
                if (strcmp(curr->link->header, buff) == 0) {
                    PathList* secondary = curr->link->secondaries;
                    while (secondary) {
                        if (strcmp(inc->str, secondary->str) == 0) {
                            if (header_link_exists(inc->str)) {
                                print("Useless include detected from \"%s\" - \"%s\" is not needed", source->link->header, inc->str);
                            } else {
                                print("Useless include detected from \"%s\" - <%s> is not needed", source->link->header, inc->str);
                            }
                            s_vulnerabilities++;
                            break;
                        }
                        secondary = (PathList*)secondary->next;
                    }
                }
                curr = (HeaderLinkList*)curr->next;
            }
            inc = (PathList*)inc->next;
        }
        source = (HeaderLinkList*)source->next;
    }
    if (s_easymemory_detected) {
        PathList* curr = s_projects;
        while (curr != NULL) {
            walkfiles(curr->str, easyc_audit);
            curr = (PathList*)curr->next;
        }
    }

    int hours, minutes;
    float seconds;
    dissect_time_elapsed(timer, &hours, &minutes, &seconds);
    print("Finished audit in %d:%d:%.3f - detected %s%d\033[0m vulnerabilities", 
       hours, minutes, seconds, 
       s_vulnerabilities == 0 ? "\033[32m" : s_vulnerabilities <= 10 ? "\033[33m" : "\033[31m", 
       s_vulnerabilities);
    clean_header_links();
    clean_source_links();
}

void port_folder(const char* path) {
    if (!dexists(path)) {
        crash("Cannot port folder \"%s\" because it does not exist", path);
    }
    affirmdir("build/env");
    int basename_ptr = 0;
    for (int i = strlen(path); i > 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            basename_ptr = i + 1;
            break;
        }
    }
    char buffer[PATHLEN] = { 0 };
    char buffer2[PATHLEN] = { 0 };
    snprintf(buffer, PATHLEN, "build/env/%s", path + basename_ptr);
    snprintf(buffer2, PATHLEN, "..%c..%c%s", PATH_SEP, PATH_SEP, path);
    if (!dexists(buffer) && !make_symlink(buffer2, buffer)) {
        crash("Unable to create symlink of path \"%s\"", path);
    }
}

int main(int argc, char* argv[]) {
    s_max_argsc = argc;
    initialize(argc, argv);
    integrate_modules();
    affirm_projects();
    if (s_flags & AUDIT) audit();
    compile_vendors();
    calculate_dependencies();
    compile_objects();
    if (!s_sources_up_to_date || !fexists("build/bin.exe") || !s_main_up_to_date || s_flags & RECOMPILE_VENDORS) {
        compile_executable();
    } else {
        print("Current build is \033[32mup to date\033[0m, no need to build executable");
    }
    pathlist_delete(s_sources);
    pathlist_delete(s_includes);
    pathlist_delete(s_links);
    pathlist_delete(s_defines);
    pathlist_delete(s_libs);
    pathlist_delete(s_raws);
    pathlist_delete(s_projects);
    int hours, minutes;
    float seconds;
    dissect_time_elapsed(s_start_time, &hours, &minutes, &seconds);
    print("\033[32mFinished\033[0m total build in %d:%d:%.3f", hours, minutes, seconds);
    if (s_flags & FAST) {
        free(s_threads);
        free(s_active_threads);
    }
    if (s_flags & RUN) {
        run_build();
    }
    for (size_t i = 0; i < s_copy_argsc; i++) {
        free(s_copy_argsv[i]);
    }
    if (s_copy_argsv) free(s_copy_argsv);
    return 0;
}
