
# nvidia-monitor
 
An applet that monitor nvidia GPU using nvidia-settings (using proprietary drivers). Not supporting SLI yet.

## Installation

### ArchLinux

ArchLinux users, just use the aur pkg : [kdeplasma-addons-applets-nvidia-monitor-git](https://aur.archlinux.org/packages/kdeplasma-addons-applets-nvidia-monitor-git/)

### Other Distributions

Go in the cloned repo and compile the project :

```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="$(kde4-config --prefix)" -DCMAKE_BUILD_TYPE=Release
make
```

You can now do a `make install` as root to install the project or you can change the prefix given to cmake to install in a chroot (that's how the archlinux package is built).

If KDE can't see the applet try a `kbuildsycoca4` to update its cache.

### Known Issues

The linking process might fail if the option `--as-needed` is enabled.
