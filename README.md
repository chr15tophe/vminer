# vminer

A VenonaCoin miner.

## Usage

First, you'll need to install `gcc` and use it to compile the `vminer` binary;
the included Makefile should help with that.

Then, create a file called `vminer.conf` and paste 

```MAGIC_BYTES  =ISVENONACOINBEST
USERNAME     =hugh
PREVIOUS_HASH=4FD097A91E71DF403E6159FC58C53ECE1A809708549073541912BCBD10C924EA
MESSAGE      =IS GENESIS BLOCK VENONA. ARE WISHING FOR ALL GOOD LUCK.
THRESHOLD    =0000020000000000000000000000000000000000000000000000000000000000
```

Obviously, you will need to modify lines 2 through 5 to include the appropriate
information. **Do not modify anything else**, such as the spacing to the left
of the equals symbols, or the ordering of the lines.

Finally, `./bin/vminer vminer.conf` or `.\bin\vminer.exe vminer.conf` (on
Windows) and wait!
