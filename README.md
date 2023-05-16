# program-time
Program time is a program that calculates time spent between two calls. Useful to get compilation time.
## Usage:
```
program_time -start dump_file.pt
// some kind of commands you decided to call
program_time -end dump_file.pt
```

Here dump_file.pt is some kind of temp file that will be empty after the call
of -end. It is necessary to specify it so that the program will be able to keep
the timestamp at which you started calling some commands.
