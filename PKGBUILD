# Maintainer: Chad Voegele

pkgname=status_bar-git
pkgver=1
pkgrel=1
pkgdesc="Status bar for displaying system information."
arch=('i686' 'x86_64')
license=('LGPL')
depends=('ttf-ionicons' 'jq' 'alsa-lib')
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
}
