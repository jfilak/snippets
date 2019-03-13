#!/usr/bin/env bash

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

