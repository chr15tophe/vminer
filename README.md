# vminer

A VenonaCoin miner.

## Usage

### Windows

You can download the latest version of the miner (for 64-bit Windows machines)
from the [Releases](https://github.com/bcgeocaching/vminer/releases) section.

Finally, execute `.\vminerX-X-X_64.exe "previous hash" "your username" "threshold"
**Don't forget to sanity-check the values that are logged at the start**; if
something doesn't look right then something has gone wrong!

### Mac/Linux

If you're on Mac or Linux, you'll have to build the miner from source.

Set the `USERNAME` and `REMEMBER_TOKEN` variables in `run.sh` as appropriate
(`REMEMBER_TOKEN` should be set to the value of the `remember_token` cookie
on the website.) Then, execute `./run.sh`. It will automatically compile, get
the values associated with the current block, and start mining. Like with
Windows, **don't forget to sanity-check the values that are logged at the
start!**

*Note*: The `REMEMBER_TOKEN` does not have to "agree" with the `USERNAME`; you
can set *someone else's* username and *your* `REMEMBER_TOKEN` to mine
VenonaCoin as if you were them. The `REMEMBER_TOKEN` is only used to scrape the
current block hash, message, and threshold from the website.

The reason that there isn't a convenient `run.bat` file for Windows users is
that I'm not familiar with Windows commands.
