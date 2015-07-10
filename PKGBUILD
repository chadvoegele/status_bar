# Maintainer: Chad Voegele

pkgname=status_bar-git
pkgver=1
pkgrel=1
pkgdesc="Status bar build on top of dzen"
arch=('i686' 'x86_64')
license=('LGPL')
depends=(dzen2)
makedepends=(git)
source=('git+https://github.com/chadvoegele/status_bar.git')
md5sums=('SKIP')
provides=('status_bar')

build() {
  cd "$srcdir/status_bar"
  make
}

package() {
  cd "$srcdir/status_bar"
  make PREFIX=/usr DESTDIR="$pkgdir" install

  for file in $(find res/*.xbm); do
    install -Dm644 ${file} "${pkgdir}/usr/share/status_bar/$(basename ${file})"
  done
}
