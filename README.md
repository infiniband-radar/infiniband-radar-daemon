# InfiniBand Radar Daemon

If you have any questions please [open an issue](https://github.com/infiniband-radar/infiniband-radar-daemon/issues)

## Warning

This tool is using and resetting the counters of `IB_GSI_PORT_COUNTERS`
which can interfere with other monitoring tools.

Known interferences:
- `Mellanox's Managed Switch Interface` does not correctly display the used bandwidth

## Notice
- This daemon must run on a server that has access to InfiniBand
- Docker is **not required** to run it, see the [pre build releases](https://github.com/infiniband-radar/infiniband-radar-daemon/releases) 
- The web interface and API server can be found [here](https://github.com/infiniband-radar/infiniband-radar-web) and can run on a separated server.
- Colliding hostnames must be resolved, either by a [node_map file](https://www.systutorials.com/docs/linux/man/8-ibnetdiscover/#lbAM) or renaming hosts/switches.
- Colliding hostnames will occure if you are using (not manully renamed) unmanaged switches


# Usage

There are two options to run the daemon:

## Option 1: Use docker

Requires docker >= 17.05

### Create and edit config

```sh
mkdir -p config
cp config.template.json config/config.<FabricId>.json
vim config/config.<FabricId>.json
```

### Build docker container
```sh
docker build -t infiniband_radar_daemon .
```

### Run docker container
```sh
docker run  -v `pwd`/config:/config/:ro \
            --privileged \
            --userns=host \
            --restart unless-stopped \
             -d \
             infiniband_radar_daemon:latest config.<FabricId>.json
```

Don't forget to verify that that the container is running correctly (`docker ps` / `docker logs`)

## Option 2: Use RPM

### Create RPM file

[Download pre build releases](https://github.com/infiniband-radar/infiniband-radar-daemon/releases) **or** build manually:

```sh
# Requires docker
./build_rpm.sh
```

### Install

```sh
sudo yum localinstall packages/fabric-radar-daemon_<version>.rpm
```

### Create and edit config

```sh
cd /etc/fabric-radar
cp config.template.json config.<FabricId>.json
vim config.<FabricId>.json
```

### Service

```sh
systemctl enable fabric-radar@<FabricId>
systemctl start fabric-radar@<FabricId>
```

# Licence
GNU GENERAL PUBLIC LICENSE

Developed by the [Deutsches Elektronen-Synchrotron](https://www.desy.de/)
