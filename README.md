🐚 Custom Unix Shell in C

A lightweight Unix-like shell implemented in C that supports command execution, input/output redirection, parallel and sequential command execution, and pipelining.
Built using fundamental system calls like fork(), execvp(), wait(), and pipe().

🚀 Features

Basic Command Execution – Run standard Linux commands like ls, pwd, cat, etc.

Sequential Execution (##) – Execute multiple commands one after another.

ls ## pwd ## date


Parallel Execution (&&) – Execute multiple commands simultaneously.

ls && date && echo "Done"


Output Redirection (>) – Redirect command output to a file.

ls > files.txt


Pipelining (|) – Pass output of one command as input to another.

ls | grep ".c"


Built-in Commands

cd [directory] – Change directory

exit – Exit the shell gracefully

Signal Handling – Ignores Ctrl+C and Ctrl+Z to prevent accidental termination.

⚙️ How It Works

Parses user input and detects special symbols (>, ##, &&, |).

Forks child processes to execute commands.

Handles redirection and pipes using file descriptors and dup2().

Implements parallelism via multiple forked processes.

Waits for all child processes to finish before showing the next prompt.

🧩 Technologies Used

Language: C

System Calls: fork(), execvp(), waitpid(), pipe(), dup2(), signal()

Headers: <unistd.h>, <sys/types.h>, <sys/wait.h>, <fcntl.h>, <signal.h>

🖥️ Usage
1️⃣ Compile
gcc shell.c -o shell

2️⃣ Run
./shell

3️⃣ Try Commands
pwd
ls -l > out.txt
cat out.txt
ls && date
ls | grep .c

📂 File Structure
.
├── shell.c        # Main program file
└── README.md      # Project documentation

🧠 Learning Outcomes

Deepened understanding of process management and system-level programming.

Implemented inter-process communication (IPC) using pipes.

Gained hands-on experience with signal handling and I/O redirection.
