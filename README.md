

# 🧠 Mini UNIX Shell (C Language)  
**Version:** Latest v6.0 — Command Chaining & Background Execution  
**Student:** BSDSF23M022  

---

## 📘 Overview

A modular UNIX-like shell written in **C**, built feature by feature as part of the **Operating Systems** course.  
Implements command parsing, process management, I/O redirection, pipes, history, and background jobs.

---

## ⚙️ Features Implemented

| # | Feature | Description | Version |
|---|----------|--------------|----------|
| 1 | Basic Shell | Executes commands using `fork()` and `execvp()` | v1.0 |
| 2 | Built-in Commands | Added `cd`, `exit`, `help`, `jobs` | v2.0 |
| 3 | Command History | Stores last 20 commands and supports `!n` | v3.0 |
| 4 | GNU Readline | Added arrow keys, tab completion | v4.0 |
| 5 | I/O Redirection & Pipes | Supports `<`, `>`, `>>`, and single  | v5.0 |
| 6 | Chaining & Background Jobs | Supports `;`, `&`, and `jobs` management | v6.0 |

---

## 🧩 Build Instructions
```bash
sudo apt install build-essential libreadline-dev
make
./bin/myshell
```

#### Author : Amir Ali Asif
#### Subject : Operating System (Fall 2025)

