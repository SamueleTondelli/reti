#!/usr/bin/env python3
import socket
import sys
import os
import time


def send_err(conn):
    conn.sendall('- ERR\r\n'.encode('ascii'))


def gen_sequence(seed, iterations):
    if iterations <= 0:
        return []
    if iterations == 1:
        return [f"1{seed}"]
    
    l = gen_sequence(seed, iterations-1)
    old_d = l[-1][0]
    n_curr_d = 0
    res = ""
    for d in l[-1]:
        if d == old_d:
            n_curr_d += 1
        else:
            res += f'{n_curr_d}{old_d}'
            old_d = d
            n_curr_d = 1
    
    res += f'{n_curr_d}{old_d}'
    l.append(res)
    print(iterations)
    return l


def handle_request(conn):
    data = conn.recv(1024).decode('ascii')
    fields = data.split(',')
    if len(fields) != 2:
        print("wrong n fields")
        send_err(conn)
        return
    if not fields[1].endswith("\r\n"):
        print("no rn")
        send_err(conn)
        return

    try:
        seed = int(fields[0])
        iterations = int(fields[1][:-2])
    except:
        print("wrong format params")
        send_err(conn)
        return
    
    if seed >= 9 or seed <= 0 or iterations <= 0:
        print("bad numbers")
        send_err(conn)
        return
    
    print("params ok")
    conn.sendall(f'+ OK {iterations} iterations on seed {seed}\r\n'.encode('ascii'))
    for i in gen_sequence(seed, iterations):
        conn.sendall(f'{i}\r\n'.encode('ascii'))
    time.sleep(1)


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(('localhost', 8080))
    s.listen()
    
    while True:
        conn, addr = s.accept()
        pid = os.fork()
        if pid == 0:
            handle_request(conn)
            sys.exit()
        else:
            conn.close()
