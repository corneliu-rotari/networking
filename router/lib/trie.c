// #include "router.h"

// typedef struct trieNode {
//     uint8_t value;
//     RTable_entry *entry;
//     struct trieNode** next;
//     int next_count; 
// } *Trie_Node;

// typedef struct trie
// {
//     Trie_Node root;
// } *Trie;

// Trie_Node create_node(RTable_entry *entry, uint8_t value) {
//     Trie_Node new_node = malloc(sizeof(struct trieNode));
//     new_node->value = value;
//     if (entry != NULL)
//         new_node->entry = NULL;
    
//     new_node->next_count = 255;
//     new_node->next = calloc(255, sizeof(struct trieNode*));

// }

// Trie create_trie() {
//     Trie tree = malloc(sizeof(struct trie));
//     tree->root = NULL;
//     return tree;
// }

// Trie_Node search_node(Trie tree, uint32_t value) {
//     if (tree == NULL || tree->root)
//         return NULL;
    
//     Trie_Node iter = tree->root;
//     int j = 0;
//     while (iter->entry == NULL && j < 4) {
//         uint8_t subpart = *((uint8_t*)(&value + j));
//         for (int i = 0; i < iter->next_count; i++)
//         {
//             if (iter->next[i] != NULL && iter->next[i]->value == subpart) {
//                 iter = iter->next[i];
//                 break;
//             }
//         }
//     }

//     for (int i = 0; i < iter->next; i++)
//     {
//         /* code */
//     }
    
    
//     return NULL;
    
// }

// void add_to_trie(Trie tree, RTable_entry* entry) {
//     if (tree == NULL)
//         return;
    
//     // uint8_t subpart = *((uint8_t*)(entry->prefix));
//     if(tree->root == NULL) {
//         Trie_Node *previous = &tree->root;
//         for (int i = 0; i < 4; i++)
//         {
//             uint8_t subpart = *((uint8_t*)(entry->prefix + i));
//             *previous = create_node(NULL, subpart);
//             previous = &((*previous)->next);
//         }
        

//     }
//     Trie_Node iter = 
//     while ()
//     {
//         /* code */
//     }
    

// }