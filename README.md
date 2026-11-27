# University Student Record & Academic Indexing System

A C++17 console application for managing university student records using both an **AVL Tree** and a **B+ Tree**, indexed by `Student ID`.

## Files

- `student.h` — `Student` record structure, table printing, and CSV helpers
- `avl_tree.h` / `avl_tree.cpp` — AVL Tree implementation with rotations, traversals, filtering, and ASCII visualization
- `bplus_tree.h` / `bplus_tree.cpp` — B+ Tree (order 5, linked leaves) with insert, borrow/merge delete, range scan, and level-order display
- `main.cpp` — Menu-driven CLI, CSV persistence, challenges, and benchmark comparison
- `students.csv` — Sample dataset (10 student records)
- `Makefile` — Build configuration

## Student Record Fields

| Field | Type | Description |
|-------|------|-------------|
| Student ID | `int` | Unique identifier (index key) |
| Student Name | `string` | Full name |
| Department | `string` | Department (`CSE`, `IT`, `AIML`, etc.) |
| Semester | `int` | Current semester |
| CGPA | `double` | Academic performance |
| City | `string` | Student city |

## Build & Run

```bash
make
./student_system
```

Or compile directly with `g++`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp avl_tree.cpp bplus_tree.cpp -o student_system
./student_system
```

## Implemented Features

- Insert, delete, and search student records by ID across both AVL and B+ Tree
- Display all records sorted by Student ID
- Single and multiple ID range queries
- Duplicate Student ID detection
- CSV persistence (`students.csv` loaded on startup and saved on updates/exit)
- Department-wise filtering (preserving ID-sorted order)
- Top CGPA student lookup (`CGPA > 9.0`)
- ASCII visualization for both AVL and B+ Tree structures
- Automated performance comparison between AVL Tree and B+ Tree (insert, point search, range query, delete) with terminal bar graphs
