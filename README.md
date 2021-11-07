# vminer

A VenonaCoin miner.

## Usage

### Windows

You can download `vminer1-1-1_64.exe` (for 64-bit Windows machines) from the
Discord, or from [Releases](https://github.com/bcgeocaching/vminer/releases).

You'll need to create a file called `vminer.conf` and paste

```
PREVIOUS_BLOCK_HASH = "4FD097A91E71DF403E6159FC58C53ECE1A809708549073541912BCBD10C924EA"
MESSAGE = "IS GENESIS BLOCK VENONA. ARE WISHING FOR ALL GOOD LUCK."
THRESHOLD = "0000020000000000000000000000000000000000000000000000000000000000"
```

Obviously, you will need to modify those variables to the appropriate values.

Finally, execute `.\vminer1-1-1_64.exe "your username" path/to/vminer.conf`.
**Don't forget to sanity-check the values that are logged at the start**; if
something isn't right then something has gone wrong!

### Mac/Linux

Set the `USERNAME` and `REMEMBER_TOKEN` variables in `run.sh` as appropriate
(`REMEMBER_TOKEN` should be set to the value of the `remember_token` cookie
on the website.)

Then, execute `./run.sh`. It will automatically `make`, get the values
associated with the current block, and start mining. **Don't forget to
sanity-check the values that are logged at the start!**

*Note*: The `REMEMBER_TOKEN` does not have to "agree" with the `USERNAME`; you
can set someone else's username and your `REMEMBER_TOKEN` to mine VenonaCoin as
if you were them. The `REMEMBER_TOKEN` is only used to scrape the current
block hash, message, and threshold from the website.

