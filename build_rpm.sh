#!/usr/bin/env bash

docker build --no-cache -t infiniband_radar_daemon_builder -f Dockerfile-build .

mkdir -p packages

docker create --name ibr_builder infiniband_radar_daemon_builder

docker cp ibr_builder:/project/packages ./

docker rm -f ibr_builder

docker rmi infiniband_radar_daemon_builder
