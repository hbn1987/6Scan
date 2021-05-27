// Space tree node
struct SpaceTreeNode
{
    // Lower bound index
    int lower;
    // Upper bound index
    int upper;
    // Parent node pointer
    struct SpaceTreeNode *parent;
    // Child node pointers
    struct SpaceTreeNode **children;
    // Number of child nodes
    int children_num = 0;
    // Subspace
    std::string subspace;
    // Number of variable dimensions
    int dim_num = 0;
    // Number of active addresses in this dimension
    int active = 0;
    // The subspace of the node's ancestor
    std::string ancestor;
};

typedef std::vector<struct SpaceTreeNode*> Node_List;

void tree_generation(Node_List& nodelist, std::vector<std::string>& active_seeds);

void tree_generation_6scan(Node_List& nodelist, std::vector<std::string>& active_seeds);

void release_tree(struct SpaceTreeNode *node);