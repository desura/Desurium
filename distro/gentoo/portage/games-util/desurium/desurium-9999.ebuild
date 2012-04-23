# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

inherit check-reqs cmake-utils eutils git-2 games

EGIT_REPO_URI="git://github.com/lodle/Desurium.git"

CHECKREQS_DISK_BUILD="3G"

DESCRIPTION="Free software version of Desura game client"
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="builtin-curl builtin-tinyxml debug"

DEPEND="
	app-arch/bzip2
	dev-db/sqlite
	dev-lang/yasm
	dev-libs/boost
	dev-libs/libevent
	dev-libs/libxml2
	dev-libs/openssl
	!builtin-tinyxml? (
		|| ( <dev-libs/tinyxml-2.6.2-r2[-stl]
		    >=dev-libs/tinyxml-2.6.2-r2
		)
	)
	dev-lang/v8
	dev-vcs/subversion
	gnome-base/libgnome-keyring
	media-libs/flac
	media-libs/libpng
	media-libs/libwebp
	media-libs/speex
	!builtin-curl? (
		net-misc/curl
	)
	builtin-curl? (
		net-dns/c-ares
	)
	>=sys-devel/gcc-4.5
	sys-libs/zlib
	virtual/jpeg
	>=x11-libs/gtk+-2.24
	x11-misc/xdg-utils
"

RDEPEND="${DEPEND}"

S="${WORKDIR}/desura"

# pkg_pretend not working EAPI < 4
if [[ ${EAPI} != 4 ]]; then
	src_unpack() {
		check-reqs_pkg_pretend
		git-2_src_unpack
	}
fi

src_configure() {
	mycmakeargs=(
		$(cmake-utils_use_with builtin-curl ARES)
		$(cmake-utils_use debug DEBUG)
		-DBUILD_CEF=TRUE
		-DCMAKE_INSTALL_PREFIX=${GAMES_PREFIX}/${PN}
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install

	dogamesbin ${FILESDIR}/launch-desura.sh
	doicon ${FILESDIR}/desura.png
	make_desktop_entry "launch-desura.sh" "Desurium" "desura"

	prepgamesdirs
}

