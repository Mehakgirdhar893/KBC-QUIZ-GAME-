KBC Quiz Simulator (C)

A Windows console-based KBC-style quiz game written in C. Users can navigate questions, select or change answers, and jump between questions. The game features colored output, a LED-style scrolling “WELCOME” display, and calculates results with prize money at the end.

🎮 Features
📂 File-based question bank (ques.txt)
🎯 Multiple-choice questions (A–D)
⏮ Previous / ⏭ Next / 🔢 Jump to question
✏ Answer and change answers anytime before submission
🎨 Colored console output (Windows)
🔴 LED scrolling WELCOME animation
🧮 Final score & total prize money calculation

System Requirements

- Windows operating system
- GCC compiler (MinGW recommended)
- Console supporting ANSI colors


Question File Format

Each question in ques.txt uses 8 lines:
1.Question text
2.Option A
3.Option B
4.Option C
5.Option D
6.Correct option (A–D)
7.Timeout in seconds
8.Prize money
