#!/bin/bash

seq 1 250 | parallel -j 0 -I{} curl -x socks5h://127.0.0.1:1080  https://discord.com/api/download?platform=linux&format=deb | sha256sum

