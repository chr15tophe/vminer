#!/bin/sh

# Set your Venonatude username and the value of `remember_token` cookie below.

USERNAME=""
REMEMBER_TOKEN=""

[ -z "$USERNAME" ] && echo "USERNAME not specified. Set it on line 5 of run.sh." && exit 1
[ -z "$REMEMBER_TOKEN" ] && echo "REMEMBER_TOKEN not specified. Set it on line 6 of run.sh." && exit 1

make > /dev/null
mkdir -p tmp
wget --header "Cookie: remember_token=$REMEMBER_TOKEN" \
    https://www.activitiesvenona.org/VENONA2021/VENONACOIN/block \
    -O tmp/current_block.html 2> /dev/null

echo "y" | python3 scripts/auto_conf.py tmp/current_block.html tmp/vminer.auto.conf > /dev/null

./bin/vminer "$USERNAME" tmp/vminer.auto.conf
