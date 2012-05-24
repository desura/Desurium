# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

unset GIT_ECLASS

if [[ ${PV} = 9999* ]]; then
	EGIT_REPO_URI="git://github.com/lodle/Desurium.git"
	GIT_ECLASS="git-2"
	SRC_URI=""
else
	SRC_URI="https://github.com/downloads/lodle/Desurium/Desura-${PV}.tar.bz2"
fi

inherit check-reqs cmake-utils eutils ${GIT_ECLASS} games

CHECKREQS_DISK_BUILD="3G"

DESCRIPTION="Free software version of Desura game client"
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="+32bit debug +games-deps"

# some deps needed by some games
GAMESDEPEND="
	games-deps? (
		dev-lang/mono
		gnome-base/libglade
		media-libs/libogg
		media-libs/libpng:1.2
		media-libs/libsdl
		media-libs/libtheora
		media-libs/libvorbis
		media-libs/openal
		media-libs/sdl-image
		media-libs/sdl-ttf
		virtual/ffmpeg
		>=virtual/jre-1.6
	)
"

COMMON_DEPEND="
	app-arch/bzip2
	dev-db/sqlite
	>=dev-libs/boost-1.47
	dev-libs/libevent
	dev-libs/libxml2
	dev-libs/openssl:0

	|| ( <dev-libs/tinyxml-2.6.2-r2[-stl]
	    >=dev-libs/tinyxml-2.6.2-r2
	)

	dev-lang/v8
	gnome-base/libgnome-keyring
	media-libs/flac
	media-libs/libpng:0
	media-libs/libwebp
	media-libs/speex
	net-misc/curl[ares]
	>=sys-devel/gcc-4.5
	sys-libs/zlib
	virtual/jpeg
	x11-libs/gtk+:2
	x11-misc/xdg-utils

	32bit? (
		app-emulation/emul-linux-x86-gtklibs
		app-emulation/emul-linux-x86-gtkmmlibs
		app-emulation/emul-linux-x86-medialibs
		app-emulation/emul-linux-x86-opengl
		app-emulation/emul-linux-x86-sdl
		app-emulation/emul-linux-x86-soundlibs
		app-emulation/emul-linux-x86-xlibs[opengl]
		sys-devel/gcc[multilib]
	)
"

RDEPEND="
	x11-misc/xdg-user-dirs
	${COMMON_DEPEND}
	${GAMESDEPEND}
"

DEPEND="
	dev-lang/yasm
	dev-vcs/subversion
	${COMMON_DEPEND}
"

if [[ $PV != 9999* ]]; then
	S="${WORKDIR}/Desura-${PV}"
fi

pkg_setup() {
	check-reqs_pkg_setup
}

src_configure() {
	# -DWITH_ARES=FALSE will use system curl, because we force curl[ares] we have ares support
	local mycmakeargs=(
		-DWITH_ARES=FALSE
		$(cmake-utils_use debug DEBUG)
		$(cmake-utils_use 32bit 32BIT_SUPPORT)
		-DCMAKE_INSTALL_PREFIX=${GAMES_PREFIX}/${PN}
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install

	dosym "${GAMES_PREFIX}/${PN}/run.sh" "${GAMES_BINDIR}/${PN}.sh"

	doicon "${FILESDIR}/${PN}.png"
	make_desktop_entry "${PN}.sh" "Desurium"

	prepgamesdirs
}
