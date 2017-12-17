# `fakehostname` - A Command To Fake Your Hostname!

_Wow!_ Now you can run a command and fake your hostname!

## tl;dr

### Usage

Usage is relatively simple: `fakehostname <new-hostname> <cmd> [<args> ...]`

For example,

```
$ fakehostname hi-mom hostname
himom

$ fakehostname hi-dad uname -n
hidad
```

### Installation

Fetch the source from Github and install!

```
$ git clone https://github.com/dtcooper/fakehostname.git
$ cd fakehostname
$ make
$ sudo make install
```

## Overview

The command (`fakehostname`), and its associated library (`libfakehostname`),
are a hack that slip between your program and the C standard library to monkey
patch the `uname` and `gethostname` routines provided therein. This is
accomplished via `LD_PRELOAD` environment variables on Linux, and the
`DYLD_INSERT_LIBRARIES` + `DYLD_FORCE_FLAT_NAMESPACE` ones on macOS (see
[note below](#important-note-for-apples-macos-darwin)).
The library reads environment variable `FAKE_HOSTNAME` -- prepped by the command
-- and uses that instead of your system's hostname.

## Important Note for Apple's macOS (Darwin)

On macOS, starting with 10.11 (El Capitan), a feature called [System Integrity
Protection](https://en.wikipedia.org/wiki/System_Integrity_Protection) was
introduced that disallows the use dynamic insertion of libraries for system
executables. One therefore can't use the environment variable
`DYLD_INSERT_LIBRARIES` to actually use `libfakehostname`'s implementation of
the `uname` and `gethostname`.

You'll have to use executables that aren't in your system's path, ie `/bin`,
`/usr/bin`, et cetera. For example, to use the `hostname` command, you'll have
to copy it locally.

```
# Run on a Mac named "byrne" and it doesn't work, since `hostname` is in /bin
$ fakehostname weymouth hostname
byrne

# So we copy the `hostname` command locally, and hurray, it works!
$ cp /bin/hostname .
$ fakehostname weymouth ./hostname
weymouth
```

Unforunately, this renders this software relatively useless on macOS. Oh well!

## Running The Tests

There a couple simple tests for the command, namely running `uname` and
`hostname` and verifying it actually works.

```
make test
```

If all goes well, you should see a `SUCCESS: All X/Y tests passed!` message!

## Uninstalling

Uninstall with one command!

```
sudo make uninstall
```
