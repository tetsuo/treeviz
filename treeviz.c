#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CHECK_ALLOC(ptr) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

struct tree {
    char *value;
    struct tree **children;
    size_t children_count;
};

struct forest {
    struct tree **trees;
    size_t count;
};

struct line_pair {
    int indent;
    char *str;
};

static struct forest forest_from_line_indents(struct line_pair *tuples, int count);
static struct tree *tree_from_text(const char *text);
static void draw(const char * const indentation, struct tree **forest, size_t count);
static void draw_tree(struct tree *root);
static void free_tree(struct tree *root);
static char *read_stdin(void);

/* Draw functions */
static void draw(const char * const indentation, struct tree **forest, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        struct tree *node = forest[i];
        int is_last = (i == count - 1);
        printf("%s%s─ %s\n", indentation, is_last ? "└" : "├", node->value);
        
        /* Dynamically allocate new indentation string */
        const char *branch_str = !is_last ? "│  " : "   ";
        size_t branch_len = strlen(branch_str);
        size_t new_indent_size = strlen(indentation) + branch_len + 1;
        char *new_indentation = malloc(new_indent_size);
        CHECK_ALLOC(new_indentation);

        if (snprintf(new_indentation, new_indent_size, "%s%s", indentation, branch_str) < 0) {
            fprintf(stderr, "snprintf failed\n");
            free(new_indentation);
            exit(EXIT_FAILURE);
        }
        
        draw(new_indentation, node->children, node->children_count);
        free(new_indentation);
    }
}

static void draw_tree(struct tree *root) {
    printf("%s\n", root->value);
    draw("", root->children, root->children_count);
}

/* Optimized recursive function to build forest from line indent pairs */
static struct forest forest_from_line_indents(struct line_pair *lines, int len) {
    struct forest result;
    result.count = 0;
    result.trees = malloc(sizeof(struct tree *) * 0);  // Start with allocation for 0 elements
    CHECK_ALLOC(result.trees);

    if (len <= 0) {
        return result;
    }

    int pos = 0;
    while (pos < len) {
        int current_indent = lines[pos].indent;
        char *current_str = lines[pos].str;
        
        int i = pos + 1;
        while (i < len && lines[i].indent > current_indent) {
            i++;
        }
        
        struct forest children = forest_from_line_indents(lines + pos + 1, i - (pos + 1));
        
        struct tree *node = malloc(sizeof(struct tree));
        CHECK_ALLOC(node);
        
        node->value = strdup(current_str);
        CHECK_ALLOC(node->value);
        
        node->children = children.trees;
        node->children_count = children.count;
        
        /* Incrementally resize result.trees to add new node */
        {
            struct tree **temp = realloc(result.trees, sizeof(struct tree *) * (result.count + 1));
            CHECK_ALLOC(temp);
            result.trees = temp;
        }
        result.trees[result.count++] = node;
        pos = i;
    }
    
    return result;
}

/* Parse text into a tree structure based on indentation */
static struct tree *tree_from_text(const char *text) {
    int lines_alloc = 100;
    int line_count = 0;
    struct line_pair *lines = malloc(sizeof(struct line_pair) * lines_alloc);
    CHECK_ALLOC(lines);

    char *text_copy = strdup(text);
    CHECK_ALLOC(text_copy);

    char *saveptr;
    char *line = strtok_r(text_copy, "\n", &saveptr);

    while (line) {
        int indent = 0;
        for (int i = 0; line[i] == ' ' || line[i] == '\t'; i++) {
            if (line[i] == '\t') {
                indent++;
            } else if (i % 2 == 0) {
                indent++;
            }
        }

        int non_space_idx = 0;
        while (isspace((unsigned char)line[non_space_idx])) {
            non_space_idx++;
        }
        char *trimmed = line + non_space_idx;

        lines[line_count].indent = indent;
        lines[line_count].str = strdup(trimmed);
        CHECK_ALLOC(lines[line_count].str);
        line_count++;

        if (line_count >= lines_alloc) {
            lines_alloc *= 2;
            struct line_pair *new_lines = realloc(lines, sizeof(struct line_pair) * lines_alloc);
            if (!new_lines) {
                fprintf(stderr, "memory allocation failed in tree_from_text\n");
                free(lines);
                free(text_copy);
                exit(EXIT_FAILURE);
            }
            lines = new_lines;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    free(text_copy);

    struct forest forest = forest_from_line_indents(lines, line_count);

    for (int i = 0; i < line_count; i++) {
        free(lines[i].str);
    }
    free(lines);

    struct tree *root = malloc(sizeof(struct tree));
    CHECK_ALLOC(root);
    
    root->value = strdup(".");
    CHECK_ALLOC(root->value);
    
    root->children = forest.trees;
    root->children_count = forest.count;
    return root;
}

/* Free tree memory recursively */
static void free_tree(struct tree *root) {
    if (!root) return;
    if (root->children) {
        for (size_t i = 0; i < root->children_count; ++i) {
            free_tree(root->children[i]);
        }
        free(root->children);
    }
    free(root->value);
    free(root);
}

/* Read all input from stdin until EOF and return it as a dynamically allocated string */
static char *read_stdin(void) {
    size_t size = 1024;
    size_t len = 0;
    char *buffer = malloc(size);
    CHECK_ALLOC(buffer);

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        buffer[len++] = (char)c;
        if (len == size) {
            size *= 2;
            char *new_buffer = realloc(buffer, size);
            if (!new_buffer) {
                fprintf(stderr, "memory allocation failed in read_stdin\n");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
    }
    buffer[len] = '\0';
    return buffer;
}

int main(void) {
    char *input_text = read_stdin();
    if (!input_text) {
        fprintf(stderr, "error reading input.\n");
        return EXIT_FAILURE;
    }

    struct tree *root = tree_from_text(input_text);
    free(input_text);

    draw_tree(root);
    free_tree(root);

    return EXIT_SUCCESS;
}
