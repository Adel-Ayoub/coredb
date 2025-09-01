#include "../../include/coredb.h"

// Read a B-Tree node from disk
void read_node(Database *db, off_t offset, BTreeNode *node)
{
    unsigned char buffer[PAGE_SIZE];
    fseek(db->file, offset, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, PAGE_SIZE, db->file);
    if (bytes_read != PAGE_SIZE)
    {
        printf("Error: Failed to read node at offset %lld\n", (long long)offset);
        exit(1);
    }
    memcpy(node, buffer, sizeof(BTreeNode));
}

// Write a B-Tree node to disk
void write_node(Database *db, off_t offset, BTreeNode *node)
{
    unsigned char buffer[PAGE_SIZE];
    memset(buffer, 0, PAGE_SIZE);
    memcpy(buffer, node, sizeof(BTreeNode));

    fseek(db->file, offset, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, PAGE_SIZE, db->file);
    if (bytes_written != PAGE_SIZE)
    {
        printf("Error: Failed to write node at offset %lld\n", (long long)offset);
        exit(1);
    }
    fflush(db->file);
}

// Allocate a new node (find a free page in the index section)
off_t allocate_node(Database *db)
{
    // Simple sequential allocation in the index section
    // We'll use a simple approach: allocate nodes sequentially
    // starting after the header and root pages
    static off_t next_offset = PAGE_SIZE * 2; // header(0) + root(PAGE_SIZE)
    
    // Check if we have space in the index section
    if (next_offset >= DATA_START_OFFSET)
    {
        printf("Error: Index section full (tried to allocate at offset %lld, max is %lld)\n", 
               (long long)next_offset, (long long)DATA_START_OFFSET);
        return -1; // Indicate failure
    }
    
    off_t new_offset = next_offset;
    next_offset += PAGE_SIZE;
    
    // Initialize the new node with zeros
    BTreeNode new_node = {0};
    write_node(db, new_offset, &new_node);
    
    return new_offset;
}

// Search the B-Tree for an ID, return its address
void btree_search(Database *db, int id, off_t *address)
{
    BTreeNode node;
    off_t current_offset = db->root_offset;

    while (1)
    {
        read_node(db, current_offset, &node);
        if (node.is_leaf)
        {
            // Search in leaf node
            for (int i = 0; i < node.num_keys; i++)
            {
                if (node.data.leaf.entries[i].id == id)
                {
                    *address = node.data.leaf.entries[i].address;
                    return;
                }
            }
            *address = -1; // Not found
            return;
        }
        else
        {
            // Search in internal node
            int i;
            for (i = 0; i < node.num_keys; i++)
            {
                if (id < node.data.internal.keys[i])
                {
                    break;
                }
            }
            current_offset = node.data.internal.children[i];
        }
    }
}

// Insert into the B-Tree
void btree_insert(Database *db, int id, off_t address)
{
    BTreeNode root;
    read_node(db, db->root_offset, &root);

    // If root is full, split it and create a new root
    if (root.num_keys >= MAX_KEYS)
    {
        off_t old_root_offset = db->root_offset;
        off_t new_root_offset = allocate_node(db);
        off_t right_offset = allocate_node(db);

        // Check if we have space for new nodes
        if (new_root_offset == -1 || right_offset == -1)
        {
            printf("Error: Cannot split B-tree root - index section full\n");
            return; // Exit early if we can't allocate new nodes
        }

        BTreeNode new_root = (BTreeNode){0};
        BTreeNode right = (BTreeNode){0};
        new_root.is_leaf = 0;
        right.is_leaf = root.is_leaf;
        new_root.num_keys = 0;
        right.num_keys = 0;

        // Split the old root
        int mid = MAX_KEYS / 2;
        int mid_key = root.is_leaf ? root.data.leaf.entries[mid].id : root.data.internal.keys[mid];

        // Move second half to right node
        for (int i = mid + (root.is_leaf ? 0 : 1); i < root.num_keys; i++)
        {
            if (root.is_leaf)
            {
                right.data.leaf.entries[right.num_keys] = root.data.leaf.entries[i];
            }
            else
            {
                right.data.internal.keys[right.num_keys] = root.data.internal.keys[i];
                right.data.internal.children[right.num_keys] = root.data.internal.children[i];
            }
            right.num_keys++;
        }
        if (!root.is_leaf)
        {
            right.data.internal.children[right.num_keys] = root.data.internal.children[root.num_keys];
        }
        root.num_keys = mid;

        // Update new root
        new_root.data.internal.keys[0] = mid_key;
        new_root.data.internal.children[0] = old_root_offset;
        new_root.data.internal.children[1] = right_offset;
        new_root.num_keys = 1;

        // Write nodes
        write_node(db, old_root_offset, &root);
        write_node(db, right_offset, &right);
        write_node(db, new_root_offset, &new_root);
        db->root_offset = new_root_offset;
    }

    // Now insert into the appropriate node
    off_t current_offset = db->root_offset;
    while (1)
    {
        read_node(db, current_offset, &root);
        if (root.is_leaf)
        {
            // Insert into leaf
            if (root.num_keys >= MAX_KEYS)
            {
                printf("Error: Leaf overflow before insert (keys=%d)\n", root.num_keys);
                exit(1);
            }
            int i;
            for (i = root.num_keys; i > 0 && root.data.leaf.entries[i - 1].id > id; i--)
            {
                root.data.leaf.entries[i] = root.data.leaf.entries[i - 1];
            }
            root.data.leaf.entries[i].id = id;
            root.data.leaf.entries[i].address = address;
            root.num_keys++;
            write_node(db, current_offset, &root);
            break;
        }
        else
        {
            // Find the child to descend into
            int i;
            for (i = 0; i < root.num_keys; i++)
            {
                if (id < root.data.internal.keys[i])
                {
                    break;
                }
            }

            // Before descending, check if the target child (leaf) is full; if so, split it
            off_t child_offset = root.data.internal.children[i];
            BTreeNode child;
            read_node(db, child_offset, &child);
            if (child.is_leaf && child.num_keys >= MAX_KEYS)
            {
                // Split leaf child
                int mid = child.num_keys / 2;
                int pivot = child.data.leaf.entries[mid].id;

                off_t right_offset = allocate_node(db);
                if (right_offset == -1)
                {
                    printf("Error: Cannot split leaf - index section full\n");
                    return;
                }
                BTreeNode right = (BTreeNode){0};
                right.is_leaf = 1;

                // Move second half to right leaf
                int k = 0;
                for (int j = mid; j < child.num_keys; j++)
                {
                    right.data.leaf.entries[k++] = child.data.leaf.entries[j];
                }
                right.num_keys = k;
                child.num_keys = mid;

                // Insert pivot and new right child into parent at position i
                for (int j = root.num_keys; j > i; j--)
                {
                    root.data.internal.keys[j] = root.data.internal.keys[j - 1];
                    root.data.internal.children[j + 1] = root.data.internal.children[j];
                }
                root.data.internal.keys[i] = pivot;
                root.data.internal.children[i + 1] = right_offset;
                root.num_keys++;

                // Persist nodes
                write_node(db, child_offset, &child);
                write_node(db, right_offset, &right);
                write_node(db, current_offset, &root);

                // Decide which side to descend
                if (id < pivot)
                {
                    current_offset = child_offset;
                }
                else
                {
                    current_offset = right_offset;
                }
                continue;
            }

            // Descend as usual
            current_offset = child_offset;
            // Check if child needs splitting (simplified, recurse if needed)
        }
    }

    // Update root_offset in file
    fseek(db->file, 0, SEEK_SET);
    fwrite(&db->root_offset, sizeof(off_t), 1, db->file);
}

// Delete from the B-Tree (simplified, no rebalancing)
void btree_delete(Database *db, int id)
{
    BTreeNode node;
    off_t current_offset = db->root_offset;
    off_t parent_offset = -1;
    int child_index = -1;

    while (1)
    {
        read_node(db, current_offset, &node);
        if (node.is_leaf)
        {
            // Delete from leaf
            int i;
            for (i = 0; i < node.num_keys; i++)
            {
                if (node.data.leaf.entries[i].id == id)
                {
                    break;
                }
            }
            if (i == node.num_keys)
            {
                return; // Not found
            }
            // Shift entries
            for (int j = i; j < node.num_keys - 1; j++)
            {
                node.data.leaf.entries[j] = node.data.leaf.entries[j + 1];
            }
            node.num_keys--;
            write_node(db, current_offset, &node);
            break;
        }
        else
        {
            // Find child to descend into
            int i;
            for (i = 0; i < node.num_keys; i++)
            {
                if (id < node.data.internal.keys[i])
                {
                    break;
                }
            }
            parent_offset = current_offset;
            child_index = i;
            current_offset = node.data.internal.children[i];
        }
    }

    // Update parent key if necessary (simplified)
    if (parent_offset != -1 && node.num_keys > 0)
    {
        BTreeNode parent;
        read_node(db, parent_offset, &parent);
        parent.data.internal.keys[child_index] = node.data.leaf.entries[0].id;
        write_node(db, parent_offset, &parent);
    }
}
