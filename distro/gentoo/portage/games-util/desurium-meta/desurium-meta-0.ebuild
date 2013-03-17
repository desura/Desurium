# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

DESCRIPTION="Desurium game client and dependencies (meta)"
HOMEPAGE="http://desura.com/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE="+32bit +games-deps"

RDEPEND="
	games-deps? (
		dev-lang/mono[-minimal]
		gnome-base/libglade
		media-libs/libogg
		media-libs/libpng:1.2
		media-libs/libsdl[X,audio,joystick,opengl,video]
		media-libs/libtheora
		media-libs/libvorbis
		media-libs/openal
		media-libs/sdl-image[gif,jpeg,png,tiff]
		media-libs/sdl-ttf[X]
		virtual/ffmpeg[X,mp3,sdl,theora,truetype]
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
	games-util/desurium[32bit?]
	media-libs/desurium-cef"
