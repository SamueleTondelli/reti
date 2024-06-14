#!/usr/bin/env python3
import socket
import sys
import os
import time


def handle_request(conn, cli_addr):
    time.sleep(1) #dopo la risposta


port = 0

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:    #tcp
#with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:    #udp
    s.bind(('localhost', port))
    s.listen()
    
    while True:
        conn, addr = s.accept()
        handle_request(conn, addr)  #single process
        
        '''
        #multi process
        pid = os.fork()
        if pid == 0:
            handle_request(conn)
            sys.exit()
        else:
            conn.close()
        '''