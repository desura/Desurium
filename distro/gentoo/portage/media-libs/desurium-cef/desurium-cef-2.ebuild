# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

PYTHON_COMPAT=( python2_6 python2_7 )

unset GIT_ECLASS

GITHUB_MAINTAINER="lodle"
GITHUB_PROJECT="Desurium"
DESURIUM_VERSION="0.8.0_rc5"

# tools versions
CEF_ARC="cef-291.tar.gz"
CHROMIUM_ARC="chromium-15.0.876.0.tar.bz2"
DEPOT_TOOLS_ARC="depot_tools-145556-2.tar.gz"

if [[ ${PV} = 9999* ]]; then
	EGIT_REPO_URI="git://github.com/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}.git"
	GIT_ECLASS="git-2"
	SRC_URI=""
	EGIT_NOUNPACK="true"
else
	DESURIUM_ARC="desurium-${DESURIUM_VERSION}.tar.gz"
	SRC_URI="http://github.com/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/tarball/${DESURIUM_VERSION} -> ${DESURIUM_ARC}"
fi
SRC_URI="${SRC_URI}
	mirror://github/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/${CEF_ARC}
	http://commondatastorage.googleapis.com/chromium-browser-official/${CHROMIUM_ARC}
	mirror://github/${GITHUB_MAINTAINER}/${GITHUB_PROJECT}/${DEPOT_TOOLS_ARC}"

inherit check-reqs cmake-utils eutils ${GIT_ECLASS} python-any-r1 games

CHECKREQS_DISK_BUILD="3G"

DESCRIPTION="highly patched CEF by desurium."
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="BSD"
SLOT="0"
IUSE=""

if [[ ${PV} != 9999* ]]; then
	KEYWORDS="~amd64 ~x86"
fi

# wxGTK-2.9.4.1 does not work!
COMMON_DEPEND="
	app-arch/bzip2
	dev-libs/dbus-glib
	dev-libs/libevent
	dev-libs/libxml2
	dev-libs/nss
	media-libs/alsa-lib
	media-libs/flac
	media-libs/libpng:0
	media-libs/libwebp
	media-libs/speex
	sys-apps/dbus
	sys-libs/zlib
	virtual/jpeg"
RDEPEND="${COMMON_DEPEND}"
DEPEND="
	dev-lang/yasm
	dev-util/gperf
	${COMMON_DEPEND}
	${PYTHON_DEPS}"

pkg_setup() {
	python-any-r1_pkg_setup
	games_pkg_setup
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
		-DFORCE_SYS_DEPS=TRUE
		-DCMAKE_INSTALL_PREFIX="${GAMES_PREFIX}"
		-DCEF_URL="file://${DISTDIR}/${CEF_ARC}"
		-DCHROMIUM_URL="file://${DISTDIR}/${CHROMIUM_ARC}"
		-DDEPOT_TOOLS_URL="file://${DISTDIR}/${DEPOT_TOOLS_ARC}"
		-DBUILD_ONLY_CEF=TRUE
		-DRUNTIME_LIBDIR="$(games_get_libdir)"
	)
	cmake-utils_src_configure
}

src_compile() {
	cmake-utils_src_compile
}

src_install() {
	cmake-utils_src_install
	prepgamesdirs
}
