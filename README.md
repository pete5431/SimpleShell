# OS-Shell

How to use the program.

The shell must be ran with ./myshell

Will print a continous prompt:

pathname/myshell> 

Where pathname is the current directory path.

Enter commands after the prompt. There are certain built-ins to know about. Externals, the commands usually used in Linux/Unix Shells can be used normally to an extent.



BUILT IN COMMANDS:

The following commands are built-in.

cd: Accepts one or no arguments. If no arguments, print the current directory. Otherwise go to the specific directory.

Examples:

cd    (prints current directory)

cd .. (goes back one directory)

cd testdir (goes into testdir is it exists)



dir: Accepts many arguments. Will print the file contents of the directory names. Supports output redirect.

Examples:

dir         (prints contents of current directory)

dir testdir (prints contents of testdir)

dir testdir > output.txt  (prints contents of testdir into output.txt)



echo: Accepts many arguments. Will print all the content after it to screen. Supports output redirect.

Examples:

echo              (prints \n to screen)

echo something    (prints something to screen)

echo something > output.txt (prints something to output.txt)



environ: Prints out all environment variables. Supports output redirect.

Examples:

environ          (prints all env variables)

environ > output.txt  (prints all env variables into output.txt)



pause: Accepts no arguments. Will pause the shell until a key is entered.

Examples:

pause



help: Accepts one argument, the command for which help is needed. Prints the built-in manual for it. Supports output redirect.

Examples:

help cd   (prints manual for cd)

help dir   (prints manual for dir)



exit: Accepts no arguments. Will exit the shell.

Examples:

exit



clr: Clears the screen like clear on regular shell.

Examples:

clr



I/O REDIRECTIONS:

Only the following scenarios are considerd for I/O redirection.

<

< and >

< and >>

The < is considered input redirection, and is used with a program name in front and a filename after like this:

testProgram < test.txt

This makes it so that when testProgram reads from stdin, it read from test.txt instead.

The > is considered output redirection that truncates a file, or overwrites it. It also creates it if it doesn't exist.

Example of how to use it:

testProgram > test.txt

Any output of testProgram is printed to test.txt

The >> is similar but appends to an existing file.

Example:

testProgram >> test.txt

The output of testProgram is now appended to test.txt

Input and output can be combined like so:

testProgram < input.txt > test.txt

testProgram reads from input.txt and its output is printed in test.txt

Same can be done for:

testProgram < input.txt >> test.txt



PIPING:

Only single pipes are supported.

Piping is when the output of one program is the input of another.

A pipe has a write end and read end. The first program in a pipe writes to write end, and the second reads from read end.

Pipes are in one direction.

Examples:

testProgram | wc

-Whatever testProgram prints is written to the write end of pipe and wc then counts its characters.

PrintHello | ReadLine

-Assume PrintHello prints "Hello" to screen and thats it. Readline simply read a line from stdin and prints it.
-PrintHello prints hello, but with pipe its written to the write end of the pipe and read by Readline in read end. So it prints Hello.



BACKGROUND:

Runs a command in the background.

Can run multiple commands in parallel.

Examples:

ls &

-runs ls in background.

ls & ls & ls &

-runs multiple ls in background.


BASH MODE:

Run shell with a text file containg commands.

Example:

./myshell bash.txt

Will exit shell upon EOF.


