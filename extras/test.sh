#!/bin/sh

cd $(dirname "$0")/..

FAILED=0
NUM_TESTS=0
IS_LINUX=

random_hostname () {
    echo "${1}_$(LC_CTYPE=C tr -dc A-Za-z0-9 < /dev/urandom | head -c 16)"
}

run_test () {
    NUM_TESTS="$(expr $NUM_TESTS + 1)"
    if [ "$2" = "$3" ]; then
        echo "  $NUM_TESTS. \"$1\" test passed."
    else
        echo "  $NUM_TESTS. \"$1\" test failed: \"$2\" (expected) != \"$3\" (actual)"
        FAILED="$(expr $FAILED + 1)"
    fi
}

strip_newlines () {
    sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/|/g'
}

# macOS won't work for commands in /bin and /usr/bin becuase of System Integrity
# Protection, so let's copy uname and hostname to this dir
SYS_CMD_PREFIX=
if [ "$(uname -s)" = "Darwin" ]; then
    LIB_SUFFIX=dylib
    echo "[macOS detected, copying \`uname\` and \`hostname\` binaries to $PWD]"
    cp $(which uname hostname) .
    SYS_CMD_PREFIX='./'

    cleanup () {
        echo "[Removing \`uname\` and \`hostname\` binaries from $PWD]"
        rm uname hostname
    }

    trap cleanup EXIT
else
    IS_LINUX=1
    LIB_SUFFIX=so
fi

if [ ! -f "example" ]; then
    echo "[Command example not found. Running make.]"
    make all example
    echo
fi

echo "Running tests..."
echo

EXPECTED="$(random_hostname uname_cmd)"
ACTUAL="$(./fakehostname "$EXPECTED" "${SYS_CMD_PREFIX}uname" -n)"
run_test "uname command" "$EXPECTED" "$ACTUAL"

EXPECTED="$(random_hostname uname_c)"
ACTUAL="$(./fakehostname "$EXPECTED" ./example quiet uname)"
run_test "example: uname() function in C" "$EXPECTED" "$ACTUAL"

EXPECTED="$(random_hostname hostname_cmd)"
ACTUAL="$(./fakehostname "$EXPECTED" "${SYS_CMD_PREFIX}hostname")"
run_test "hostname command" "$EXPECTED" "$ACTUAL"

EXPECTED="$(random_hostname hostname_c)"
ACTUAL="$(./fakehostname "$EXPECTED" ./example quiet gethostname)"
run_test "example: gethostname() function in C" "$EXPECTED" "$ACTUAL"

EXPECTED="$(random_hostname libarg)"
cp "libfakehostname.$LIB_SUFFIX" testlib.bin
ACTUAL="$(./fakehostname --library ./testlib.bin "$EXPECTED" ./example quiet gethostname)"
rm testlib.bin
run_test "library argument (--library)" "$EXPECTED" "$ACTUAL"

EXPECTED="$(hostname)"
ACTUAL="$(./fakehostname "" ${SYS_CMD_PREFIX}uname -n)"
run_test "empty hostname, hostname" "$EXPECTED" "$ACTUAL"

EXPECTED="$(hostname)"
ACTUAL="$(./fakehostname "" ${SYS_CMD_PREFIX}hostname)"
run_test "empty hostname, uname" "$EXPECTED" "$ACTUAL"

EXPECTED="uname(): hi|gethostname(): hi|time(): 1234567890"
if [ "$IS_LINUX" ]; then
    ACTUAL="$(LD_PRELOAD="./time_preload.so" ./fakehostname hi ./example all | strip_newlines)"
else
    ACTUAL="$(DYLD_INSERT_LIBRARIES="./time_preload.dylib" DYLD_FORCE_FLAT_NAMESPACE=1 ./fakehostname hi ./example all | strip_newlines)"
fi
run_test "Preserve pre-existing preload (time)" "$EXPECTED" "$ACTUAL"

echo
if [ "$FAILED" -gt 0 ]; then
    echo "... FAILURE: $FAILED/$NUM_TESTS tests FAILED!"
    exit 1
else
    echo "... SUCCESS: All $NUM_TESTS/$NUM_TESTS tests passed!"
fi
