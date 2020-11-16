#!/bin/bash
#
#	lookup a number via dasoertliche.de
#

#
#	strip every non-digits off
#
PHONENR="$( echo "$1" | tr --complement --delete "+0123456789" )"

#
#	we have to replace a leading +49 by a single 0
#
PHONENR="$( echo "$PHONENR" | sed -e "s/^+49/0/" )"

#
#	lookup via wget
#
URL="http://www.dasoertliche.de/Controller?form_name=search_inv&ph=$PHONENR"

wget --output-document=- "$URL" 2>/dev/null | \
	tr '\t' ' ' | \
	tr -s ' ' | \
	grep "var item =" --after-context=10 | \
	grep "\(ci\|na\):" | \
	sort -r | \
	sed -e "s/^ *\(ci\|na\): *\"//" | \
	sed -e "s/\".*$//" | \
	tr '\n' '#' | \
	sed -e "s/#$//" | \
	sed -s "s/#/, /"
