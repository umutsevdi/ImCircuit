#!/bin/bash
# ------------------------------------------------------------------------------
#
# File: i18n/build.sh
# Created: 08/22/25
# Author: Umut Sevdi
# Description: Generates translation files.
#
# Project: logic-circuit-simulator-2
# License: 
# GNU GENERAL PUBLIC LICENSE
# Copyright (C) 2025 Umut Sevdi
# ------------------------------------------------------------------------------
APPNAME_BIN="ImCircuit"
AUTHOR="Umut Sevdi"
AUTHOR_MAIL="ask@umutsevdi.com"
LOCALES=(
    "de_DE" "en_US" "es_ES" "fr_FR"
    "ja_JP" "ru_RU" "tr_TR" "zh_CN"
)

NAMES=("Deutsch" "English" "Español" "Français"
       "日本語"  "Русский" "Türkçe"  "中国人"
)

xgettext \
    --copyright-holder="$AUTHOR"\
    --language=C++ \
    --keyword=_ \
    --keyword=WINDOWNAME\
    --from-code=UTF-8 \
    --msgid-bugs-address="${AUTHOR_MAIL}" \
    --add-comments="TRANSLATORS:$AUTHOR" \
    --package-name=$APPNAME_BIN \
    --package-version="0.1.0" \
    --output=i18n/${APPNAME_BIN}.pot \
    $(find src/ -name '*.cpp' -o -name '*.h')


for loc in "${LOCALES[@]}"; do
    loc+=".po"
    po_path="i18n/${loc}"
    if [[ -f "$po_path" ]]; then
        echo "Merging updates into $po_path"
        msgmerge --quiet --update \
                 --no-fuzzy-matching \
                 "$po_path" i18n/${APPNAME_BIN}.pot
    else
        echo "Creating new $po_path"
        msginit --input=i18n/${APPNAME_BIN}.pot \
                --no-translator \
                --locale="${loc}" \
                --output="$po_path"
    fi
done
rm i18n/po.h
echo -e "/** Generated at `date`\r\n" \
        " * Copyright (C) 2025 Umut Sevdi\r\n" \
        " * This file is distributed under the same license as\r\n" \
        " * the ${APPNAME_BIN} package.\r\n" \
        " */\r\n" \
        "#ifndef __LCS_POGEN__\r\n" \
        "#define __LCS_POGEN__\r\n" \
        "#define __LOCALE_S ${#LOCALES[@]}\r\n" \
        "static const char* __LOCALES__[] = {" >> i18n/po.h
for loc in "${LOCALES[@]}"; do
    echo "\"$loc\"," >> i18n/po.h
done
echo "};" >> i18n/po.h
echo "static const char* __NAMES__[] = {" >> i18n/po.h
for loc in "${NAMES[@]}"; do
    echo "\"$loc\"," >> i18n/po.h
done
echo "};" >> i18n/po.h
echo "#endif // __LCS_POGEN__" >> i18n/po.h

rm -rf build/locale
for i in i18n/*.po; do
    base=`basename -s .po $i`
    target="build/locale/$base/LC_MESSAGES/"
    mkdir -p $target
    msgfmt -o ${target}/${APPNAME_BIN}.mo $i

done
