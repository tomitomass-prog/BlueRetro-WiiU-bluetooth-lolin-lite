#!/usr/bin/env python3
"""Offline deterministic protocol test; no dependencies."""
from pathlib import Path

def crc8(data):
    c=0
    for byte in data:
        c ^= byte
        for _ in range(8):
            c=((c<<1)^0x07 if c&0x80 else c<<1)&255
    return c

def encode(seq=1,buttons=1,hat=0):
    p=bytearray(20)
    p[:2]=b'\xa5\x5a'
    p[2]=seq;p[3]=buttons&255;p[4]=buttons>>8;p[5]=hat
    p[12]=1
    p[-1]=crc8(p[:-1])
    return p

if __name__=='__main__':
    p=encode()
    assert len(p)==20 and p[0:2]==b'\xa5\x5a' and p[-1]==crc8(p[:-1])
    p[3]^=1
    assert p[-1]!=crc8(p[:-1]), 'CRC must detect bit flip'
    print('PASS: size, sync, CRC integrity')
