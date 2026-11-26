FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential gdb cmake libgtest-dev libgmock-dev \
      clang-format clang-tidy libcli11-dev \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /work
