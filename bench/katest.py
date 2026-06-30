import socket, sys, time
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 28082
N = int(sys.argv[2]) if len(sys.argv) > 2 else 2000
s = socket.socket(); s.connect(("127.0.0.1", PORT))
s.settimeout(5)
req = b"GET / HTTP/1.1\r\nHost: x\r\nConnection: keep-alive\r\n\r\n"
t0 = time.time()
ok = 0
for i in range(N):
    try:
        s.sendall(req)
    except Exception as e:
        print(f"send fail at {i}: {e}"); break
    buf = b""
    got = False
    try:
        while True:
            c = s.recv(4096)
            if not c:
                print(f"server closed at req {i}"); break
            buf += c
            if b"\r\n\r\n" in buf:
                hdr, _, body = buf.partition(b"\r\n\r\n")
                # parse content-length
                cl = 0
                for line in hdr.split(b"\r\n"):
                    if line.lower().startswith(b"content-length:"):
                        cl = int(line.split(b":")[1].strip())
                if len(body) >= cl:
                    got = True; break
    except socket.timeout:
        print(f"TIMEOUT at req {i} (buf {len(buf)}B)"); break
    if got: ok += 1
    else: break
dt = time.time() - t0
print(f"ok={ok}/{N}  time={dt:.2f}s  req/s={ok/dt:.0f}" if dt>0 else f"ok={ok}")
s.close()
