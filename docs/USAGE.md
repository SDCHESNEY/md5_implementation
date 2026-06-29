# Usage Guide

## CLI

```bash
./bin/md5 -t "Hello, World!"
./bin/md5 -f document.txt
./bin/md5 -b image.png
echo "test" | ./bin/md5 -s
```

## TUI

Run without arguments to launch the ncurses interface:

```bash
./bin/md5
```

The current scaffold provides the main screen structure and placeholder screens for the remaining TUI flows.