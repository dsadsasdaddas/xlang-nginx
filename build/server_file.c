#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

int __xlang_argc_g = 0;
char** __xlang_argv_g = 0;
char* __xlang_str_concat(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    char* out = (char*)malloc(la + lb + 1);
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = 0;
    return out;
}
char* __xlang_int_to_str(int32_t n) {
    char* buf = (char*)malloc(16);
    snprintf(buf, 16, "%d", n);
    return buf;
}
char* __xlang_read_stdin() {
    size_t cap = 65536, len = 0;
    char* buf = (char*)malloc(cap);
    size_t r;
    while ((r = fread(buf + len, 1, cap - len, stdin)) > 0) {
        len += r;
        if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
    }
    buf[len] = 0;
    return buf;
}
char* __xlang_read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) { char* e = (char*)malloc(1); e[0] = 0; return e; }
    size_t cap = 65536, len = 0;
    char* buf = (char*)malloc(cap);
    size_t r;
    while ((r = fread(buf + len, 1, cap - len, f)) > 0) {
        len += r;
        if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
    }
    buf[len] = 0; fclose(f);
    return buf;
}
void __xlang_write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(content, 1, strlen(content), f); fclose(f);
}
int32_t __xlang_str_find(const char* s, const char* sub) {
    const char* p = strstr(s, sub);
    return p ? (int32_t)(p - s) : -1;
}
char* __xlang_str_slice(const char* s, int32_t start, int32_t end) {
    if (start < 0) start = 0;
    if (end < start) end = start;
    int32_t len = end - start;
    char* out = (char*)malloc((size_t)len + 1);
    memcpy(out, s + start, (size_t)len); out[len] = 0;
    return out;
}
char* __xlang_str_reverse(const char* s) {
    int32_t n = (int32_t)strlen(s);
    char* out = (char*)malloc(n + 1);
    for (int32_t i = 0; i < n; i++) out[i] = s[n - 1 - i];
    out[n] = 0;
    return out;
}
char* __xlang_str_translate(const char* s, const char* from, const char* to) {
    int32_t n = (int32_t)strlen(s);
    int32_t tn = (int32_t)strlen(to);
    char* out = (char*)malloc(n + 1);
    for (int32_t i = 0; i < n; i++) {
        char* p = strchr(from, s[i]);
        out[i] = (p && (p - from) < tn) ? to[p - from] : s[i];
    }
    out[n] = 0;
    return out;
}
char* __xlang_read_line() {
    char* buf = (char*)malloc(65536);
    if (!fgets(buf, 65536, stdin)) { buf[0] = 0; return buf; }
    int32_t n = (int32_t)strlen(buf);
    if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = 0;
    return buf;
}
static char* __sb_buf = 0;
static size_t __sb_len = 0;
static size_t __sb_cap = 0;
void __xlang_sb_new() {
    if (!__sb_buf) { __sb_buf = (char*)malloc(65536); __sb_cap = 65536; }
    __sb_len = 0; __sb_buf[0] = 0;
}
void __xlang_sb_push(const char* s) {
    size_t sl = strlen(s);
    if (__sb_len + sl + 1 > __sb_cap) {
        while (__sb_len + sl + 1 > __sb_cap) __sb_cap *= 2;
        __sb_buf = (char*)realloc(__sb_buf, __sb_cap);
    }
    memcpy(__sb_buf + __sb_len, s, sl);
    __sb_len += sl;
    __sb_buf[__sb_len] = 0;
}
const char* __xlang_sb_str() {
    return __sb_buf ? __sb_buf : "";
}
void __xlang_sb_push_char(int32_t c) {
    if (__sb_len + 2 > __sb_cap) { __sb_cap *= 2; __sb_buf = (char*)realloc(__sb_buf, __sb_cap); }
    __sb_buf[__sb_len++] = (char)c;
    __sb_buf[__sb_len] = 0;
}
char* __xlang_time_str() {
    setlocale(LC_TIME, "");
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    char* s = (char*)malloc(64);
    strftime(s, 64, "%a %b %e %H:%M:%S %Z %Y", tm);
    return s;
}

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/utsname.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sched.h>
int32_t __xlang_tcp_listen(int32_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 64);
    return (int32_t)fd;
}
char* __xlang_recv_str(int32_t fd) {
    static char buf[65536];
    ssize_t n = recv(fd, buf, 65535, 0);
    if (n < 0) n = 0;
    buf[n] = 0;
    return buf;
}
// epoll event-loop support. A single global epoll fd + a ready-fd
// ring buffer, so xlang treats epoll_wait(timeout) as "next ready fd".
#define __XLANG_EPQ_CAP 8192
static int32_t __xlang_epfd_g = -1;
static int __xlang_epq_fd[__XLANG_EPQ_CAP];
static int __xlang_epq_head = 0;
static int __xlang_epq_tail = 0;
int32_t __xlang_epoll_create() {
    __xlang_epfd_g = epoll_create1(0);
    return __xlang_epfd_g;
}
int32_t __xlang_epoll_add(int32_t fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    return epoll_ctl(__xlang_epfd_g, EPOLL_CTL_ADD, fd, &ev) == 0 ? 0 : -1;
}
int32_t __xlang_epoll_del(int32_t fd) {
    epoll_ctl(__xlang_epfd_g, EPOLL_CTL_DEL, fd, 0);
    return 0;
}
int32_t __xlang_epoll_wait(int32_t timeout) {
    if (__xlang_epq_head != __xlang_epq_tail) {
        int fd = __xlang_epq_fd[__xlang_epq_head];
        __xlang_epq_head = (__xlang_epq_head + 1) % __XLANG_EPQ_CAP;
        return (int32_t)fd;
    }
    struct epoll_event events[256];
    int n = epoll_wait(__xlang_epfd_g, events, 256, timeout);
    if (n <= 0) return -1;
    int i;
    for (i = 0; i < n; i++) {
        __xlang_epq_fd[__xlang_epq_tail] = events[i].data.fd;
        __xlang_epq_tail = (__xlang_epq_tail + 1) % __XLANG_EPQ_CAP;
    }
    int fd = __xlang_epq_fd[__xlang_epq_head];
    __xlang_epq_head = (__xlang_epq_head + 1) % __XLANG_EPQ_CAP;
    return (int32_t)fd;
}
int32_t __xlang_set_nonblock(int32_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0 ? 0 : -1;
}
int32_t __xlang_set_nodelay(int32_t fd) {
    int flag = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0 ? 0 : -1;
}
int32_t __xlang_open_read(const char* path) {
    return (int32_t)open(path, O_RDONLY);
}
int32_t __xlang_sendfile_fd(int32_t out_fd, int32_t in_fd, int32_t len) {
    off_t off = 0;
    size_t remaining = (size_t)len;
    while (remaining > 0) {
        ssize_t s = sendfile(out_fd, in_fd, &off, remaining);
        if (s > 0) { remaining -= (size_t)s; continue; }
        // non-blocking socket buffer full: retry when writable. This keeps
        // the send complete (no truncation) on non-blocking sockets while
        // staying out of the way for small bodies that never hit EAGAIN.
        if (s < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) { sched_yield(); continue; }
        break;
    }
    return (int32_t)((size_t)len - remaining);
}
int32_t __xlang_dir_count(const char* path) {
    DIR* d = opendir(path);
    if (!d) return 0;
    int32_t n = 0;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}
char* __xlang_dir_entry(const char* path, int32_t idx) {
    DIR* d = opendir(path);
    if (!d) return "";
    struct dirent* e;
    int32_t i = 0;
    while ((e = readdir(d))) {
        if (i == idx) {
            char* copy = (char*)malloc(strlen(e->d_name) + 1);
            strcpy(copy, e->d_name);
            closedir(d);
            return copy;
        }
        i++;
    }
    closedir(d);
    return "";
}
int32_t __xlang_is_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}
int32_t __xlang_file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (int32_t)st.st_size;
}
int32_t __xlang_file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 ? 1 : 0;
}
char* __xlang_getcwd() {
    char* buf = (char*)malloc(4096);
    return getcwd(buf, 4096);
}
char* __xlang_readlink(const char* path) {
    char* buf = (char*)malloc(4096);
    ssize_t n = readlink(path, buf, 4095);
    if (n < 0) { buf[0] = 0; return buf; }
    buf[n] = 0;
    return buf;
}
char* __xlang_realpath(const char* path) {
    char* resolved = realpath(path, NULL);
    return resolved ? resolved : "";
}
extern char** environ;
int32_t __xlang_env_count() {
    int32_t n = 0;
    while (environ[n]) n++;
    return n;
}
const char* __xlang_env_entry(int32_t idx) {
    extern char** environ;
    int32_t n = 0;
    while (environ[n]) {
        if (n == idx) return environ[n];
        n++;
    }
    return "";
}
const char* __xlang_tty() {
    char* name = ttyname(0);
    return name ? name : "";
}
const char* __xlang_uname_machine() {
    struct utsname u;
    if (uname(&u) != 0) return "";
    char* m = (char*)malloc(strlen(u.machine) + 1);
    strcpy(m, u.machine);
    return m;
}
#endif

int32_t main(int argc, char** argv);

int32_t main(int argc, char** argv) {
    __xlang_argc_g = argc;
    __xlang_argv_g = argv;
    int32_t fd = __xlang_tcp_listen(28080);
    int32_t i = 0;
    while ((i < 15)) {
        int32_t pid = fork();
        if ((pid == 0)) {
            break;
        }
        (i = (i + 1));
    }
    while (true) {
        int32_t client = accept(fd, 0, 0);
        while (true) {
            const char * req = __xlang_recv_str(client);
            if (((int32_t)strlen(req) == 0)) {
                break;
            }
            const char * body = __xlang_read_file("/home/wzu/payload.txt");
            int32_t len = (int32_t)strlen(body);
            const char * resp = __xlang_str_concat(__xlang_str_concat("HTTP/1.1 200 OK\r\nContent-Length: ", __xlang_int_to_str(len)), __xlang_str_concat("\r\n\r\n", body));
            send(client, resp, strlen(resp), 0);
        }
        close(client);
    }
    return 0;
}
