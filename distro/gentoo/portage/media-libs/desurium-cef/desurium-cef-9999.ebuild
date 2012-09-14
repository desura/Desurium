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
DEPOT_TOOLS_ARC="depot_tools-145556-2.tar.gz"
DEPOT_TOOLS_URI="https://github.com/downloads/lodle/Desurium/${DEPOT_TOOLS_ARC}"
SRC_URI+="${CEF_URI} ${CHROMIUM_URI} ${DEPOT_TOOLS_URI}"

inherit check-reqs cmake-utils eutils games ${GIT_ECLASS}

CHECKREQS_DISK_BUILD="3G"

DESCRIPTION="highly patched CEF by desurium."
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="BSD"
SLOT="0"
IUSE=""

if [[ ${PV} != 9999* ]]; then
	KEYWORDS="~amd64 ~x86"
fi

# some deps needed by some games
COMMON_DEPEND="
	app-arch/bzip2
	dev-libs/libevent
	dev-libs/libxml2
	dev-libs/openssl:0
	gnome-base/libgnome-keyring
	media-libs/flac
	media-libs/libpng:0
	media-libs/libwebp
	media-libs/speex
	net-print/cups
	sys-libs/zlib
	virtual/jpeg
	=x11-libs/wxGTK-2.9.3.1
"

RDEPEND="
	${COMMON_DEPEND}
"

DEPEND="
	dev-lang/yasm
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
		-DFORCE_SYS_DEPS=TRUE
		-DCMAKE_INSTALL_PREFIX="${GAMES_PREFIX}"
		-DCEF_URL="file://${DISTDIR}/${CEF_ARC}"
		-DCHROMIUM_URL="file://${DISTDIR}/${CHROMIUM_ARC}"
		-DDEPOT_TOOLS_URL="file://${DISTDIR}/${DEPOT_TOOLS_ARC}"
		-DBUILD_ONLY_CEF=TRUE
		-DRUNTIME_LIBDIR="${GAMES_PREFIX}/lib"
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install
}

