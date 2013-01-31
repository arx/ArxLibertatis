#!/bin/sh

die() {
	echo "$@" >&2
	exit 1
}

# Handle arguments

if [ "$1" != "--help" ] && [ "$1" != "-h" ] ; then
	source="$(readlink -m "$1")"
fi

[ "$source" = "" ] && die "\
Usage: install-copy path/to/ArxFatalis/ [output_dir]

This script can be used to copy the game data from a fully patched
Arx Fatalis 1.21 install (for example a Steam download).
Files will be verified and renamed as needed by Arx Libertatis.

You can get the 1.21 patch from http://www.arkane-studios.com/uk/arx_downloads.php"

if [ "$2" = "" ]
	then destdir="$(pwd)"
	else destdir="$(readlink -m "$2")"
fi

cd "$(dirname "$0")"
here="$(pwd)"

echo "Installing Arx Fatalis data from \"$source\" to \"$destdir\".
"

[ -f "$here/install-verify" ] || die "Missing install-verify script."
install_verify_sourced=1
. "$here/install-verify"

# Check for required files

[ -d "$source" ] || die "$source does not exist"

# Prepare output and temp dirs

mkdir -p "$destdir" || exit 1

# Detect language

cd "$source" && detect_language

# Install required files

for f in "$@" ; do
	
	dst="$destdir/$f"
	
	dir="$(dirname "$f")"
	file="$(basename "$f" | sed 's/[^[:alnum:]_-]/\\&/g')"
	file_default="$(echo "$file" | sed 's/^\(.*\)\(\.[^.]*\)$/\1_default\2/g')"
	
	mkdir -pv "$destdir/$dir"
	rm "$dst" > /dev/null 2>&1
	
	[ -f "$dst" ] || find "$source" -iname "$file" -exec cp -fv {} "$dst" \;
	[ -f "$dst" ] || find "$source" -iname "$file_default" -exec cp -fv {} "$dst" \;
	
	[ -f "$dst" ] && chmod "--reference=$destdir" "$dst" > /dev/null 2>&1
	[ -f "$dst" ] && chmod -x "$dst" > /dev/null 2>&1
	
done

# Verify installed files

cd "$destdir" && verify_checksums
