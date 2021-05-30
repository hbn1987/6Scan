// Space tree node
struct SpaceTreeNode
{
    // Lower bound index
    int lower = 0;
    // Upper bound index
    int upper = 0;
    // Parent node pointer
    struct SpaceTreeNode *parent = NULL;
    // Child node pointers
    struct SpaceTreeNode **children = NULL;
    // Number of child nodes
    int children_num = 0;
    // Subspace
    std::string subspace;
    // Number of variable dimensions
    int dim_num = 0;

    // Number of active addresses in this dimension
    int active = 0;
    // The subspace of the node's grandfather
    std::string grandfather;
    // The index of sibling node which is to be scanned
    uint64_t index = 0;
};

typedef std::vector<struct SpaceTreeNode*> Node_List;

struct Node_Dim_Cmp {
    bool operator() (struct SpaceTreeNode* p1, struct SpaceTreeNode* p2) {
        return p1->dim_num < p2->dim_num;
    }
};

struct Node_Active_Cmp {
    bool operator() (struct SpaceTreeNode* p1, struct SpaceTreeNode* p2) {
        return p1->active > p2->active;
    }
};

void tree_generation(Node_List&, std::vector<std::string>&);

void tree_generation_6scan(Node_List&, std::vector<std::string>&);