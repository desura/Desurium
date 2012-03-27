# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

EGIT_REPO_URI="https://github.com/karolherbst/Desurium"

inherit eutils cmake-utils git-2

DESCRIPTION="Free software version of Desura game client"
HOMEPAGE="https://github.com/lodle/Desurium"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="builtin-curl builtin-tinyxml +builtin-breakpad +builtin-wxWidgets"

DEPEND="
	app-arch/bzip2
    dev-db/sqlite
	dev-libs/boost
	dev-libs/openssl
	!builtin-tinyxml? (
        || ( <dev-libs/tinyxml-2.6.2-r2[-stl]
             >=dev-libs/tinyxml-2.6.2-r2
        )
    )
	dev-lang/v8
	!builtin-breakpad? ( dev-util/google-breakpad )
    dev-vcs/subversion
	!builtin-curl? ( net-misc/curl[ares] )
	>=x11-libs/gtk+-2.24"
#	!builtin-wxWidgets? ( >=x11-libs/wxGTK-2.9.0 )
#	net-print/libgnomecups
#	dev-util/gyp
#	dev-util/depot_tools
#	check svn co http://google-breakpad.googlecode.com/svn/trunk -r 699  breakpad
#	check svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_0/ wxWidgets


RDEPEND="${DEPEND}"

S="${WORKDIR}/desura"

src_configure() {
	mycmakeargs="
        cmake-utils_use_with	builtin-curl      ARES
	"
	cmake-utils_src_configure
}

