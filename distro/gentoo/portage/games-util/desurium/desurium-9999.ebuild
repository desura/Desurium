# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

inherit cmake-utils git-2 games

EGIT_REPO_URI="git://github.com/lodle/Desurium.git"
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
    media-libs/flac
    media-libs/libpng
    media-libs/speex
	!builtin-curl? (
        net-misc/curl[ares]
    )
    builtin-curl? (
        net-dns/c-ares
    )
    >=sys-devel/gcc-4.5
    sys-libs/zlib
    virtual/jpeg
	>=x11-libs/gtk+-2.24
	x11-misc/xdg-utils"

RDEPEND="${DEPEND}"

S="${WORKDIR}/desura"

src_configure() {
	mycmakeargs=(
        $(cmake-utils_use_with builtin-curl ARES)
        $(cmake-utils_use debug DEBUG)
    )
	cmake-utils_src_configure
}

