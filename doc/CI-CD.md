# CI/CD

Thanks to the migration to https://forge.a-lec.org/, Speed Dreams enjoys
automated builds with [Forgejo Actions]. As of the time of this writing,
the following artifacts are generated:

- [AppImage]
(see [`build-appimage.yml`](../.forgejo/workflows/build-appimage.yml)):
    - `x86_64`
    - `aarch64`

- Installers for
(see [`build-w64-mingw32.yml`](../.forgejo/workflows/build-w64-mingw32.yml)):
    - `i686`

There is also a workflow for building `.deb` packages
(see [`build-deb.yml`](../.forgejo/workflows/build-deb.yml)), but it has
been limited to tags only, since it is assumed distributions will eventually
package Speed Dreams on their own.

These builds require many dependencies, so the procedure is a bit fragile.
Fortunately, we build our own [OCI images] with [Podman] in order to make
the environment reproducible and hopefully easier to use for developers.

## Background

During the first discussions about our planned CI/CD design with
[Libre en Communs] (who generously host the forge and our VPS), there were
concerns about relying on the base images typically downloaded from third-party
networks, most prominently [DockerHub].

Therefore, it was suggested to build our own base images from scratch,
using [`mmdebstrap`], from which we build more specialised images.

## Base images

The following base images are required:

- `forge.a-lec.org/speed-dreams/ubuntu:focal`
- `forge.a-lec.org/speed-dreams/ubuntu:jammy`

Ubuntu `focal` is used by the AppImage builds because relying on older
versions of `glibc` can help with binary compatibility. For other builds,
Ubuntu `jammy` is good enough.

Both are built from the [`ubuntu-docker`] repository, which defines
[a shell script](https://forge.a-lec.org/speed-dreams/ubuntu-docker/src/branch/main/run.sh)
to bootstrap an OCI image.

The two base images are therefore created with:

```
$ ./run.sh # by default, focal is selected
$ ./run.sh -d jammy
```

**IMPORTANT NOTE:** `run.sh` relies on the `podman` executable to be available
from the `$PATH`.

## AppImage image

The CI/CD relies on the following image:

- `forge.a-lec.org/speed-dreams/ubuntu-appimage:focal`

This image can be constructed from the [`oci-appimage`] repository with:

```
$ podman build -t forge.a-lec.org/speed-dreams/ubuntu-appimage:focal .
```

Optionally, intermediate layers can be discarded from the build cache by
exporting the following environment variable before running `podman build`:

```sh
export BUILDAH_LAYERS=false
```

Since the image relies on a very old Ubuntu version (`focal`, released in 2020),
some packages are too old to build Speed Dreams and the dependencies required to
build the AppImage. More specifically, the following packages are built from
source by the `Dockerfile`:

- Python 3.10.16.
    - [`appimage-builder`] seems to break with other Python versions.
- CMake 3.22.6.
    - Speed Dreams requires `3.22` or higher. **NOTE:** this is not explicitly
    requested by the `CMakeLists.txt` and should be fixed accordingly.
    - [`add_library INTERFACE`] with additional sources requires CMake 3.19 or
    higher.
- cJSON 1.7.18.
    - Required by Speed Dreams and its in-game download manager.

The following repositories had to be forked:

- [`appimage-builder`]
    - `appimage-builder` was forked because of ghost dependencies against
    GitHub, a non-free service. See [commit `343f8740`] for further reference.
- [`AppImageKit`]
    - [The `README.md`](https://forge.a-lec.org/speed-dreams/appimagekit#readme)
    provides a detailed explanation of the several issues found in the upstream
    repository.
- [`libappimage`]
    - Required by `AppImageKit`, upstream contained broken patches. See
    [commit `d9b68ace`](https://forge.a-lec.org/speed-dreams/AppImageKit/commit/d9b68acebd497ef432b81fc5bfc41c1e9c7ffb10)
    for further reference.
- [`AppRun`]
    - [Its release artifacts](https://forge.a-lec.org/speed-dreams/AppRun/releases)
    are required by `appimage-builder`, which were originally hosted on GitHub.

[The releases](https://forge.a-lec.org/speed-dreams/AppImageKit/releases) are
particularly important as they include the runtime executables consumed by
`appimage-builder`. The release page details how these executables are built,
repeated below for the sake of convenience:

```sh
cmake -B build -DUSE_SYSTEM_MKSQUASHFS=ON -DBUILD_TESTING=OFF -DUSE_SYSTEM_XZ=ON-DUSE_SYSTEM_LIBARCHIVE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/
mv build/src/runtime build/src/runtime-$(uname -m)
```

Finally, `forge.a-lec.org/speed-dreams/ubuntu-appimage:focal` relies on a few
more self-made tools:

- [`upload-artifact`]
- [`forgejo-clone`]

These two tools share the same goal: provide features similar to those provided
by the [`actions/upload-artifact`] and [`actions/checkout@v4`] actions,
respectively, without requiring a 2GB base `node` image (i.e., [Node.js]) to
execute what are otherwise trivial programs.

While this might be thought as too opinionated, disk space was initially scarce
enough in the VPS to care about it, let alone the concerns about having to run
a full-blown JavaScript interpreter like Node.js for what can be achieved with
a few calls to [`curl`].

## Images for Windows builds

The CI/CD builds Windows "32-bit" (`i686-w64-mingw32`) installers thanks to
[CPack] and [NSIS]. See [`nsis.cmake`](../cpack/nsis.cmake) for further
reference about the CPack macros defined by the project.

For Windows builds, a different image is used, namely
`forge.a-lec.org/speed-dreams/ubuntu-i686-w64-mingw32:jammy`. This image is
built from the `forge.a-lec.org/speed-dreams/ubuntu-appimage:jammy` base image
with the [`i686-w64-mingw32-docker`] repository. Similarly, the image can be
built with:

```
$ podman build -t forge.a-lec.org/speed-dreams/ubuntu-i686-w64-mingw32:jammy .
```

**IMPORTANT NOTE:** this image depends on a few artifacts from another image,
namely `forge.a-lec.org/speed-dreams/nsis-i686-w64-mingw32`, which is in turn
built from the [`nsis-docker`] repository with:

```
$ podman build -t forge.a-lec.org/speed-dreams/nsis-i686-w64-mingw32:jammy .
```

`forge.a-lec.org/speed-dreams/nsis-i686-w64-mingw32` contains a NSIS compiler
configured for `i686-w64-mingw32` (aka "32-bit" Windows) that can be used by
CPack to generate the installer `.exe`.

As opposed to Linux builds, where dependencies can be obtained from the
distribution, Windows builds unfortunately require to build *all* dependencies
from source before building Speed Dreams.
[`packaging/3rdParty-devel/README.md`](../packaging/3rdParty-devel/README.md)
has more information about how the `3rdParty` toolchain is generated.
For the purposes of this documentation, the toolchain is available as
`THIRDPARTY_URL` from the
[project settings](https://forge.a-lec.org/speed-dreams/speed-dreams-code/settings/actions/variables).

## Variables and secrets

Forgejo Actions allows to define variables and secrets that are injected into
the containers to access public and private resources, respectively.

The following variables are required:

- `STORE_URL`: defines the URL to the [`slcl`] instance where the uploads
shall be uploaded to.
- `THIRDPARTY_URL`: defines the publicly accessible URL where the `3rdParty`
tarball can be downloaded.

The following secrets are required:

- `STORE_USERNAME`: username for the `slcl` instance.
- `STORE_PASSWORD`: password for the `slcl` instance.

## Blob storage

The CI/CD relies on a [`slcl`] instance to upload its artifacts to. `slcl` is
a small and JS-less alternative to full-blown file storages such as [Nextcloud],
while providing features such as:

- Read-only, public file sharing.
- Private file server.
- Configurable user quota.

**Disclaimer:** the CI/CD for Speed Dreams and `slcl` were both written by the
same author.

[AppImage]: https://appimage.org/
[Forgejo Actions]: https://forgejo.org/docs/latest/user/actions/
[OCI images]: https://en.wikipedia.org/wiki/Open_Container_Initiative
[Podman]: https://github.com/containers/podman
[Libre en Communs]: https://a-lec.org
[DockerHub]: https://hub.docker.com
[`mmdebstrap`]: https://gitlab.mister-muffin.de/josch/mmdebstrap/
[`ubuntu-docker`]: https://forge.a-lec.org/speed-dreams/ubuntu-docker
[`oci-appimage`]: https://forge.a-lec.org/speed-dreams/oci-appimage
[`appimage-builder`]: https://forge.a-lec.org/speed-dreams/appimage-builder
[`AppImageKit`]: https://forge.a-lec.org/speed-dreams/AppImageKit
[`libappimage`]: https://forge.a-lec.org/speed-dreams/libappimage
[`AppRun`]: https://forge.a-lec.org/speed-dreams/AppRun
[commit `343f8740`]: https://forge.a-lec.org/speed-dreams/appimage-builder/commit/343f87408bb2abee299d2c1415bcc62849202850
[`actions/checkout@v4`]: https://code.forgejo.org/actions/checkout
[`actions/upload-artifact`]: https://code.forgejo.org/actions/upload-artifact
[Node.js]: https://en.wikipedia.org/wiki/Nodejs
[`curl`]: https://curl.se/
[`upload-artifact`]: https://forge.a-lec.org/speed-dreams/upload-artifact
[`forgejo-clone`]: https://forge.a-lec.org/speed-dreams/forgejo-clone
[`add_library INTERFACE`]: https://cmake.org/cmake/help/latest/command/add_library.html#interface-with-sources
[CPack]: https://cmake.org/cmake/help/latest/manual/cpack.1.html
[NSIS]: https://nsis.sourceforge.io/
[`i686-w64-mingw32-docker`]: https://forge.a-lec.org/speed-dreams/i686-w64-mingw32-docker
[`nsis-docker`]: https://forge.a-lec.org/speed-dreams/nsis-docker
[`slcl`]: https://gitea.privatedns.org/xavi/slcl
[Nextcloud]: https://nextcloud.com/
