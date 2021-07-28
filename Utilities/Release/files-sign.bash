#!/usr/bin/env bash

set -e

gpg --armor --detach-sign cmake-*-SHA-256.txt
