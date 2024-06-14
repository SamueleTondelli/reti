#!/usr/bin/env python3
import socket


def send_request(s):
    pass


HOST = ""
PORT = 0

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:    #tcp
#with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:    #udp
    s.connect((HOST, PORT))
    send_request(s)
    s.close()