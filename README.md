# xlang-nginx — nginx replication in X Language

HTTP servers written in [xlang](https://github.com/dsadsasdaddas/xlang), benchmarked against real **nginx 1.28** (built from source).

## Servers

- `server.x` — single-request HTTP server
- `server_loop.x` — looping (blocking) server
- `server_keepalive.x` — keepalive server
- `server_prefork.x` — prefork 16-worker server (nginx/apache model)
- `server_file.x` — static file serving
- `server_route.x` — request-path routing (nginx location-style)

## Benchmarks

See `bench/RESULTS.md` for full methodology and data.

| Workload | nginx | xlang |
|----------|-------|-------|
| Fixed response, keepalive 16-conc (prefork) | 77k req/s | **129k req/s** |
| 64KB file serving | 25.6k req/s | 22.7k req/s |

## Build

Requires the [xlang compiler](https://github.com/dsadsasdaddas/xlang):
```sh
xlangc c servers/server_prefork.x && cc -O2 -o server server_prefork.c && ./server
```
