#include "avl_tree.h"
#include "bplus_tree.h"
#include "student.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

const std::string DATA_FILE = "students.csv";

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            clearInput();
            return val;
        }
        std::cout << "Invalid number, try again.\n";
        clearInput();
    }
}

static double readDouble(const std::string& prompt) {
    double val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            clearInput();
            return val;
        }
        std::cout << "Invalid decimal value, try again.\n";
        clearInput();
    }
}

static std::string readText(const std::string& prompt) {
    std::string s;
    std::cout << prompt;
    std::getline(std::cin, s);
    return s;
}

static void seedDefaultStudents(AVLTree& avl, BPlusTree& bpt) {
    std::vector<Student> defaults = {
        {1001, "Aditi Sharma",   "CSE",  5, 8.7, "Pune"},
        {1002, "Rahul Patil",    "IT",   5, 7.9, "Mumbai"},
        {1003, "Neha Desai",     "AIML", 3, 9.1, "Ahmedabad"},
        {1004, "Arjun Mehta",    "CSE",  7, 8.2, "Bengaluru"},
        {1005, "Sneha Kulkarni", "IT",   3, 8.5, "Nagpur"},
        {1006, "Rohan Shah",     "AIML", 5, 7.6, "Surat"},
        {1007, "Priya Nair",     "CSE",  3, 9.3, "Kochi"},
        {1008, "Karan Joshi",    "IT",   7, 8.0, "Nashik"},
        {1009, "Isha Gupta",     "AIML", 5, 8.9, "Delhi"},
        {1010, "Aditya Rao",     "CSE",  7, 7.8, "Hyderabad"}
    };

    for (const Student& s : defaults) {
        avl.insert(s);
        bpt.insert(s);
    }
}

static bool saveToCsv(const BPlusTree& bpt, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cout << "Could not open " << filename << " for writing.\n";
        return false;
    }

    out << "StudentID,StudentName,Department,Semester,CGPA,City\n";
    std::vector<Student> all;
    bpt.getAllSorted(all);
    for (const Student& s : all) {
        out << toCsvLine(s) << "\n";
    }
    return true;
}

static void loadFromCsv(AVLTree& avl, BPlusTree& bpt, const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cout << filename << " not found. Loading default 10 student records.\n";
        seedDefaultStudents(avl, bpt);
        saveToCsv(bpt, filename);
        return;
    }

    std::string line;
    int loaded = 0;
    bool firstLine = true;
    while (std::getline(in, line)) {
        if (firstLine) {
            firstLine = false;
            if (line.find("StudentID") != std::string::npos) {
                continue;
            }
        }
        Student s;
        if (parseCsvLine(line, s)) {
            if (avl.insert(s)) {
                bpt.insert(s);
                loaded++;
            }
        }
    }

    if (loaded == 0) {
        std::cout << filename << " was empty. Seeding default records.\n";
        seedDefaultStudents(avl, bpt);
        saveToCsv(bpt, filename);
    } else {
        std::cout << "Loaded " << loaded << " records from " << filename << ".\n";
    }
}

static void printStudentList(const std::vector<Student>& list) {
    if (list.empty()) {
        std::cout << "No matching records found.\n";
        return;
    }
    printTableHeader();
    for (const Student& s : list) {
        printStudentRow(s);
    }
    std::cout << "Total: " << list.size() << " record(s).\n";
}

static void handleAdd(AVLTree& avl, BPlusTree& bpt) {
    int id = readInt("Student ID: ");

    Student existing;
    if (avl.search(id, existing)) {
        std::cout << "Error: Duplicate Student ID " << id
                  << " already exists (" << existing.name << ").\n";
        return;
    }

    std::string name = readText("Full Name: ");
    std::string dept = readText("Department (e.g. CSE, IT, AIML): ");
    int sem = readInt("Semester: ");
    double cgpa = readDouble("CGPA: ");
    std::string city = readText("City: ");

    Student s(id, name, dept, sem, cgpa, city);
    bool okAvl = avl.insert(s);
    bool okBpt = bpt.insert(s);

    if (okAvl && okBpt) {
        saveToCsv(bpt, DATA_FILE);
        std::cout << "Student " << id << " added to both trees.\n";
    } else {
        std::cout << "Error: Failed to insert duplicate record.\n";
    }
}

static void handleDelete(AVLTree& avl, BPlusTree& bpt) {
    int id = readInt("Enter Student ID to delete: ");
    bool okAvl = avl.remove(id);
    bool okBpt = bpt.remove(id);

    if (okAvl && okBpt) {
        saveToCsv(bpt, DATA_FILE);
        std::cout << "Student " << id << " removed from both trees.\n";
    } else {
        std::cout << "Student ID " << id << " not found.\n";
    }
}

static void handleSearch(const AVLTree& avl, const BPlusTree& bpt) {
    int id = readInt("Enter Student ID to search: ");
    Student sAvl, sBpt;

    auto t1 = Clock::now();
    bool foundAvl = avl.search(id, sAvl);
    auto t2 = Clock::now();
    bool foundBpt = bpt.search(id, sBpt);
    auto t3 = Clock::now();

    auto usAvl = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    auto usBpt = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();

    if (foundAvl && foundBpt) {
        printTableHeader();
        printStudentRow(sBpt);
    } else {
        std::cout << "Student ID " << id << " not found.\n";
    }

    std::cout << "Lookup time -> AVL: " << usAvl << " ns | B+ Tree: " << usBpt << " ns\n";
}

static void handleDisplayAll(const AVLTree& avl, const BPlusTree& bpt) {
    std::cout << "1. Display via AVL Tree (in-order)\n";
    std::cout << "2. Display via B+ Tree (leaf scan)\n";
    int ch = readInt("Choice: ");

    std::vector<Student> list;
    auto t1 = Clock::now();
    if (ch == 2) {
        bpt.getAllSorted(list);
    } else {
        avl.getAllSorted(list);
    }
    auto t2 = Clock::now();

    printStudentList(list);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Traversal time: " << us << " us\n";
}

static void handleRangeQuery(const AVLTree& avl, const BPlusTree& bpt) {
    int low = readInt("Start ID: ");
    int high = readInt("End ID: ");

    if (low > high) {
        std::cout << "Start ID cannot be greater than End ID.\n";
        return;
    }

    std::vector<Student> resAvl, resBpt;
    auto t1 = Clock::now();
    avl.rangeQuery(low, high, resAvl);
    auto t2 = Clock::now();
    bpt.rangeQuery(low, high, resBpt);
    auto t3 = Clock::now();

    printStudentList(resBpt);
    auto nsAvl = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    auto nsBpt = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
    std::cout << "Range query time -> AVL: " << nsAvl << " ns | B+ Tree: " << nsBpt << " ns\n";
}

static void handleMultiRange(const AVLTree& avl, const BPlusTree& bpt) {
    int k = readInt("How many range queries to run? ");
    if (k <= 0) return;

    std::vector<std::pair<int, int>> ranges;
    for (int i = 0; i < k; ++i) {
        std::cout << "Range " << (i + 1) << ":\n";
        int lo = readInt("  Low ID: ");
        int hi = readInt("  High ID: ");
        ranges.push_back({lo, hi});
    }

    for (size_t i = 0; i < ranges.size(); ++i) {
        int lo = ranges[i].first;
        int hi = ranges[i].second;
        std::cout << "\n--- Query " << (i + 1) << ": [" << lo << " .. " << hi << "] ---\n";

        std::vector<Student> r1, r2;
        auto t1 = Clock::now();
        avl.rangeQuery(lo, hi, r1);
        auto t2 = Clock::now();
        bpt.rangeQuery(lo, hi, r2);
        auto t3 = Clock::now();

        printStudentList(r2);
        auto nsAvl = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        auto nsBpt = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
        std::cout << "AVL: " << nsAvl << " ns | B+ Tree: " << nsBpt << " ns\n";
    }
}

static void handleDeptFilter(const BPlusTree& bpt) {
    std::string dept = readText("Enter Department (e.g. CSE, IT, AIML): ");
    std::vector<Student> res;
    bpt.filterByDept(dept, res);
    printStudentList(res);
}

static void handleTopCgpa(const BPlusTree& bpt) {
    std::vector<Student> res;
    bpt.filterTopCgpa(9.0, res);
    std::cout << "\nStudents with CGPA > 9.0:\n";
    printStudentList(res);
}

static void handleVisualize(const AVLTree& avl, const BPlusTree& bpt) {
    if (avl.size() > 60) {
        std::cout << "Tree has " << avl.size() << " records; visualization may be long.\n";
    }
    std::cout << "\n";
    avl.printStructure();
    std::cout << "\n";
    bpt.printStructure();
}

static void printBar(const std::string& label, double valMs, double maxMs) {
    int width = 30;
    int len = (maxMs > 0.0) ? static_cast<int>((valMs / maxMs) * width) : 0;
    if (valMs > 0.0 && len == 0) len = 1;
    std::cout << "  " << std::left << std::setw(10) << label << " |"
              << std::string(len, '#') << " "
              << std::fixed << std::setprecision(3) << valMs << " ms\n";
}

static void handleBenchmark() {
    int n = readInt("Number of synthetic records to benchmark (e.g. 5000): ");
    if (n <= 0) {
        std::cout << "Record count must be positive.\n";
        return;
    }

    std::vector<Student> dataset;
    dataset.reserve(static_cast<size_t>(n));
    const char* depts[] = {"CSE", "IT", "AIML", "ECE", "MECH"};
    const char* cities[] = {"Pune", "Mumbai", "Delhi", "Bengaluru", "Hyderabad", "Kochi"};

    std::vector<int> ids(static_cast<size_t>(n));
    std::iota(ids.begin(), ids.end(), 10000);
    std::mt19937 rng(42);
    std::shuffle(ids.begin(), ids.end(), rng);

    for (int i = 0; i < n; ++i) {
        dataset.emplace_back(
            ids[static_cast<size_t>(i)],
            "Student_" + std::to_string(ids[static_cast<size_t>(i)]),
            depts[i % 5],
            (i % 8) + 1,
            6.0 + (i % 40) * 0.1,
            cities[i % 6]
        );
    }

    AVLTree testAvl;
    BPlusTree testBpt;

    // 1. Insertion
    auto t1 = Clock::now();
    for (const Student& s : dataset) testAvl.insert(s);
    auto t2 = Clock::now();
    for (const Student& s : dataset) testBpt.insert(s);
    auto t3 = Clock::now();

    double avlInsertMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double bptInsertMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    // 2. Point search (1000 lookups)
    int searchCount = std::min(n, 1000);
    std::vector<int> queryIds(static_cast<size_t>(searchCount));
    for (int i = 0; i < searchCount; ++i) {
        queryIds[static_cast<size_t>(i)] = ids[static_cast<size_t>((i * 7) % n)];
    }

    Student tmp;
    int avlHits = 0, bptHits = 0;
    t1 = Clock::now();
    for (int id : queryIds) {
        if (testAvl.search(id, tmp)) avlHits++;
    }
    t2 = Clock::now();
    for (int id : queryIds) {
        if (testBpt.search(id, tmp)) bptHits++;
    }
    t3 = Clock::now();

    double avlSearchMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double bptSearchMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    // 3. Range queries (200 ranges of width 500)
    int rangeQueries = 200;
    std::vector<Student> rangeRes;
    long long avlRangeTotal = 0, bptRangeTotal = 0;

    t1 = Clock::now();
    for (int i = 0; i < rangeQueries; ++i) {
        int lo = 10000 + (i * (n / rangeQueries));
        testAvl.rangeQuery(lo, lo + 500, rangeRes);
        avlRangeTotal += static_cast<long long>(rangeRes.size());
    }
    t2 = Clock::now();
    for (int i = 0; i < rangeQueries; ++i) {
        int lo = 10000 + (i * (n / rangeQueries));
        testBpt.rangeQuery(lo, lo + 500, rangeRes);
        bptRangeTotal += static_cast<long long>(rangeRes.size());
    }
    t3 = Clock::now();

    double avlRangeMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double bptRangeMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    // 4. Deletions (delete 20% of records)
    int delCount = std::max(1, n / 5);
    t1 = Clock::now();
    for (int i = 0; i < delCount; ++i) {
        testAvl.remove(ids[static_cast<size_t>(i)]);
    }
    t2 = Clock::now();
    for (int i = 0; i < delCount; ++i) {
        testBpt.remove(ids[static_cast<size_t>(i)]);
    }
    t3 = Clock::now();

    double avlDelMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double bptDelMs = std::chrono::duration<double, std::milli>(t3 - t2).count();

    std::cout << "\n--- Benchmark Report (" << n << " records) ---\n";
    std::cout << std::left << std::setw(22) << "Operation"
              << std::setw(15) << "AVL (ms)"
              << std::setw(15) << "B+ Tree (ms)" << "\n";
    std::cout << std::string(52, '-') << "\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << std::left << std::setw(22) << "Bulk Insert"
              << std::setw(15) << avlInsertMs << std::setw(15) << bptInsertMs << "\n";
    std::cout << std::left << std::setw(22) << ("Search (" + std::to_string(searchCount) + "x)")
              << std::setw(15) << avlSearchMs << std::setw(15) << bptSearchMs << "\n";
    std::cout << std::left << std::setw(22) << ("Range (" + std::to_string(rangeQueries) + "x)")
              << std::setw(15) << avlRangeMs << std::setw(15) << bptRangeMs << "\n";
    std::cout << std::left << std::setw(22) << ("Delete (" + std::to_string(delCount) + "x)")
              << std::setw(15) << avlDelMs << std::setw(15) << bptDelMs << "\n";

    std::cout << "\nConsistency check: search hits (" << avlHits << "/" << bptHits
              << "), range matches (" << avlRangeTotal << "/" << bptRangeTotal
              << "), remaining size (" << testAvl.size() << "/" << testBpt.size() << ")\n";

    std::cout << "\nVisual comparison:\n";
    std::cout << "Insert:\n";
    double m1 = std::max(avlInsertMs, bptInsertMs);
    printBar("AVL", avlInsertMs, m1);
    printBar("B+ Tree", bptInsertMs, m1);

    std::cout << "Point Search:\n";
    double m2 = std::max(avlSearchMs, bptSearchMs);
    printBar("AVL", avlSearchMs, m2);
    printBar("B+ Tree", bptSearchMs, m2);

    std::cout << "Range Search:\n";
    double m3 = std::max(avlRangeMs, bptRangeMs);
    printBar("AVL", avlRangeMs, m3);
    printBar("B+ Tree", bptRangeMs, m3);

    std::cout << "Delete:\n";
    double m4 = std::max(avlDelMs, bptDelMs);
    printBar("AVL", avlDelMs, m4);
    printBar("B+ Tree", bptDelMs, m4);
}

static void showMenu() {
    std::cout << "\n--- University Student Indexing System ---\n";
    std::cout << "1. Add student\n";
    std::cout << "2. Delete student\n";
    std::cout << "3. Search student by ID\n";
    std::cout << "4. Display all students (sorted)\n";
    std::cout << "5. Range query (ID between low and high)\n";
    std::cout << "6. Multiple range queries\n";
    std::cout << "7. Filter by department\n";
    std::cout << "8. Top CGPA students (> 9.0)\n";
    std::cout << "9. Visualize AVL and B+ Tree\n";
    std::cout << "10. Compare AVL vs B+ Tree performance\n";
    std::cout << "0. Save and exit\n";
}

int main() {
    AVLTree avl;
    BPlusTree bpt;

    loadFromCsv(avl, bpt, DATA_FILE);

    while (true) {
        showMenu();
        int choice = readInt("Select option: ");

        switch (choice) {
            case 1: handleAdd(avl, bpt); break;
            case 2: handleDelete(avl, bpt); break;
            case 3: handleSearch(avl, bpt); break;
            case 4: handleDisplayAll(avl, bpt); break;
            case 5: handleRangeQuery(avl, bpt); break;
            case 6: handleMultiRange(avl, bpt); break;
            case 7: handleDeptFilter(bpt); break;
            case 8: handleTopCgpa(bpt); break;
            case 9: handleVisualize(avl, bpt); break;
            case 10: handleBenchmark(); break;
            case 0:
                saveToCsv(bpt, DATA_FILE);
                std::cout << "Records saved to " << DATA_FILE << ". Exiting.\n";
                return 0;
            default:
                std::cout << "Invalid option.\n";
        }
    }
}
