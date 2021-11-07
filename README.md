# vminer

A VenonaCoin miner.

## Usage

First, you'll need to install `gcc` and use it to compile the `vminer` binary;
the included Makefile should help with that.

Then, edit the `vminer.conf` file. **Do not change the overall format**, only
edit

 - your Venonatude username on line 3;
 - the PREV_HASH on line 6;
 - the MESSAGE on line 7; and
 - the THRESHOLD on line 8.

Then,

```sh
$ ./bin/vminer path/to/vminer.conf
```

And wait!
