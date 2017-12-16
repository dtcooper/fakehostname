#!/bin/sh

cd $(dirname "$0")

FAILED=0
TOTAL_TESTS=0

random_hostname () {
    echo "${1}_$(LC_CTYPE=C tr -dc A-Za-z0-9 < /dev/urandom | head -c 16)"
}

run_test () {
    TOTAL_TESTS="$(expr $TOTAL_TESTS + 1)"
    if [ "$2" = "$3" ]; then
        echo " * \`$1\` test passed."
    else
        echo " * \`$1\` test failed: \"$2\" (expected) != \"$3\" (actual)"
        FAILED="$(expr $FAILED + 1)"
    fi
}

# macOS won't work for commands in /bin and /usr/bin becuase of System Integrity
# Protection, so let's copy uname and hostname to this dir
BIN_PREFIX=
if [ "$(uname -s)" = "Darwin" ]; then
    echo "[macOS detected, copying \`uname\` and \`hostname\` binaries to $PWD]"
    cp $(which uname hostname) .
    BIN_PREFIX="./"

    cleanup () {
        echo "[Removing \`uname\` and \`hostname\` binaries from $PWD]"
        rm uname hostname
    }

    trap cleanup EXIT
fi

echo "Running tests..."

UNAME_EXPECTED="$(random_hostname uname)"
UNAME_ACTUAL="$(./fakehostname "$UNAME_EXPECTED" "${BIN_PREFIX}uname" -n)"
run_test uname "$UNAME_EXPECTED" "$UNAME_ACTUAL"

HOSTNAME_EXPECTED="$(random_hostname hostname)"
HOSTNAME_ACTUAL="$(./fakehostname "$HOSTNAME_EXPECTED" "${BIN_PREFIX}hostname")"
run_test hostname "$HOSTNAME_EXPECTED" "$HOSTNAME_ACTUAL"

if [ "$FAILED" -gt 0 ]; then
    echo "... $FAILED/$TOTAL_TESTS tests FAILED!"
    exit 1
else
    echo "... all $TOTAL_TESTS/$TOTAL_TESTS tests passed."
fi
