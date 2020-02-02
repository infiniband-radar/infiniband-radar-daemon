FROM debian:buster-slim as builder

RUN apt update && \
    apt install -y cmake make g++ libibverbs-dev libibnetdisc-dev libibmad-dev libopensm-dev libcurl4-openssl-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /project/code
COPY CMakeLists.txt ./
COPY src/ ./src
COPY 3d_party  ./3d_party

WORKDIR /project/code/build
RUN cmake .. && cmake --build . --target infiniband_radar_daemon -- -j 2

FROM debian:buster-slim

RUN apt update && \
    apt install -y libcurl4 libibnetdisc5 && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /project/code/build/infiniband_radar_daemon /usr/sbin/infiniband_radar_daemon

VOLUME /config
WORKDIR /config
ENTRYPOINT ["/usr/sbin/infiniband_radar_daemon"]
