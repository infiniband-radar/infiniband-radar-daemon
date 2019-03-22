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
- Colliding hostnames will occure if you are using (not manully renamed) unmaned switches



# Build

## Create RPM file

( Requires docker )

```sh
./build_rpm.sh
```

## Install

```sh
sudo yum localinstall packages/fabric-radar-daemon_<version>.rpm
```

## Edit config

```sh
cd /etc/fabric-radar
cp config.template.json config.<FabricId>.json
vim config.<FabricId>.json
```

## Service

```sh
systemctl enable fabric-radar@<FabricId>
systemctl start fabric-radar@<FabricId>
```

## Requirement for development:

```sh
apt install libibverbs-dev libibnetdisc-dev libibmad-dev libopensm-dev libcurl4-openssl-dev
```

# Licence
GNU GENERAL PUBLIC LICENSE

Developed by the [Deutsches Elektronen-Synchrotron](https://www.desy.de/)
