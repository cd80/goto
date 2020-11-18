# goto
goto utility to stop boring cd &amp; ls

![screenshot](https://github.com/cd80/goto/blob/main/screenshot.png?raw=true)

# Usage
```
usage: goto [OPTION] [ALIAS/INDEX]

	-h              	Print this message
	-l              	List aliases
	-s <directory>  	Set directory to alias
	-c              	Clear alias
```

# Build
```
make
```

# Install
```
make install
```

# UnInstall
```
rm /usr/bin/gotobin
# and remove goto() from shrc
```
Figuring out how to remove function automatically from .zshrc
