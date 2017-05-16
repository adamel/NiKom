The botcheck script can be used as a simple but effective way to get
rid of bots attempting to connect.

Requirements:
- The readline.py script must be found on the system path.

It's intended to be launched by inetd. Example configuration file
additions are shown below.

/etc/services:

    mybbs 2323/tcp

/etc/inetd.conf:

    mybbs stream  tcp     nowait  nobody  /path/to/botcheck       botcheck 1234 amiga4000 23

In the above example the external port you connect to is 2323, the expected
answer on the botcheck question is "1234" and if you answer "1234", you will
be forwarded to the host amiga4000 on port 23.
