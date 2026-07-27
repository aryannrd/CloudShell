A Unix shell built from scratch in C, with a full-stack telemetry and prediction pipeline. CloudShell executes real system commands, tracks usage patterns, and uses a Markov chain model to predict the next command.
All of this is served through a Dockerized FastAPI backend and a React dashboard.

Browser (React + xterm.js)
        ↕ WebSockets (terminal I/O)
        ↕ HTTP (stats, predictions)
FastAPI Backend (Docker)
        ├── SQLite (telemetry storage)
        ├── PTY → C Shell process (custom shell that has bash functionality, coded for full functionality)
        └── Markov chain predictor

1. Features
  -Shell (C)
  -REPL loop with raw mode terminal input (termios)
  -Manual tokenizer and parser with quote and escape character support
  -Command execution via fork / execvp / waitpid
  -Built-in commands: cd, exit, jobs, fg, bg
  -Input and output redirection (<, >)
  -Single and multi-command pipes (|)
  -Background execution (&)
  -Job control with SIGTSTP, SIGINT, SIGCHLD handling
  -Process groups for correct signal delivery
  -Command history with arrow key navigation
  -Environment variable expansion ($VAR)
  -Execution telemetry: timestamp, command, exit code, working directory, duration
   
2. Backend (Python / FastAPI)
  /log — receives shell telemetry and stores in SQLite
  /predict — returns predicted next command using a Markov chain transition table
  /stats — returns top commands by frequency
  /health — health check endpoint
  onevent("startup") - creates database to store telemety
  onevent("shutdown")- kills current shell session and creates new one
  WebSocket endpoint for browser terminal (PTY bridge)

4. Frontend (React)
  -Browser-based terminal powered by xterm.js
  -Real-time WebSocket connection to shell process
  -Workflow graph visualizing command transition patterns (Yet to finish)
