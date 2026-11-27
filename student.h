#ifndef STUDENT_H
#define STUDENT_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

struct Student {
    int id;
    std::string name;
    std::string dept;
    int sem;
    double cgpa;
    std::string city;

    Student() : id(0), sem(0), cgpa(0.0) {}
    Student(int id, const std::string& name, const std::string& dept,
            int sem, double cgpa, const std::string& city)
        : id(id), name(name), dept(dept), sem(sem), cgpa(cgpa), city(city) {}
};

inline void printTableHeader() {
    std::cout << std::left
              << std::setw(8) << "ID"
              << std::setw(20) << "Name"
              << std::setw(8) << "Dept"
              << std::setw(6) << "Sem"
              << std::setw(8) << "CGPA"
              << std::setw(15) << "City" << "\n";
    std::cout << std::string(65, '-') << "\n";
}

inline void printStudentRow(const Student& s) {
    std::cout << std::left
              << std::setw(8) << s.id
              << std::setw(20) << s.name
              << std::setw(8) << s.dept
              << std::setw(6) << s.sem
              << std::setw(8) << std::fixed << std::setprecision(2) << s.cgpa
              << std::setw(15) << s.city << "\n";
}

inline std::string toCsvLine(const Student& s) {
    std::ostringstream oss;
    oss << s.id << "," << s.name << "," << s.dept << ","
        << s.sem << "," << std::fixed << std::setprecision(2) << s.cgpa
        << "," << s.city;
    return oss.str();
}

inline bool parseCsvLine(const std::string& line, Student& out) {
    if (line.empty()) return false;

    std::string clean = line;
    if (!clean.empty() && clean.back() == '\r') {
        clean.pop_back();
    }
    if (clean.empty()) return false;

    std::stringstream ss(clean);
    std::string idStr, name, dept, semStr, cgpaStr, city;

    if (!std::getline(ss, idStr, ',')) return false;
    if (!std::getline(ss, name, ',')) return false;
    if (!std::getline(ss, dept, ',')) return false;
    if (!std::getline(ss, semStr, ',')) return false;
    if (!std::getline(ss, cgpaStr, ',')) return false;
    if (!std::getline(ss, city)) return false;

    try {
        out.id = std::stoi(idStr);
        out.name = name;
        out.dept = dept;
        out.sem = std::stoi(semStr);
        out.cgpa = std::stod(cgpaStr);
        out.city = city;
    } catch (...) {
        return false;
    }
    return true;
}

#endif
