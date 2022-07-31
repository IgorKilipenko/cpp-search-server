#include <cassert>

template <typename T>
struct TreeNode {
    T value;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
};

template <typename T>
void DeleteTree(TreeNode<T>* node) {
    if (!node) {
        return;
    }
    DeleteTree(node->left);
    DeleteTree(node->right);
    delete node;
}
/*
template <typename T>
bool CheckTreeProperty(const TreeNode<T>* node) {
    if (!node->left && !node->right) {
        return true;
    }
    if (node->left && node->value < node->left->value) {
        return false;
    }
    if (node->right && node->right->value < node->value) {
        return false;
    }
    if (node->left && node->right && node->right->value < node->left->value) {
        return false;
    }

    bool reult = CheckTreeProperty(node->left);
    reult = reult && CheckTreeProperty(node->right);

    return reult;
}*/

template <typename T>
bool CheckTreeProperty(const TreeNode<T>* node, const T* min, const T* max) {
    if (min && node->value < *min) {
        return false;
    }
    if (max && *max < node->value) {
        return false;
    }
    if (!node->left && !node->right) {
        return true;
    }
    bool result = node->left == nullptr;
    result = !result &&
             CheckTreeProperty(
                 node->left, node->left->left ? &(node->left->left->value) : nullptr, node->left->right ? &(node->left->right->value) : nullptr);
    
    if (!result) {
        return false;
    }

    result = node->right == nullptr;
    result = !result &&
             CheckTreeProperty(
                 node->right, node->right->left ? &(node->right->left->value) : nullptr, node->right->right ? &(node->right->right->value) : nullptr);
    return result;
}

template <typename T>
bool CheckTreeProperty(const TreeNode<T>* node) {
    return CheckTreeProperty<T>(node, nullptr, nullptr);
}

int main() {
    using T = TreeNode<int>;
    T* root1 = new T{6, new T{4, new T{3}, new T{5}}, new T{7}};
    assert(CheckTreeProperty(root1));

    T* root2 = new T{6, new T{4, new T{3}, new T{5}}, new T{7, new T{8}}};
    assert(!CheckTreeProperty(root2));

    DeleteTree(root1);
    DeleteTree(root2);
}