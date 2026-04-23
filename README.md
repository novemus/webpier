# README

The [WebPier](https://github.com/novemus/webpier) app is designed to exchange TCP services with remote hosts, primarily those located behind the NAT. Peers do not need to have public IP addresses and no third-party relay servers are used. The `WebPier` creates a direct UDP/TCP/SSL tunnel between the hosts and maps the remote TCP service to the local interface, or forwards the local TCP service to the remote side. The UDP/TCP-hole-punching technique using STUN server is used to overcome NAT, and Email or DHT services are used as a rendezvous for exchanging endpoints. This program is essentially a graphical shell for the [plexus](https://github.com/novemus/plexus) and [wormhole](https://github.com/novemus/wormhole) utilities and is designed to provide a user-friendly mean for managing the export/import of TCP services.

## What are the benefits of using the WebPier?

First of all, it is safety. Traffic of your services doesn't pass through third-party servers. There is no need to make your services public. You only open them to whom you wish and verify with the public key on the stage of tunnel creation. The obscuration feature protects the TCP/UDP tunnels from protocol stack detection. Using `WebPier` as alternative to VPN gives a better throughput because of minimal packet route and absence of packet processing on the intermediate server. This is a convenient mean to provide individual access to remote non-public servers.

## How about reliability?

Due to the fact that `WebPier` does not use third-party relay servers, the possibility to create a tunnel depends on the settings of NAT or firewall. The NAT should realize *Full Cone* mapping of the internal endpoint to the public NAT interface. Fortunately, providers usually implement this policy on their NATs. If both piers are located behind the same NAT, then the *hairpin* policy must be implemented on it so that packets from the internal endpoint can be transmitted back to the internal network. If there is no NAT and strict firewall on one side, then strict NAT settings on the other side are not a problem. In future releases, it is planned to introduce the use of custom TURN server to cover the cases of bad NATs, but in practice this happens infrequently.

## Using

The easiest way to get started is to download and install the [prebuilt](https://github.com/novemus/webpier/releases) package for Linux, Windows and MacOS platforms.

On the first start, you will be prompted to define the local *pier* identity.

![welcome](resources/welcome.png)

It consists from the two parts. The first one is the owner identifier, which should be your email address if you want to use the email as the rendezvous service. It can be any word in case of using DHT network as the rendezvous, but using an email address is more unified approach. In addition, this is a good way to distinguish participants and avoid naming collisions. The second part of the identity is the name of the local *pier* which must be unique for the owner. After providing the identifier, a pair of cryptographic keys will be generated. They will be used to protect rendezvous messages and for SSL tunneling.

Then you have to setup some of parameters by the dialog from the *"File -> Settings..."* menu.

![settings](resources/settings.png)

You should specify accessible STUN servers if NAT traverse is needed, DHT bootstrap URL or(and) your email account. Then you can add export services.

![service](resources/service.png)

* **Name** - name of the service to refer it in the rendezvous service
* **Pier** - remote pier to export or import the TCP service
* **Address** - address:port pair of the exporting service or local address:port pair to import the remote service
* **Tunnel** - tunnel protocol, supported UDP/TCP/SSL protocols or Auto to negotiate most suitable one at runtime
* **Gateway** - local address:port pair for the transport tunnel
* **Schema** - tunnel connection schema for the local pier, supported Client/Server/Mutual schemes or Auto to negotiate most suitable one at runtime
* **Autostart** - should the service be run with the application startup or manually
* **Obscure** - should the transport UDP/TCP tunnel be obfuscated, must be equal for both sides
* **Rendezvous** - rendezvous service, must match the remote side

After that you have to create the *offer* for your partner. Invoke the dialogs from the *"File -> Create an offer..."* menu, select your services you want to offer and save the *offer* file. The *offer* will also contain public key of your pier.

![share](resources/share.png)

Send the *offer* to your partner, retrieve the counter *offer* and adopt it by the dialog from *"File -> Upload an offer..."* menu. Choose from offered services that you want to import and mark your services that you want to share.

![adopt](resources/adopt.png)

Your partner must adopt your *offer* too. After that you can manage the services from the context menu or from the desktop tray. It takes some time to establish a tunnel. You will be notified about the tunnel status and service indicator will change color. The services can be created and edited later, but their parameters must be synchronized with the remote piers. It is not necessary to exchange offers again.

## Build

To build the project from the source you will need to provide the following dependencies: [plexus](https://github.com/novemus/plexus), [wormhole](https://github.com/novemus/wormhole), [wxWidgets](https://github.com/wxWidgets/wxWidgets), [openssl](https://github.com/openssl/openssl), [boost](https://github.com/boostorg/boost) libraries and their interface dependencies. For example, `plexus` library also needs the [tubus](https://github.com/novemus/tubus) and [opendht](https://github.com/savoirfairelinux/opendht) library with their own dependencies. If that doesn't scare you then try to build the project.

Note that the `CMakePresets.json` directs the build output to the `out` directory next to the project source folder. The CPack presets are based on the static release build.

The Linux CMake presets assume that the dependencies are installed and accessible via CMAKE_PREFIX_PATH and PKG_CONFIG_PATH variables or located in the default install directory, which is set as `${sourceParentDir}/out/$env{HOSTNAME}/install/x64-static` for the static build and `${sourceParentDir}/out/$env{HOSTNAME}/install/x64` for the shared build.

```console
$ git clone https://github.com/novemus/webpier.git
$ cd webpier
$ cmake --preset=linux-static-release
$ cpack --preset=debian-package
```

The MacOS and Windows presets are based on the [vcpkg](https://vcpkg.io) tool chain to install and resolve dependencies, except of the [opendht](https://github.com/savoirfairelinux/opendht), [tubus](https://github.com/novemus/tubus), [plexus](https://github.com/novemus/plexus) and [wormhole](https://github.com/novemus/wormhole) libraries. You have to install them yourself, for example to the default install directory, which is set in the corresponding configure preset.

```console
$ git clone https://github.com/novemus/webpier.git
$ set VCPKG_ROOT=/path/to/vcpkg/root
$ cd webpier
$ cmake --preset=darwin-static-release # windows-static-release
$ cpack --preset=darwin-pkg # windows-msi
```

## Bugs and improvements

Feel free to [report](https://github.com/novemus/webpier/issues) bugs and [suggest](https://github.com/novemus/webpier/issues) improvements. 

## License

`WebPier` is licensed under the Apache License 2.0, which means that you are free to get and use it for commercial and non-commercial purposes as long as you fulfill its conditions. See the LICENSE.txt file for more details.

## Copyright

Copyright © 2025 Novemus Band. All Rights Reserved.
