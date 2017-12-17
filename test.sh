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
        echo " * \"$1\" test passed."
    else
        echo " * \"$1\" test failed: \"$2\" (expected) != \"$3\" (actual)"
        FAILED="$(expr $FAILED + 1)"
    fi
}

# macOS won't work for commands in /bin and /usr/bin becuase of System Integrity
# Protection, so let's copy uname and hostname to this dir
BIN_PREFIX=
if [ "$(uname -s)" = "Darwin" ]; then
    LIB_SUFFIX=dylib
    echo "[macOS detected, copying \`uname\` and \`hostname\` binaries to $PWD]"
    cp $(which uname hostname) .
    BIN_PREFIX='./'

    cleanup () {
        echo "[Removing \`uname\` and \`hostname\` binaries from $PWD]"
        rm uname hostname
    }

    trap cleanup EXIT
else
    LIB_SUFFIX=so
fi

echo "Running tests..."

UNAME_CMD_EXPECTED="$(random_hostname uname_cmd)"
UNAME_CMD_ACTUAL="$(./fakehostname "$UNAME_EXPECTED" "${BIN_PREFIX}uname" -n)"
run_test "uname command" "$UNAME_EXPECTED" "$UNAME_ACTUAL"

UNAME_C_EXPECTED="$(random_hostname uname_c)"
UNAME_C_ACTUAL="$(./fakehostname "$UNAME_C_EXPECTED" ./test_cmd uname)"
run_test "uname() function in C" "$UNAME_C_EXPECTED" "$UNAME_C_ACTUAL"

HOSTNAME_CMD_EXPECTED="$(random_hostname hostname_cmd)"
HOSTNAME_CMD_ACTUAL="$(./fakehostname "$HOSTNAME_EXPECTED" "${BIN_PREFIX}hostname")"
run_test "hostname command" "$HOSTNAME_EXPECTED" "$HOSTNAME_ACTUAL"

GETHOSTNAME_C_EXPECTED="$(random_hostname hostname_c)"
GETHOSTNAME_C_ACTUAL="$(./fakehostname "$GETHOSTNAME_C_EXPECTED" ./test_cmd gethostname)"
run_test "gethostname() function in C" "$GETHOSTNAME_C_EXPECTED" "$GETHOSTNAME_C_ACTUAL"

LIBARG_EXPECTED="$(random_hostname libarg)"
LIBARG_ACTUAL="$(./fakehostname --library "./libfakehostname.$LIB_SUFFIX" "$LIBARG_EXPECTED" "${BIN_PREFIX}hostname")"
run_test "library argument (--library)" "$LIBARG_EXPECTED" "$LIBARG_ACTUAL"

if [ "$FAILED" -gt 0 ]; then
    echo "... FAILURE: $FAILED/$TOTAL_TESTS tests FAILED!"
    exit 1
else
    echo "... SUCCESS: All $TOTAL_TESTS/$TOTAL_TESTS tests passed!"
fi
