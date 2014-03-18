
# Maintainer : mdevlamynck <matthias.devlamynck@mailoo.org>

pkgname=kdeplasma-addons-applets-nvidia-monitor-git
_gitname=nvidia-monitor
_gitrepo=https://github.com/mdevlamynck/nvidia-monitor.git
url=https://github.com/mdevlamynck/nvidia-monitor
pkgver=nvctrl
pkgrel=3
pkgdesc="An applet that monitor nvidia GPU using nvidia-settings (using proprietary drivers). Not supporting SLI yet."

arch=('i686' 'x86_64')
license=('GPL3')
depends=('nvidia-utils' 'kdebase-workspace' 'libxnvctrl')
makedepends=('git' 'cmake' 'kdeplasma-addons-libs')

source=("git+${_gitrepo}")
install=nvidia-monitor.install
md5sums=('SKIP')

#pkgver() {
#	cd ${srcdir}/${_gitname}
#	git tag|sort -V|tail -1|tr "-" "."|sed "s/^v//"
#}

build () {
	cd "${srcdir}/${_gitname}"
	#git checkout v${pkgver}
	git checkout nvctrl
	mkdir -p build
	cd build
	cmake .. -DCMAKE_INSTALL_PREFIX="${pkgdir}/$(kde4-config --prefix)" -DCMAKE_BUILD_TYPE=Release
	make
}

package() {
	cd "${srcdir}/${_gitname}/build"
	make install
}

