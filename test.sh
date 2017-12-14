#!/bin/sh

cd $(dirname "$0")
set -e

random_hostname () {
    echo "${1}_$(LC_CTYPE=C tr -dc A-Za-z0-9 < /dev/urandom | head -c 16)"
}

run_test () {
    if [ "$2" = "$3" ]; then
        echo "Test for \`$1\` passed."
        return 0
    else
        echo "Test for \`$1\` failed: \"$2\" (expected) != \"$3\" (actual)"
        return 1
    fi
}

# macOS won't work for commands in /bin and /usr/bin becuase of System Integrity
# Protection, so let's copy uname and hostname to this dir
BIN_PREFIX=
if [ "$(uname -s)" = "Darwin" ]; then
    echo '[Darwin detected, copying `uname` and `hostname` binaries]'
    cp $(which uname hostname) .
    BIN_PREFIX="./"

    cleanup () {
        echo '[Removing `uname` and `hostname` binaries]'
        rm uname hostname
    }

    trap cleanup EXIT
fi

UNAME_EXPECTED="$(random_hostname uname)"
UNAME_ACTUAL="$(./fakehostname "$UNAME_EXPECTED" "${BIN_PREFIX}uname" -n)"
run_test uname "$UNAME_EXPECTED" "$UNAME_ACTUAL"

HOSTNAME_EXPECTED="$(random_hostname hostname)"
HOSTNAME_ACTUAL="$(./fakehostname "$HOSTNAME_EXPECTED" "${BIN_PREFIX}hostname")"
run_test hostname "$HOSTNAME_EXPECTED" "$HOSTNAME_ACTUAL"
