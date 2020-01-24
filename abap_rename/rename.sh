#!/usr/bin/env bash
# Author:  Jakub Filak <jakub@thefilaks.net>
#
# Purpose: Rename ABAP type serialized by abapGit
#
# Usage:   ./rename_abap OLD_TYPE_NAME NEW_TYPE_NAME
#
# The script will find all source files with the prefix
# OLD_TYPE_NAME. and rename the files to NEW_TYPE_MANE.
#
# Then the script tries to find all occurrences of
# the string OLD_TYPE_NAME in files in the current
# working directory and its subdirectories and replaces
# these occurrence with NEW_TYPE_NAME.
#
# The script tries to keep the casing, so it first
# try to search for files and string in all uppercase,
# then in lower case and finally in the given form.

set -o pipefail
set -o errexit
set -o nounset

OLD_NAME=$1
NEW_NAME=$2

MV="git mv"

function upper
{
    echo $1 | tr [[:lower:]] [[:upper:]]
}

function lower
{
    echo $1 | tr [[:upper:]] [[:lower:]]
}

function rename_sources
{
    echo "Checking sources: $1"
    for src in $(find -name "$1.*")
    do
        dest=$(echo $src | sed 's/'$1'/'$2'/')
        echo "Moving: $src -> $dest"
        $MV $src $dest
    done
}

function rename_in_code
{
    echo "Renaming ABAP tokens: $1"
    for src in $(grep -rilw $1)
    do
        sed -i 's/\b'$1'\b/'$2'/g' $src
    done
}

rename_sources $(upper $OLD_NAME) $(upper $NEW_NAME)
rename_sources $(lower $OLD_NAME) $(lower $NEW_NAME)
rename_sources $OLD_NAME $NEW_NAME

rename_in_code $(upper $OLD_NAME) $(upper $NEW_NAME)
rename_in_code $(lower $OLD_NAME) $(lower $NEW_NAME)
rename_in_code $OLD_NAME $NEW_NAME

