# UNIX-Shell-Implementation-in-C
A fully-featured UNIX shell implementation written in C that supports basic Linux commands, I/O redirection, piping, and advanced shell features. This project was developed as a system programming assignment to demonstrate proficiency with UNIX system calls and shell internals.

# Core Shell Functionality
Interactive Command Prompt: Customizable prompt (sh>) for user input

Command Execution: Supports execution of basic Linux commands using fork() and exec() system calls

Error Handling: Comprehensive error handling for system calls and invalid commands

# I/O Redirection
Input Redirection (<): Redirect input from files to commands

Output Redirection (>): Overwrite output to files

Append Redirection (>>): Append output to existing files

Implementation: Uses dup() and dup2() system calls for file descriptor manipulation

# Advanced Features
Command Piping (|): Supports any number of pipes (e.g., cmd1 | cmd2 | cmd3 | cmd4)

Multiple Commands:

Sequential execution with semicolons (;)

Conditional execution with logical AND (&&)

Command History: Tracks and maintains history of executed commands

Signal Handling:

CTRL+C terminates only the currently running child process, not the shell itself

Implemented using signal() and sigaction() system calls

# Technical Implementation
System Calls Used
fork() - Process creation

exec() family - Command execution

dup()/dup2() - I/O redirection

pipe() - Inter-process communication

wait()/waitpid() - Process management

signal()/sigaction() - Signal handling

open()/close() - File operations
