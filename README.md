# `fakehostname` - A Command To Fake Your Hostname!

_Wow!_ Now you can run a command and fake your hostname!

## tl;dr

### Usage

Usage is relatively simple: `fakehostname <new-hostname> <cmd> [<args> ...]`

For example on Linux,

```
$ fakehostname joan hostname
joan

$ fakehostname rivers uname -n
rivers
```

Or you can use the library directly, ie with `libfakehostname.so` in the current
directory,

```
$ LD_PRELOAD=./libfakehostname.so FAKE_HOSTNAME=joan-rivers hostname
joan-rivers
```

Note you _can_ use `fakehostname` on macOS, but it's a little tricky and you
should read the [note below](#important-note-for-apples-macos-darwin).

### Installation

Fetch the source from Github and install!

```
$ git clone https://github.com/dtcooper/fakehostname.git
$ cd fakehostname
$ make
$ sudo make install
```

On Debian/Ubuntu you'll need [GCC](https://gcc.gnu.org/) which you can install
via,

```
$ sudo apt-get install -y build-essential
```

## Overview

The command (`fakehostname`), and its associated library (`libfakehostname`),
are a hack that slip between your program and the C standard library to monkey
patch the `uname` and `gethostname` functions provided therein. This is
accomplished via the `LD_PRELOAD` environment variables on Linux, and the
`DYLD_INSERT_LIBRARIES` + `DYLD_FORCE_FLAT_NAMESPACE` evironment variables on
macOS (see [important note below](#important-note-for-apples-macos-darwin)).
The library reads environment variable `FAKE_HOSTNAME` -- prepped by the command
-- and uses that instead of your system's hostname.

This software has been tested on macOS 10.11 (El Capitan) and Ubuntu 16.04 LTS
\+ 17.10.

## Important Note for Apple's macOS (Darwin)

On macOS, starting with 10.11 (El Capitan), a feature called [System Integrity
Protection](https://en.wikipedia.org/wiki/System_Integrity_Protection) was
introduced that disallows the dynamic insertion of libraries for system
executables. One therefore can't use the environment variable
`DYLD_INSERT_LIBRARIES` to actually use `libfakehostname`'s implementation of
the `uname` and `gethostname`.

You'll have to use executables that aren't in your system's path, ie `/bin`,
`/usr/bin`, et cetera. For example, to use the `hostname` command, you'll have
to copy it locally.

```
# Run on a Mac named "joan" and it doesn't work, since `hostname` is in /bin
$ fakehostname rivers hostname
joan

# So we copy the `hostname` command locally, and hurray, it works!
$ cp /bin/hostname .
$ fakehostname rivers ./hostname
rivers
```

Unfortunately, this renders this software relatively useless on macOS. Oh well!

## Running The Tests

There a couple simple tests for the command, namely running `uname` and
`hostname` and verifying it actually works.

```
make test
```

If all goes well, you should see a `SUCCESS: All N/N tests passed!` message!

## Uninstalling

Uninstall with one command,

```
sudo make uninstall
```

## Author and License

This project was created by [David Cooper](http://dtcooper.com/) and is licensed
under the MIT License. See the
[`LICENSE`](https://github.com/dtcooper/fakehostname/blob/master/LICENSE) file
for details.
