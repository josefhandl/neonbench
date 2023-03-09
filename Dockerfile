
FROM ubuntu:22.04

RUN apt update && apt install -y --no-install-recommends \
        ca-certificates \
        git \
        cmake \
        make \
        gcc \
        g++ \
        clang-12 \
        nvidia-cuda-toolkit

COPY docker-entrypoint.sh /
RUN chmod +x /docker-entrypoint.sh

ENTRYPOINT ["/docker-entrypoint.sh"]
