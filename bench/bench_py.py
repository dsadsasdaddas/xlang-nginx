#!/usr/bin/env python3
"""Concurrent keepalive HTTP load generator (ab/wrk substitute).

Usage: bench_py.py <port> <total_requests> <concurrency>

Each of <concurrency> threads keeps ONE persistent connection open and fires
<total/concurrency> GET requests over it (HTTP keepalive), counting only fully
received 5-byte "hello" bodies. Reports req/s. This stresses the event loop's
ability to multiplex many simultaneous keepalive connections — exactly where
epoll beats prefork.
"""
import socket, sys, time, threading

HOST = "127.0.0.1"
PORT = int(sys.argv[1])
TOTAL = int(sys.argv[2])
CONC = int(sys.argv[3])

REQ = b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"

lock = threading.Lock()
done = [0]
errors = [0]


def worker(n):
    buf = b""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
    except Exception:
        with lock:
            errors[0] += 1
        return
    for _ in range(n):
        try:
            s.sendall(REQ)
            while b"hello" not in buf:
                chunk = s.recv(4096)
                if not chunk:
                    with lock:
                        errors[0] += 1
                    s.close()
                    return
                buf += chunk
            idx = buf.find(b"hello")
            buf = buf[idx + 5:]
            with lock:
                done[0] += 1
        except Exception:
            with lock:
                errors[0] += 1
            s.close()
            return
    s.close()


per = max(1, TOTAL // CONC)
threads = [threading.Thread(target=worker, args=(per,)) for _ in range(CONC)]
t0 = time.time()
for t in threads:
    t.start()
for t in threads:
    t.join()
dt = time.time() - t0
print(f"req/s={done[0]/dt:.0f}  ok={done[0]}  err={errors[0]}  time={dt:.2f}s  conc={CONC}")
