#!/bin/sh

cd $(dirname "$0")
set -e

random_hostname () {
    echo "${1}_$(LC_CTYPE=C tr -dc A-Za-z0-9 < /dev/urandom | head -c 16)"
}

run_test () {
    if [ "$2" = "$3" ]; then
        echo "Test for \`$1\` passed."
    else
        echo "Test for \`$1\` failed!"
        echo "Hostname \"$2\" (expected) != \"$3\" (actual)"
        exit 1
    fi
}

# macOS won't work for commands in /bin and /usr/bin becuase of System Integrity
# Protection, so let's copy uname and hostname to this dir
BIN_PREFIX=
IS_MACOS=
if [ "$(uname -s)" = "Darwin" ]; then
    cp $(which uname hostname) .
    BIN_PREFIX="./"
    IS_MACOS=1
fi

UNAME_EXPECTED="$(random_hostname uname)"
UNAME_ACTUAL="$(./fakehostname "$UNAME_EXPECTED" "${BIN_PREFIX}uname" -n)"
run_test uname "$UNAME_EXPECTED" "$UNAME_ACTUAL"

HOSTNAME_EXPECTED="$(random_hostname hostname)"
HOSTNAME_ACTUAL="$(./fakehostname "$HOSTNAME_EXPECTED" "${BIN_PREFIX}hostname")"
run_test hostname "$HOSTNAME_EXPECTED" "$HOSTNAME_ACTUAL"

# Clean up for macOS
if [ "$IS_MACOS" ]; then
    rm uname hostname
fi
