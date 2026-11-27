#ifndef BPLUS_TREE_H
#define BPLUS_TREE_H

#include "student.h"
#include <string>
#include <vector>

struct BPlusNode {
    bool isLeaf;
    std::vector<int> keys;
    std::vector<Student> records;       // used only in leaf nodes
    std::vector<BPlusNode*> children;   // used only in internal nodes
    BPlusNode* next;                    // leaf linked list pointer

    explicit BPlusNode(bool leaf) : isLeaf(leaf), next(nullptr) {}
};

class BPlusTree {
public:
    static const int MAX_KEYS = 4;
    static const int MIN_KEYS = MAX_KEYS / 2;

    BPlusTree();
    ~BPlusTree();

    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

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
    BPlusNode* root;
    int count;

    BPlusNode* findLeaf(int id) const;
    bool insertRec(BPlusNode* node, const Student& s,
                   int& upKey, BPlusNode*& newRight, bool& duplicate);
    bool removeRec(BPlusNode* node, int id);
    void fixChildUnderflow(BPlusNode* parent, size_t childIdx);
    int leftmostKey(BPlusNode* node) const;
    void destroyRec(BPlusNode* node);
};

#endif
