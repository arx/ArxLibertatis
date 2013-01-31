#!/bin/sh

die() {
	echo "$@" >&2
	exit 1
}

# Handle arguments

innoextract_opts=''
if [ "$1" = "--no-progress" ] ; then
	innoextract_opts='--progress=off'
	shift
fi

[ "$1" = "--help" ] || [ "$1" = "-h" ] || setupfile="$(readlink -m "$1")"

[ "$setupfile" != "" ] || die "\
Usage: install-gog path/to/setup_arx_fatalis.exe [output_dir]

Optional option (must be the first argument):
 --no-progress  Disable the innoextract progress bar

This script can be used to install the Arx Fatalis data from a GOG.com setup file.
Files will be verified and renamed as needed by Arx Libertatis.

setup_arx_fatalis.exe can be downloaded from your GOG.com account after buying Arx Fatalis"

if [ "$2" = "" ]
	then destdir="$(pwd)"
	else destdir="$(readlink -m "$2")"
fi

cd "$(dirname "$0")"
here="$(pwd)"

echo "Installing Arx Fatalis GOG.com data from \"$setupfile\" to \"$destdir\".
"

[ -f "$here/install-verify" ] || die "Missing install-verify script."
install_verify_sourced=1
. "$here/install-verify"

# Check for required commands

innoextract=`which innoextract 2> /dev/null`

[ -f "$innoextract" ] \
	|| die "Please install innoextract (http://constexpr.org/innoextract/)"

# Verify input file

checksum="$("$md5sum" -b "$setupfile" | sed 's/ .*//')"
expected='0dd8ec13c10146db1a741258b624040a'
if [ "$checksum" = "$expected" ] 
	then echo "Checksum matched."
	else echo "Got checksum $checksum, expected $expected."
fi

# Prepare output and temp dirs

mkdir -p "$destdir" || exit 1

tempdir="$destdir/arx-install-gog-temp"

rm -rf "$tempdir" 2> /dev/null
mkdir "$tempdir" || exit 1
cd "$tempdir" || exit 1

# Extract files

"$innoextract" $innoextract_opts --lowercase "$setupfile"

# Install required files

for f in "$@" ; do
	
	dir="$(dirname "$f")"
	mkdir -pv "$destdir/$dir"
	
	mv -fv "app/$f" "$destdir/$f"
	
	chmod "--reference=$destdir" "$destdir/$f" > /dev/null 2>&1
	chmod -x "$destdir/$f" > /dev/null 2>&1
	
done

# Cleanup temporary files

rm -rf "$tempdir"

# Verify installed files

cd "$destdir"
detect_language
verify_checksums
