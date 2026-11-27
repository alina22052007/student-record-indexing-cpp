#include "bplus_tree.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <queue>

static std::string upperDept(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

BPlusTree::BPlusTree() : root(nullptr), count(0) {}

BPlusTree::~BPlusTree() {
    clear();
}

void BPlusTree::clear() {
    destroyRec(root);
    root = nullptr;
    count = 0;
}

void BPlusTree::destroyRec(BPlusNode* node) {
    if (!node) return;
    if (!node->isLeaf) {
        for (BPlusNode* child : node->children) {
            destroyRec(child);
        }
    }
    delete node;
}

BPlusNode* BPlusTree::findLeaf(int id) const {
    if (!root) return nullptr;
    BPlusNode* cur = root;
    while (!cur->isLeaf) {
        size_t i = 0;
        while (i < cur->keys.size() && id >= cur->keys[i]) {
            i++;
        }
        cur = cur->children[i];
    }
    return cur;
}

bool BPlusTree::search(int id, Student& out) const {
    BPlusNode* leaf = findLeaf(id);
    if (!leaf) return false;

    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), id);
    if (it != leaf->keys.end() && *it == id) {
        size_t idx = static_cast<size_t>(it - leaf->keys.begin());
        out = leaf->records[idx];
        return true;
    }
    return false;
}

bool BPlusTree::insert(const Student& s) {
    if (!root) {
        root = new BPlusNode(true);
        root->keys.push_back(s.id);
        root->records.push_back(s);
        count = 1;
        return true;
    }

    int upKey = 0;
    BPlusNode* newRight = nullptr;
    bool duplicate = false;

    bool split = insertRec(root, s, upKey, newRight, duplicate);
    if (duplicate) {
        return false;
    }

    if (split) {
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->keys.push_back(upKey);
        newRoot->children.push_back(root);
        newRoot->children.push_back(newRight);
        root = newRoot;
    }

    count++;
    return true;
}

bool BPlusTree::insertRec(BPlusNode* node, const Student& s,
                          int& upKey, BPlusNode*& newRight, bool& duplicate) {
    if (node->isLeaf) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), s.id);
        if (it != node->keys.end() && *it == s.id) {
            duplicate = true;
            return false;
        }

        size_t pos = static_cast<size_t>(it - node->keys.begin());
        node->keys.insert(node->keys.begin() + pos, s.id);
        node->records.insert(node->records.begin() + pos, s);

        if (static_cast<int>(node->keys.size()) <= MAX_KEYS) {
            return false;
        }

        // Split leaf node: right node keeps the middle key
        size_t mid = node->keys.size() / 2;
        newRight = new BPlusNode(true);
        newRight->keys.assign(node->keys.begin() + mid, node->keys.end());
        newRight->records.assign(node->records.begin() + mid, node->records.end());

        node->keys.resize(mid);
        node->records.resize(mid);

        newRight->next = node->next;
        node->next = newRight;

        upKey = newRight->keys[0];
        return true;
    }

    size_t idx = 0;
    while (idx < node->keys.size() && s.id >= node->keys[idx]) {
        idx++;
    }

    int childUpKey = 0;
    BPlusNode* childRight = nullptr;
    bool childSplit = insertRec(node->children[idx], s, childUpKey, childRight, duplicate);
    if (duplicate || !childSplit) {
        return false;
    }

    node->keys.insert(node->keys.begin() + idx, childUpKey);
    node->children.insert(node->children.begin() + idx + 1, childRight);

    if (static_cast<int>(node->keys.size()) <= MAX_KEYS) {
        return false;
    }

    // Split internal node: middle key moves up and is removed from both halves
    size_t mid = node->keys.size() / 2;
    upKey = node->keys[mid];

    newRight = new BPlusNode(false);
    newRight->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newRight->children.assign(node->children.begin() + mid + 1, node->children.end());

    node->keys.resize(mid);
    node->children.resize(mid + 1);
    return true;
}

int BPlusTree::leftmostKey(BPlusNode* node) const {
    BPlusNode* cur = node;
    while (cur && !cur->isLeaf) {
        cur = cur->children[0];
    }
    return (cur && !cur->keys.empty()) ? cur->keys[0] : 0;
}

bool BPlusTree::remove(int id) {
    if (!root) return false;

    if (!removeRec(root, id)) {
        return false;
    }

    count--;
    if (root->keys.empty()) {
        if (root->isLeaf) {
            delete root;
            root = nullptr;
        } else {
            BPlusNode* oldRoot = root;
            root = root->children[0];
            delete oldRoot;
        }
    }
    return true;
}

bool BPlusTree::removeRec(BPlusNode* node, int id) {
    if (node->isLeaf) {
        auto it = std::lower_bound(node->keys.begin(), node->keys.end(), id);
        if (it == node->keys.end() || *it != id) {
            return false;
        }
        size_t idx = static_cast<size_t>(it - node->keys.begin());
        node->keys.erase(node->keys.begin() + idx);
        node->records.erase(node->records.begin() + idx);
        return true;
    }

    size_t idx = 0;
    while (idx < node->keys.size() && id >= node->keys[idx]) {
        idx++;
    }

    if (!removeRec(node->children[idx], id)) {
        return false;
    }

    if (static_cast<int>(node->children[idx]->keys.size()) < MIN_KEYS) {
        fixChildUnderflow(node, idx);
    }

    // Refresh any separator matching the deleted ID
    for (size_t i = 0; i < node->keys.size(); ++i) {
        if (node->keys[i] == id) {
            node->keys[i] = leftmostKey(node->children[i + 1]);
        }
    }

    return true;
}

void BPlusTree::fixChildUnderflow(BPlusNode* parent, size_t idx) {
    BPlusNode* child = parent->children[idx];

    // Try borrowing from left sibling
    if (idx > 0 && static_cast<int>(parent->children[idx - 1]->keys.size()) > MIN_KEYS) {
        BPlusNode* left = parent->children[idx - 1];
        if (child->isLeaf) {
            child->keys.insert(child->keys.begin(), left->keys.back());
            child->records.insert(child->records.begin(), left->records.back());
            left->keys.pop_back();
            left->records.pop_back();
            parent->keys[idx - 1] = child->keys[0];
        } else {
            child->keys.insert(child->keys.begin(), parent->keys[idx - 1]);
            parent->keys[idx - 1] = left->keys.back();
            left->keys.pop_back();
            child->children.insert(child->children.begin(), left->children.back());
            left->children.pop_back();
        }
        return;
    }

    // Try borrowing from right sibling
    if (idx + 1 < parent->children.size() &&
        static_cast<int>(parent->children[idx + 1]->keys.size()) > MIN_KEYS) {
        BPlusNode* right = parent->children[idx + 1];
        if (child->isLeaf) {
            child->keys.push_back(right->keys.front());
            child->records.push_back(right->records.front());
            right->keys.erase(right->keys.begin());
            right->records.erase(right->records.begin());
            parent->keys[idx] = right->keys[0];
        } else {
            child->keys.push_back(parent->keys[idx]);
            parent->keys[idx] = right->keys.front();
            right->keys.erase(right->keys.begin());
            child->children.push_back(right->children.front());
            right->children.erase(right->children.begin());
        }
        return;
    }

    // Merge with left sibling if possible, otherwise right sibling
    if (idx > 0) {
        BPlusNode* left = parent->children[idx - 1];
        if (child->isLeaf) {
            left->keys.insert(left->keys.end(), child->keys.begin(), child->keys.end());
            left->records.insert(left->records.end(), child->records.begin(), child->records.end());
            left->next = child->next;
        } else {
            left->keys.push_back(parent->keys[idx - 1]);
            left->keys.insert(left->keys.end(), child->keys.begin(), child->keys.end());
            left->children.insert(left->children.end(), child->children.begin(), child->children.end());
        }
        parent->keys.erase(parent->keys.begin() + idx - 1);
        parent->children.erase(parent->children.begin() + idx);
        delete child;
    } else {
        BPlusNode* right = parent->children[idx + 1];
        if (child->isLeaf) {
            child->keys.insert(child->keys.end(), right->keys.begin(), right->keys.end());
            child->records.insert(child->records.end(), right->records.begin(), right->records.end());
            child->next = right->next;
        } else {
            child->keys.push_back(parent->keys[idx]);
            child->keys.insert(child->keys.end(), right->keys.begin(), right->keys.end());
            child->children.insert(child->children.end(), right->children.begin(), right->children.end());
        }
        parent->keys.erase(parent->keys.begin() + idx);
        parent->children.erase(parent->children.begin() + idx + 1);
        delete right;
    }
}

void BPlusTree::getAllSorted(std::vector<Student>& out) const {
    out.clear();
    if (!root) return;

    BPlusNode* cur = root;
    while (!cur->isLeaf) {
        cur = cur->children[0];
    }
    while (cur) {
        for (const Student& s : cur->records) {
            out.push_back(s);
        }
        cur = cur->next;
    }
}

void BPlusTree::rangeQuery(int low, int high, std::vector<Student>& out) const {
    out.clear();
    if (!root || low > high) return;

    BPlusNode* cur = findLeaf(low);
    while (cur) {
        for (size_t i = 0; i < cur->keys.size(); ++i) {
            if (cur->keys[i] > high) {
                return;
            }
            if (cur->keys[i] >= low) {
                out.push_back(cur->records[i]);
            }
        }
        cur = cur->next;
    }
}

void BPlusTree::filterByDept(const std::string& dept, std::vector<Student>& out) const {
    out.clear();
    if (!root) return;

    std::string target = upperDept(dept);
    BPlusNode* cur = root;
    while (!cur->isLeaf) {
        cur = cur->children[0];
    }
    while (cur) {
        for (const Student& s : cur->records) {
            if (upperDept(s.dept) == target) {
                out.push_back(s);
            }
        }
        cur = cur->next;
    }
}

void BPlusTree::filterTopCgpa(double threshold, std::vector<Student>& out) const {
    out.clear();
    if (!root) return;

    BPlusNode* cur = root;
    while (!cur->isLeaf) {
        cur = cur->children[0];
    }
    while (cur) {
        for (const Student& s : cur->records) {
            if (s.cgpa > threshold) {
                out.push_back(s);
            }
        }
        cur = cur->next;
    }
}

void BPlusTree::printStructure() const {
    if (!root) {
        std::cout << "(empty B+ tree)\n";
        return;
    }

    std::cout << "B+ Tree (order " << (MAX_KEYS + 1)
              << ", records: " << count
              << ", height: " << getHeight() << ")\n";

    std::queue<BPlusNode*> q;
    q.push(root);
    int level = 0;

    while (!q.empty()) {
        size_t levelSize = q.size();
        std::cout << "L" << level << ": ";
        for (size_t n = 0; n < levelSize; ++n) {
            BPlusNode* cur = q.front();
            q.pop();

            std::cout << "[";
            for (size_t k = 0; k < cur->keys.size(); ++k) {
                if (k > 0) std::cout << "|";
                std::cout << cur->keys[k];
            }
            std::cout << "]";

            if (cur->isLeaf && cur->next) {
                std::cout << "->";
            } else if (n + 1 < levelSize) {
                std::cout << "  ";
            }

            if (!cur->isLeaf) {
                for (BPlusNode* ch : cur->children) {
                    q.push(ch);
                }
            }
        }
        std::cout << "\n";
        level++;
    }
}

int BPlusTree::size() const {
    return count;
}

int BPlusTree::getHeight() const {
    if (!root) return 0;
    int h = 1;
    BPlusNode* cur = root;
    while (!cur->isLeaf) {
        cur = cur->children[0];
        h++;
    }
    return h;
}
