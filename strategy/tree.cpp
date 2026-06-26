/****************************************************************************
 * Copyright (c) 2021 Bingnan Hou <houbingnan19@nudt.edu.cn> all rights reserved.
 ***************************************************************************/
#include "strategy.h"

void init_subspace(struct SpaceTreeNode *node, std::vector<std::string>& active_seeds)
{
    // Initialize the subspace
    node->subspace = active_seeds[node->lower];

    int lower = node->lower;
    int upper = node->upper;

    int *dimension_flag = new int [32];
    for (int i = 0; i < 32; i++)
    {
        dimension_flag[i] = 0;
    }

    for (int i = lower+1; i < upper; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (node->subspace[j] != active_seeds[i][j])
                dimension_flag[j] = 1;
        }
    }

    // Update the subspace
    for (int i = 0; i < 32; i++)
    {
        if (dimension_flag[i])
        {
            node->dim_num++;
            node->subspace[i] = '*';
        }
    }
    delete [] dimension_flag;
}

void Strategy::DHC(struct SpaceTreeNode *node, std::vector<std::string>& active_seeds, Node_List& node_list)
{
    // int node_vector_num = node->upper - node->lower + 1;
    if (node->dim_num <= config->dimension - 4){
        return ;
    }
    int var_dimen = 0;
    int lower = node->lower;
    int upper = node->upper;
    for (int i = 0; i < 32; i++)
    {
        bool find = false;
        for (int j = lower; j < upper; j++)
        {
            if (active_seeds[j][i] != active_seeds[j + 1][i])
            {
                find = true;
                var_dimen = i;
                break;
            }
        }
        if (find == true)
        {
            break;
        }
    }

    int children_num = 1;
    for (int i = lower; i < upper; i++)
    {
        if (active_seeds[i][var_dimen] != active_seeds[i + 1][var_dimen])
        {
            children_num++;
        }
    }
    node->children = new SpaceTreeNode *[children_num];
    node->children_num = children_num;
    int step = 0;
    node->children[step] = new SpaceTreeNode;
    node->children[step]->parent = node;
    node->children[step]->children = NULL;
    node->children[step]->children_num = 0;
    node->children[step]->lower = lower;
    for (int i = lower; i < upper; i++)
    {
        if (active_seeds[i][var_dimen] != active_seeds[i + 1][var_dimen])
        {
            node->children[step++]->upper = i;
            node->children[step] = new SpaceTreeNode;
            node->children[step]->parent = node;
            node->children[step]->children = NULL;
            node->children[step]->children_num = 0;
            node->children[step]->lower = i + 1;
        }
    }
    node->children[step]->upper = upper;
    for (int i = 0; i < children_num; i++)
    {
        init_subspace(node->children[i], active_seeds);
        if (node->children[i]->dim_num)
            node_list.push_back(node->children[i]);
        DHC(node->children[i], active_seeds, node_list);
    }
}

void DHC_6hit(struct SpaceTreeNode *node, std::vector<std::string>& active_seeds, Node_List& node_list, int subspace_dim) {
    if (node->dim_num <= subspace_dim){
        return;
    }
    int var_dimen = 0;
    int lower = node->lower;
    int upper = node->upper;
    for (int i = 0; i < 32; i++)
    {
        bool find = false;
        for (int j = lower; j < upper; j++)
        {
            if (active_seeds[j][i] != active_seeds[j + 1][i])
            {
                find = true;
                var_dimen = i;
                break;
            }
        }
        if (find == true)
        {
            break;
        }
    }

    int children_num = 1;
    for (int i = lower; i < upper; i++)
    {
        if (active_seeds[i][var_dimen] != active_seeds[i + 1][var_dimen])
        {
            children_num++;
        }
    }
    node->children = new SpaceTreeNode *[children_num];
    node->children_num = children_num;
    int step = 0;
    node->children[step] = new SpaceTreeNode;
    node->children[step]->parent = node;
    node->children[step]->children = NULL;
    node->children[step]->children_num = 0;
    node->children[step]->lower = lower;
    for (int i = lower; i < upper; i++)
    {
        if (active_seeds[i][var_dimen] != active_seeds[i + 1][var_dimen])
        {
            node->children[step++]->upper = i;
            node->children[step] = new SpaceTreeNode;
            node->children[step]->parent = node;
            node->children[step]->children = NULL;
            node->children[step]->children_num = 0;
            node->children[step]->lower = i + 1;
        }
    }
    node->children[step]->upper = upper;
    for (int i = 0; i < children_num; i++)
    {
        init_subspace(node->children[i], active_seeds);
        if (node->children[i]->dim_num && node->children[i]->dim_num <= subspace_dim)
            node_list.push_back(node->children[i]);
        DHC_6hit(node->children[i], active_seeds, node_list, subspace_dim);
    }
}

void Strategy::tree_generation(Node_List& node_list, std::vector<std::string>& active_seeds)
{
    // Space tree generation
    struct SpaceTreeNode *root = new struct SpaceTreeNode;
    root->lower = 0;
    root->upper = active_seeds.size() - 1;
    root->parent = NULL;
    root->children = NULL;
    root->children_num = 0;

    init_subspace(root, active_seeds);
    node_list.push_back(root);
    DHC(root, active_seeds, node_list);
}

void Strategy::tree_generation_6hit(Node_List& node_list, std::vector<std::string>& active_seeds)
{
    // Space tree generation
    struct SpaceTreeNode *root = new struct SpaceTreeNode;
    root->lower = 0;
    root->upper = active_seeds.size() - 1;
    root->parent = NULL;
    root->children = NULL;
    root->children_num = 0;

    init_subspace(root, active_seeds);
    int subspace_dim = config->dimension - 2;
    DHC_6hit(root, active_seeds, node_list, subspace_dim);
}

void Strategy::tree_generation_hmap6(Node_List& node_list, std::vector<std::string>& active_seeds)
{
    // Space tree generation
    struct SpaceTreeNode *root = new struct SpaceTreeNode;
    root->lower = 0;
    root->upper = active_seeds.size() - 1;
    root->parent = NULL;
    root->children = NULL;
    root->children_num = 0;

    init_subspace(root, active_seeds);
    int subspace_dim = config->dimension;
    DHC_6hit(root, active_seeds, node_list, subspace_dim);
}

void release_tree(struct SpaceTreeNode *node)
{
    int children_num = node->children_num;
    for (int i = 0; i < children_num; i++)
    {
        release_tree(node->children[i]);
    }
    if (children_num != 0)
    {
        delete [] node->children;
    }
    delete node;
}