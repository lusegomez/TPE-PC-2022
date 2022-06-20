#!/bin/bash

seq 1 500 | parallel -j 0 -I{} curl -x socks5h://127.0.0.1:1080 "http://br.mirror.archlinux-br.org/iso/2022.06.01/archlinux-2022.06.01-x86_64.iso" | sha256sum

