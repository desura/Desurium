# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

unset GIT_ECLASS
unset WX_ECLASS

GITHUB_MAINTAINER="lodle"
GITHUB_PROJECT="Desurium"

if [[ ${PV} = 9999* ]]; then
	EGIT_REPO_URI="git://github.com/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}.git"
	GIT_ECLASS="git-2"
	SRC_URI=""
	EGIT_NOUNPACK="true"
else
	DESURIUM_ARC="${P}.tar.gz"
	SRC_URI="http://github.com/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/tarball/${PV} -> ${DESURIUM_ARC}"
fi
BREAKPAD_ARC="breakpad-850.tar.gz"
BREAKPAD_URI="mirror://github/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/${BREAKPAD_ARC}"
CEF_ARC="cef-291.tar.gz"
CEF_URI="mirror://github/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/${CEF_ARC}"
WX_ARC="wxWidgets-2.9.3.tar.bz2"
WX_URI="ftp://ftp.wxwidgets.org/pub/2.9.3/${WX_ARC}"
SRC_URI="${SRC_URI}
	${BREAKPAD_URI}
	${CEF_URI}
	bundled-wxgtk? (
		${WX_URI}
	)"

inherit cmake-utils eutils ${GIT_ECLASS} gnome2-utils ${WX_ECLASS} games toolchain-funcs

DESCRIPTION="Free software version of Desura game client"
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="GPL-3"
SLOT="0"
IUSE="+32bit +bundled-wxgtk debug tools"

if ! use bundled-wxgtk ; then
	WX_GTK_VER="2.9"
	WX_ECLASS="wxwidgets"
fi

if [[ ${PV} != 9999* ]]; then
	KEYWORDS="~amd64 ~x86"
fi

# wxGTK-2.9.4.1 does not work!
COMMON_DEPEND="
	app-arch/bzip2
	dev-db/sqlite
	>=dev-libs/boost-1.47:=
	dev-libs/openssl:0

	|| ( <dev-libs/tinyxml-2.6.2-r2[-stl]
	    >=dev-libs/tinyxml-2.6.2-r2
	)

	<dev-lang/v8-3.16.5:=
	|| (
		net-misc/curl[adns]
		net-misc/curl[ares]
	)
	>=sys-devel/gcc-4.6
	virtual/pkgconfig
	x11-libs/gtk+:2
	x11-libs/libnotify
	!bundled-wxgtk? (
		=x11-libs/wxGTK-2.9.3.1[X]
	)

	amd64? ( 32bit? (
		sys-devel/gcc[multilib]
	) )
"

RDEPEND="
	media-libs/desurium-cef
	x11-misc/xdg-user-dirs
	x11-misc/xdg-utils
	${COMMON_DEPEND}
"

DEPEND="
	${COMMON_DEPEND}
"

pkg_pretend() {
	if [[ ${MERGE_TYPE} != binary ]]; then
		if [[ $(tc-getCC) =~ gcc ]]; then
			if [[ $(gcc-major-version) == 4 && $(gcc-minor-version) -lt 6 || $(gcc-major-version) -lt 4 ]] ; then
				eerror "You need at least sys-devel/gcc-4.6.0"
				die "You need at least sys-devel/gcc-4.6.0"
			fi
		fi
	fi
}

src_unpack() {
	if [[ ${PV} = 9999* ]]; then
		git-2_src_unpack
	else
		unpack ${DESURIUM_ARC}
		S="${WORKDIR}/$(ls ${WORKDIR})"
	fi
}

src_configure() {
	# -DWITH_ARES=FALSE will use system curl, because we force curl[ares] to have ares support
	local mycmakeargs=(
		-DWITH_ARES=FALSE
		-DFORCE_SYS_DEPS=TRUE
		-DBUILD_CEF=FALSE
		-BUILD_ONLY_CEF=FALSE
		$(cmake-utils_use debug DEBUG)
		$(cmake-utils_use 32bit 32BIT_SUPPORT)
		$(cmake-utils_use tools BUILD_TOOLS)
		-DCMAKE_INSTALL_PREFIX="${GAMES_PREFIX}"
		-DBREAKPAD_URL="file://${DISTDIR}/${BREAKPAD_ARC}"
		-DCEF_URL="file://${DISTDIR}/${CEF_ARC}"
		-DBINDIR="${GAMES_BINDIR}"
		-DDATADIR="${GAMES_DATADIR}"
		-DRUNTIME_LIBDIR="$(games_get_libdir)"
		-DDESKTOPDIR="/usr/share/applications"
		$( if use bundled-wxgtk ; then
			echo -DWXWIDGET_URL="file://${DISTDIR}/${WX_ARC}"
		fi )
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install

	newicon -s scalable "${S}/src/branding_${PN}/sources/desubot.svg" "${PN}.svg"
	make_desktop_entry "${GAMES_BINDIR}/desura" "Desurium" "${PN}"

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
