# What is it?
Faye is a terminal-based file explorer.

## Build
To build faye, use the build script
```
./build.sh
```

## Usage
After building it, faye can be executed by itself to open the current directory
```
faye
```

Or, alternatively, the path to the starting directory can be set by the first argument
```
faye <path>
```

### Changing cwd on exit
In order to make faye able to change cwd on exit, paste this function at the end of your startup script
```
fy (){
	ret=$(faye $1)
	cd $ret
}

``` 
And then launch faye with
```
fy
```

## Milestones
Check faye milestones [here](ROADMAP.md).

## Documentation
Check implementation details [here](notes/details.md).
