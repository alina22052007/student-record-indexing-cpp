#include "avl_tree.h"
#include <algorithm>
#include <cctype>
#include <iostream>

static std::string toUpperStr(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

AVLTree::AVLTree() : root(nullptr), count(0) {}

AVLTree::~AVLTree() {
    clear();
}

void AVLTree::clear() {
    destroyRec(root);
    root = nullptr;
    count = 0;
}

void AVLTree::destroyRec(AVLNode* node) {
    if (!node) return;
    destroyRec(node->left);
    destroyRec(node->right);
    delete node;
}

int AVLTree::height(AVLNode* n) const {
    return n ? n->height : 0;
}

int AVLTree::balanceFactor(AVLNode* n) const {
    return n ? height(n->left) - height(n->right) : 0;
}

void AVLTree::updateHeight(AVLNode* n) {
    if (n) {
        n->height = 1 + std::max(height(n->left), height(n->right));
    }
}

AVLNode* AVLTree::rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* t2 = x->right;

    x->right = y;
    y->left = t2;

    updateHeight(y);
    updateHeight(x);
    return x;
}

AVLNode* AVLTree::rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* t2 = y->left;

    y->left = x;
    x->right = t2;

    updateHeight(x);
    updateHeight(y);
    return y;
}

AVLNode* AVLTree::rebalance(AVLNode* node) {
    updateHeight(node);
    int bf = balanceFactor(node);

    // LL and LR cases
    if (bf > 1) {
        if (balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    // RR and RL cases
    if (bf < -1) {
        if (balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

bool AVLTree::insert(const Student& s) {
    bool inserted = false;
    root = insertRec(root, s, inserted);
    if (inserted) {
        count++;
    }
    return inserted;
}

AVLNode* AVLTree::insertRec(AVLNode* node, const Student& s, bool& inserted) {
    if (!node) {
        inserted = true;
        return new AVLNode(s);
    }

    if (s.id < node->data.id) {
        node->left = insertRec(node->left, s, inserted);
    } else if (s.id > node->data.id) {
        node->right = insertRec(node->right, s, inserted);
    } else {
        inserted = false;
        return node;
    }

    return rebalance(node);
}

AVLNode* AVLTree::minNode(AVLNode* node) const {
    AVLNode* cur = node;
    while (cur && cur->left) {
        cur = cur->left;
    }
    return cur;
}

bool AVLTree::remove(int id) {
    bool removed = false;
    root = removeRec(root, id, removed);
    if (removed) {
        count--;
    }
    return removed;
}

AVLNode* AVLTree::removeRec(AVLNode* node, int id, bool& removed) {
    if (!node) {
        removed = false;
        return nullptr;
    }

    if (id < node->data.id) {
        node->left = removeRec(node->left, id, removed);
    } else if (id > node->data.id) {
        node->right = removeRec(node->right, id, removed);
    } else {
        removed = true;
        if (!node->left || !node->right) {
            AVLNode* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }

        AVLNode* succ = minNode(node->right);
        node->data = succ->data;
        bool dummy = false;
        node->right = removeRec(node->right, succ->data.id, dummy);
    }

    return rebalance(node);
}

bool AVLTree::search(int id, Student& out) const {
    AVLNode* cur = root;
    while (cur) {
        if (id == cur->data.id) {
            out = cur->data;
            return true;
        }
        cur = (id < cur->data.id) ? cur->left : cur->right;
    }
    return false;
}

void AVLTree::getAllSorted(std::vector<Student>& out) const {
    out.clear();
    inorderRec(root, out);
}

void AVLTree::inorderRec(AVLNode* node, std::vector<Student>& out) const {
    if (!node) return;
    inorderRec(node->left, out);
    out.push_back(node->data);
    inorderRec(node->right, out);
}

void AVLTree::rangeQuery(int low, int high, std::vector<Student>& out) const {
    out.clear();
    if (low > high) return;
    rangeRec(root, low, high, out);
}

void AVLTree::rangeRec(AVLNode* node, int low, int high, std::vector<Student>& out) const {
    if (!node) return;

    if (node->data.id > low) {
        rangeRec(node->left, low, high, out);
    }
    if (node->data.id >= low && node->data.id <= high) {
        out.push_back(node->data);
    }
    if (node->data.id < high) {
        rangeRec(node->right, low, high, out);
    }
}

void AVLTree::filterByDept(const std::string& dept, std::vector<Student>& out) const {
    out.clear();
    deptRec(root, toUpperStr(dept), out);
}

void AVLTree::deptRec(AVLNode* node, const std::string& dept, std::vector<Student>& out) const {
    if (!node) return;
    deptRec(node->left, dept, out);
    if (toUpperStr(node->data.dept) == dept) {
        out.push_back(node->data);
    }
    deptRec(node->right, dept, out);
}

void AVLTree::filterTopCgpa(double threshold, std::vector<Student>& out) const {
    out.clear();
    cgpaRec(root, threshold, out);
}

void AVLTree::cgpaRec(AVLNode* node, double threshold, std::vector<Student>& out) const {
    if (!node) return;
    cgpaRec(node->left, threshold, out);
    if (node->data.cgpa > threshold) {
        out.push_back(node->data);
    }
    cgpaRec(node->right, threshold, out);
}

void AVLTree::printStructure() const {
    if (!root) {
        std::cout << "(empty AVL tree)\n";
        return;
    }
    std::cout << "AVL Tree (nodes: " << count << ", height: " << getHeight() << ")\n";
    printRec(root, "", false);
}

void AVLTree::printRec(AVLNode* node, const std::string& prefix, bool isLeft) const {
    if (!node) return;
    std::cout << prefix << (isLeft ? "|-- " : "+-- ")
              << node->data.id << " (" << node->data.name
              << ", bf=" << balanceFactor(node) << ")\n";

    std::string nextPrefix = prefix + (isLeft ? "|   " : "    ");
    if (node->left || node->right) {
        if (node->left) {
            printRec(node->left, nextPrefix, true);
        } else {
            std::cout << nextPrefix << "|-- (null)\n";
        }
        if (node->right) {
            printRec(node->right, nextPrefix, false);
        } else {
            std::cout << nextPrefix << "+-- (null)\n";
        }
    }
}

int AVLTree::size() const {
    return count;
}

int AVLTree::getHeight() const {
    return height(root);
}
