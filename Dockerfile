FROM alpine:latest as builder

RUN echo http://dl-cdn.alpinelinux.org/alpine/edge/testing/ >> /etc/apk/repositories && \
    apk add --no-cache infiniband-diags-dev curl-dev g++ make cmake

WORKDIR /project/code
COPY CMakeLists.txt ./
COPY src/ ./src
COPY 3d_party  ./3d_party

WORKDIR /project/code/build
RUN cmake .. && cmake --build . --target infiniband_radar_daemon -- -j 2


FROM alpine:latest

RUN echo http://dl-cdn.alpinelinux.org/alpine/edge/testing/ >> /etc/apk/repositories && \
    apk add --no-cache infiniband-diags libcurl

COPY --from=builder /project/code/build/infiniband_radar_daemon /usr/sbin/infiniband_radar_daemon

VOLUME /config
WORKDIR /config
ENTRYPOINT ["/usr/sbin/infiniband_radar_daemon"]
