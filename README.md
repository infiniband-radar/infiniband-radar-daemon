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
