#!/bin/sh

# Verify arx fatalis non-demo checksums.
# This script is meant to be run on it's own or sourced by the other
# install scripts.

# List of required files

if [ "$install_verify_sourced" != "1" ] ; then
	die() {
		echo "$@" >&2
		exit 1
	}
	( [ "$1" = "--help" ] || [ "$1" = "-h" ] ) && die "\
Usage: install-verify [directory]"
	if [ "$1" = "" ]
		then checkdir="$(pwd)"
		else checkdir="$(readlink -m "$1")"
	fi
	[ -d "$checkdir" ] || die "directory $checkdir does not exist"
fi

set -- \
	"data2.pak" \
	"graph/interface/misc/arkane.bmp" \
	"graph/interface/misc/quit1.bmp" \
	"graph/obj3d/textures/fixinter_barrel.jpg" \
	"graph/obj3d/textures/fixinter_bell.bmp" \
	"graph/obj3d/textures/fixinter_metal_door.jpg" \
	"graph/obj3d/textures/fixinter_public_notice.bmp" \
	"graph/obj3d/textures/item_bread.bmp" \
	"graph/obj3d/textures/item_club.jpg" \
	"graph/obj3d/textures/item_long_sword.jpg" \
	"graph/obj3d/textures/item_mauld_sabre.jpg" \
	"graph/obj3d/textures/item_mauldsword.jpg" \
	"graph/obj3d/textures/item_mirror.jpg" \
	"graph/obj3d/textures/item_ring_casting.bmp" \
	"graph/obj3d/textures/item_rope.bmp" \
	"graph/obj3d/textures/item_spell_sheet.jpg" \
	"graph/obj3d/textures/item_torch2.jpg" \
	"graph/obj3d/textures/item_torch.jpg" \
	"graph/obj3d/textures/item_zohark.bmp" \
	"graph/obj3d/textures/l7_dwarf_[wood]_board08.jpg" \
	"graph/obj3d/textures/l7_dwarf_[wood]_board80.jpg" \
	"graph/obj3d/textures/npc_dog.bmp" \
	"graph/obj3d/textures/npc_pig.bmp" \
	"graph/obj3d/textures/npc_pig_dirty.bmp" \
	"graph/obj3d/textures/npc_rat_base.bmp" \
	"graph/obj3d/textures/npc_rat_base_cm.bmp" \
	"graph/obj3d/textures/npc_worm_body_part1.jpg" \
	"graph/obj3d/textures/npc_worm_body_part2.bmp" \
	"graph/obj3d/textures/[wood]_light_door.jpg" \
	"manual.pdf" \
	"map.pdf" \
	"misc/arx_default.ttf" \
	"misc/arx_russian.ttf" \
	"misc/arx_taiwanese.ttf" \
	"misc/logo.avi" \
	"misc/logo.bmp" \
	"sfx.pak" \
	"data.pak" \
	"loc.pak" \
	"speech.pak"

# Common functions

md5sum=`which md5sum 2> /dev/null`
[ -f "$md5sum" ] \
	|| die "Please install md5sum (http://www.gnu.org/software/coreutils/)"

data_lang='english'
detect_language() {
	
	speech_checksum=`find '.' -iname "speech.pak" -exec "$md5sum" -b {} \; \
	                 | sed "s/ .*//g"`
	if [ "$speech_checksum" = '' ] ; then
		speech_checksum=`find '.' -iname "speech_default.pak" \
		                           -exec "$md5sum" -b {} \; | sed "s/ .*//g"`
	fi
	
	# check if the checksum is of a known localisation and set data_lang to
	# the language string to be used with the 1.21 patch installer
	case "$speech_checksum" in
		
		'4c3fdb1f702700255924afde49081b6e') data_lang='german'
			loc_checksum='31bc35bca48e430e108db1b8bcc2621d' ;;
		
		# Bundled version of AF included with NVIDIA card
		'ab8a93161688d793a7c78fbefd7d133e') data_lang='german'
			loc_checksum='31bc35bca48e430e108db1b8bcc2621d' ;;
		
		'4e8f962d8204bcfd79ce6f3226d6d6de') data_lang='english'
			loc_checksum='a47b192493afb5794e2161a62d35b69f' ;;
		
		'2f88c67ae1537919e69386d27583125b') data_lang='spanish'
			loc_checksum='121f99608814a2c9c5857cfadb665553' ;;
		
		'4edf9f8c799190590b4cd52cfa5f91b1') data_lang='french'
			loc_checksum='f8fc448fea12469ed94f417c313fe5ea' ;;
		
		'81f05dea47c52d43f01c9b44dd8fe962') data_lang='italian'
			loc_checksum='a9e162f2916f5737a95bd8c5bd8a979e' ;;
		
		'677163bc319cd1e9aa1b53b5fb3e9402') data_lang='russian'
			loc_checksum='a131bf2398ee70a9c22a2bbffd9d0d99' ;;
		
		'') die "speech*.pak not found in $(pwd)" ;;
		*) die "unsupported data language - speech*.pak checksum:" \
		        "$speech_checksum" ;;
	esac
	
	echo "
Data language: $data_lang
"
}

record_checksum_failure() {
	if [ $checksum_failed = 0 ] ; then
		echo "
Checksum failed:"
		checksum_failed=1
	fi
}

verify_checksum() {
	
	file="$1" ; shift
	
	if [ ! -f "$file" ] ; then
		record_checksum_failure
		echo "- missing $file"
		return
	fi
	
	checksum=`"$md5sum" -b "$file" | sed "s/ .*//g"`
	
	checksum_matched=0
	for valid_checksum in "$@" ; do
		[ "$checksum" = "$valid_checksum" ] && checksum_matched=1
	done
	
	if [ $checksum_matched = 0 ] ; then
		
		record_checksum_failure
		
		errorstr="- $file: got '$checksum', expected '$1'" ; shift
		for alternate_checksum in "$@" ; do
			errorstr="$errorstr or '$alternate_checksum'"
		done
		echo "$errorstr"
	fi
}

verify_checksums() {
	
	echo "
Done:"
	
	checksum_failed=0
	
	# Common files
	verify_checksum 'data2.pak' \
	                'f7e0ce700bf963429ac535ca86f8a7b4'
	verify_checksum 'graph/interface/misc/arkane.bmp' \
	                'afff1099c01ffeb03b9a351f7b5966b6'
	verify_checksum 'graph/interface/misc/quit1.bmp' \
	                '41445d3792a1f8818d950aca47254488'
	verify_checksum 'graph/obj3d/textures/fixinter_barrel.jpg' \
	                '8419274acbff7346c3661b18d6aad6dc'
	verify_checksum 'graph/obj3d/textures/fixinter_bell.bmp' \
	                '5743b9047c9ad65540c318dfcc98123a'
	verify_checksum 'graph/obj3d/textures/fixinter_metal_door.jpg' \
	                'f246eff6b19c9c710313b4a4dce96a69'
	verify_checksum 'graph/obj3d/textures/fixinter_public_notice.bmp' \
	                'f81394abbb9006ce0950843b7909db33'
	verify_checksum 'graph/obj3d/textures/item_bread.bmp' \
	                '544448f8eedc912aa231a6a04fffb7c5'
	verify_checksum 'graph/obj3d/textures/item_club.jpg' \
	                '7e26c4199ddaca494c8b369294306b0b'
	verify_checksum 'graph/obj3d/textures/item_long_sword.jpg' \
	                '3a6196fe9b7666c7d80d82be06f6de86'
	verify_checksum 'graph/obj3d/textures/item_mauld_sabre.jpg' \
	                '18492c25ebac02f83e2f0ebda61ecb00'
	verify_checksum 'graph/obj3d/textures/item_mauldsword.jpg' \
	                '503a5c2f23668040c675aefdde6dbbe5'
	verify_checksum 'graph/obj3d/textures/item_mirror.jpg' \
	                'c0a22b4f7a7a6461da68206e94928637'
	verify_checksum 'graph/obj3d/textures/item_ring_casting.bmp' \
	                '348f9add709bacee08556d1f8cf10f3f'
	verify_checksum 'graph/obj3d/textures/item_rope.bmp' \
	                'ff05de281c8b380ee98f6e123d3d51cb'
	verify_checksum 'graph/obj3d/textures/item_spell_sheet.jpg' \
	                '024ccbb520020f92fba5a5a4f0270cea'
	verify_checksum 'graph/obj3d/textures/item_torch2.jpg' \
	                '027951899b4829599ca611010ea3484f'
	verify_checksum 'graph/obj3d/textures/item_torch.jpg' \
	                '9ada166f23ddcb775ac20836e752187e'
	verify_checksum 'graph/obj3d/textures/item_zohark.bmp' \
	                'cd206a4027f86c6e57b7710c94049efa'
	verify_checksum 'graph/obj3d/textures/l7_dwarf_[wood]_board08.jpg' \
	                '79ccc81adb7c37b98f40b478ef1fccd4'
	verify_checksum 'graph/obj3d/textures/l7_dwarf_[wood]_board80.jpg' \
	                '691611087b13d38ef02bb9dfd6a2518e'
	verify_checksum 'graph/obj3d/textures/npc_dog.bmp' \
	                '116bd374c14ae8c387a4da1899e1dca7'
	verify_checksum 'graph/obj3d/textures/npc_pig.bmp' \
	                'b7a4d0d3d230b2d1470176909004e38b'
	verify_checksum 'graph/obj3d/textures/npc_pig_dirty.bmp' \
	                '76034d8d74056c8a982479d36321c228'
	verify_checksum 'graph/obj3d/textures/npc_rat_base.bmp' \
	                '00c585ec9ebe8006d7ca72993de7b51b'
	verify_checksum 'graph/obj3d/textures/npc_rat_base_cm.bmp' \
	                'cae38facbf77db742180b9e58d0eb42f'
	verify_checksum 'graph/obj3d/textures/npc_worm_body_part1.jpg' \
	                '0b220bffaedc89fa663f08d12630c342'
	verify_checksum 'graph/obj3d/textures/npc_worm_body_part2.bmp' \
	                '20797cb78f6393a0fb5405969ba9f805'
	verify_checksum 'graph/obj3d/textures/[wood]_light_door.jpg' \
	                '00d0b018e995e7d013d6e52e92126901'
	verify_checksum 'misc/arx_default.ttf' \
	                '9a95ff96795c034524ba1c2e94ea12c7'
	verify_checksum 'misc/arx_russian.ttf' \
	                '921561e83786efcd25f92147b60a13db'
	verify_checksum 'misc/arx_taiwanese.ttf' \
	                'da59198061cef0761c6b2fca113f76f6'
	verify_checksum 'misc/logo.avi' \
	                '63ed31a4eb3d226c23e58cfaa974d484'
	verify_checksum 'misc/logo.bmp' \
	                'afff1099c01ffeb03b9a351f7b5966b6'
	verify_checksum 'sfx.pak' \
	                '2efc9a74c517fd1ee9919900cf4091d2'
	
	# data.pak is censored in some versions (presumably has less gore)
	# At least the original german and italian CDs have the censored version.
	# The censored version has different level files and a different
	# human_female_villager model.
	# There are also minor differences in the scripts, but those are
	# overwritten by data2.pak from the 1.21 patch.
	data_checksum_original='a91a0b39a046233debbb10b4850e13eb'
	data_checksum_censored='a88d239dc7919ab113ff45483cb4ad46'
	verify_checksum 'data.pak' "$data_checksum_original" "$data_checksum_censored"
	
	# Language-specific files
	verify_checksum 'loc.pak' "$loc_checksum"
	# There is no need to check speech.pak here as we already used it to
	# detect $loc_checksum
	
	[ $checksum_failed = 0 ] || die "
ERROR: Checksum mismatch."
	echo "Checksum match."
}

if [ "$install_verify_sourced" != "1" ] ; then
	cd "$checkdir"
	detect_language
	verify_checksums
fi
