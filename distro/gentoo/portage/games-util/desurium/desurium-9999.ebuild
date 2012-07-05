# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=4

unset GIT_ECLASS

if [[ ${PV} = 9999* ]]; then
	EGIT_REPO_URI="git://github.com/lodle/Desurium.git"
	GIT_ECLASS="git-2"
	SRC_URI=""
	EGIT_NOUNPACK="true"
else
	DESURA_ARC="Desura-${PV}.tar.bz2"
	SRC_URI="mirror://github.com/downloads/lodle/Desurium/${DESURA_ARC}"
fi
CEF_ARC="cef-291.tar.gz"
CEF_URI="http://github.com/downloads/lodle/Desurium/${CEF_ARC}"
CHROMIUM_ARC="chromium-15.0.876.0.tar.bz2"
CHROMIUM_URI="http://commondatastorage.googleapis.com/chromium-browser-official/${CHROMIUM_ARC}"
WX_ARC="wxWidgets-2.9.3.tar.bz2"
WX_URI="ftp://ftp.wxwidgets.org/pub/2.9.3/${WX_ARC}"
SRC_URI+="${CEF_URI} ${CHROMIUM_URI} ${WX_URI}"

inherit check-reqs cmake-utils eutils ${GIT_ECLASS} games gnome2-utils

CHECKREQS_DISK_BUILD="3G"

DESCRIPTION="Free software version of Desura game client"
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="GPL-3"
SLOT="0"
IUSE="+32bit debug +games-deps tools"

if [[ ${PV} != 9999* ]]; then
	KEYWORDS="~amd64 ~x86"
fi

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

		amd64? ( 32bit? (
			app-emulation/emul-linux-x86-gtklibs
			app-emulation/emul-linux-x86-gtkmmlibs
			app-emulation/emul-linux-x86-medialibs
			app-emulation/emul-linux-x86-opengl
			app-emulation/emul-linux-x86-sdl
			app-emulation/emul-linux-x86-soundlibs
			app-emulation/emul-linux-x86-xlibs[opengl]
		) )
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
	net-print/cups
	>=sys-devel/gcc-4.5
	sys-libs/zlib
	virtual/jpeg
	x11-libs/gtk+:2

	amd64? ( 32bit? (
		sys-devel/gcc[multilib]
	) )
"

RDEPEND="
	x11-misc/xdg-user-dirs
	x11-misc/xdg-utils
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

src_unpack() {
	if [[ ${PV} = 9999* ]]; then
		git-2_src_unpack
	else
		unpack ${DESURA_ARC}
	fi
}

src_configure() {
	# -DWITH_ARES=FALSE will use system curl, because we force curl[ares] to have ares support
	local mycmakeargs=(
		-DWITH_ARES=FALSE
		$(cmake-utils_use debug DEBUG)
		$(cmake-utils_use 32bit 32BIT_SUPPORT)
		$(cmake-utils_use tools BUILD_TOOLS)
		-DCMAKE_INSTALL_PREFIX="${GAMES_PREFIX}/${PN}"
		-DCEF_URL="file://${DISTDIR}/${CEF_ARC}"
		-DCHROMIUM_URL="file://${DISTDIR}/${CHROMIUM_ARC}"
		-DWXWIDGET_URL="file://${DISTDIR}/${WX_ARC}"
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install

	dosym "${GAMES_PREFIX}/${PN}/run.sh" "${GAMES_BINDIR}/${PN}.sh"

	doicon -s 256 "${FILESDIR}/${PN}.png"
	make_desktop_entry "${PN}.sh" "Desurium"

	prepgamesdirs
}

pkg_preinst() {
	games_pkg_preinst
	gnome2_icon_savelist
}

pkg_postinst() {
	games_pkg_postinst
	gnome2_icon_cache_update
}

pkg_postrm() {
	gnome2_icon_cache_update
}
