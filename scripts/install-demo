#!/bin/sh

die() {
	echo "$@" >&2
	exit 1
}

# Handle arguments

[ "$1" = "--help" ] || [ "$1" = "-h" ] || zipfile="$(readlink -m "$1")"

[ "$zipfile" != "" ] || die "\
Usage: install-demo path/to/arx_demo_english.zip [output_dir]

This script can be used to install the Arx Fatalis demo data.
Files will be verified and renamed as needed by Arx Libertatis.

For demo download locations see http://wiki.arx-libertatis.org/Getting_the_game_data#Demo"

if [ "$2" = "" ]
	then destdir="$(pwd)"
	else destdir="$(readlink -m "$2")"
fi

cd "$(dirname "$0")"
here="$(pwd)"

echo "Installing Arx Fatalis demo data from \"$zipfile\" to \"$destdir\".
"

# Check for required commands

cabextract=`which cabextract 2> /dev/null`
unzip=`which unzip 2> /dev/null`
bsdtar=`which bsdtar 2> /dev/null`
md5sum=`which md5sum 2> /dev/null`

[ -f "$cabextract" ] \
	|| die "Please install cabextract (http://www.cabextract.org.uk/)"

[ -f "$unzip" ] || [ -f "$bsdtar" ] \
	|| die "Please install unzip (http://www.info-zip.org/)" \
	       "or bsdtar from libarchive (http://libarchive.github.com/)"

[ -f "$zipfile" ] || die "Input file \"$zipfile\" not found."

extract_cab() {
	"$cabextract" "$1"
}

extract_zip() {
	if [ -f "$unzip" ]
		then "$unzip" "$1"
		else "$bsdtar" xvf "$1"
	fi
}

# Verify input file

if [ -f "$md5sum" ]
	then
		checksum="$(md5sum -b "$zipfile" | sed 's/ .*//')"
		expected='3c59a5799e1237b1b181c96e8c09155a'
		if [ "$checksum" = "$expected" ] 
			then echo "Checksum matched."
			else echo "Got checksum $checksum, expected $expected."
		fi
	else echo "Missing md5sum, cannot verify that you have the correct archive."
fi

# Prepare output and temp dirs

mkdir -p "$destdir" || exit 1

tempdir="$destdir/arx-install-demo-temp"

rm -rf "$tempdir" 2> /dev/null
mkdir "$tempdir" || exit 1
cd "$tempdir" || exit 1

# Extract files

extract_zip "$zipfile"

extract_cab Setup1.cab
extract_cab Setup2.cab
extract_cab Setup3.cab

# Install required files

doinstall() {
	mv -fv "$1" "$destdir/$2"
	chmod "--reference=$destdir" "$destdir/$2" > /dev/null 2>&1
	chmod -x "$destdir/$2" > /dev/null 2>&1
}

mkdir -pv "$destdir/misc"
doinstall "bin/Arx.ttf" "misc/arx.ttf"
doinstall "bin/Logo.bmp" "misc/logo.bmp"
doinstall "bin/data2.pak" "data2.pak"
doinstall "bin/LOC.pak" "loc.pak"
doinstall "data.pak" "data.pak"
doinstall "SFX.pak" "sfx.pak"
doinstall "SPEECH.pak" "speech.pak"

# Cleanup temporary files

rm -rf "$tempdir"

echo "
Done:"

# Verify installed files

[ -f "$md5sum" ] || ( echo "Could not verify checksum, md5sum not available." ; exit )

cd "$destdir"
checksums=`"$md5sum" -b data2.pak data.pak loc.pak misc/arx.ttf misc/logo.bmp sfx.pak speech.pak`

checksums_demo="\
958b78f8f370b06d769843137138c461 *data2.pak
5d7ba6e6c79ebf7fbb232eaced9e8ad9 *data.pak
2ae16d3925c597dca70f960f175def3a *loc.pak
9a95ff96795c034524ba1c2e94ea12c7 *misc/arx.ttf
aa3dfbd4bc9c863d10a0c5345ae5a4c9 *misc/logo.bmp
ea1b3e6d6f4906905d4a34f07e9a59ac *sfx.pak
62ca7b1751c0615ee131a94f0856b389 *speech.pak"

[ "$checksums" = "$checksums_demo" ] || die "
Checksum mismatch, expected

$checksums_demo

 got

$checksums"

echo "Checksum match." 
