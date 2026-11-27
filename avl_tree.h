#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "student.h"
#include <string>
#include <vector>

struct AVLNode {
    Student data;
    AVLNode* left;
    AVLNode* right;
    int height;

    explicit AVLNode(const Student& s)
        : data(s), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
public:
    AVLTree();
    ~AVLTree();

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(const Student& s);
    bool remove(int id);
    bool search(int id, Student& out) const;

    void getAllSorted(std::vector<Student>& out) const;
    void rangeQuery(int low, int high, std::vector<Student>& out) const;
    void filterByDept(const std::string& dept, std::vector<Student>& out) const;
    void filterTopCgpa(double threshold, std::vector<Student>& out) const;

    void printStructure() const;
    int size() const;
    int getHeight() const;
    void clear();

private:
    AVLNode* root;
    int count;

    int height(AVLNode* n) const;
    int balanceFactor(AVLNode* n) const;
    void updateHeight(AVLNode* n);

    AVLNode* rotateRight(AVLNode* y);
    AVLNode* rotateLeft(AVLNode* x);
    AVLNode* rebalance(AVLNode* node);

    AVLNode* insertRec(AVLNode* node, const Student& s, bool& inserted);
    AVLNode* removeRec(AVLNode* node, int id, bool& removed);
    AVLNode* minNode(AVLNode* node) const;

    void inorderRec(AVLNode* node, std::vector<Student>& out) const;
    void rangeRec(AVLNode* node, int low, int high, std::vector<Student>& out) const;
    void deptRec(AVLNode* node, const std::string& dept, std::vector<Student>& out) const;
    void cgpaRec(AVLNode* node, double threshold, std::vector<Student>& out) const;
    void printRec(AVLNode* node, const std::string& prefix, bool isLeft) const;
    void destroyRec(AVLNode* node);
};

#endif
