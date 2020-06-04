/**
 * @file    main.c
 * \mainpage HW04 Documentation
 *
 *
 * On the 'Files' page, there is a list of documented files with brief descriptions.
 *
*/

#include "graph.h"
#include "heap.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int run(char *, char *, char *, char *, char *);

void release(FILE *, FILE *, Graph *, Heap *);

bool loadFile_nodes(FILE *, Graph *);

bool loadFile_edges(FILE *, Graph *);

int countChars(char *, char);

Heap *findPath(Graph *, unsigned int, Node *);

void printPath(unsigned int, Node *, FILE *);

int main(int argc, char **argv)
{
    switch (argc) {
    case 5:
        return run(argv[1], argv[2], argv[3], argv[4], NULL);
    case 6:
        return run(argv[1], argv[2], argv[3], argv[4], argv[5]);
    default:
        fprintf(stderr, "Invalid number of parameters [%d].\n", argc - 1);
        return 1;
    }
}

int run(char *nodesFile, char *edgesFile, char *srcNode, char *dstNode, char *outputFile)
{
    FILE *nodes = NULL;

    if (!(nodes = fopen(nodesFile, "r"))) {
        fprintf(stderr, "Cannot open nodes file '%s'.\n", nodesFile);
        return 1;
    }

    FILE *edges = NULL;

    if (!(edges = fopen(edgesFile, "r"))) {
        release(nodes, NULL, NULL, NULL);
        fprintf(stderr, "Cannot open edges file '%s'.\n", edgesFile);
        return 1;
    }

    Graph *graph = graph_new();
    if (!graph) {
        release(nodes, edges, NULL, NULL);
        fprintf(stderr, "Unable to allocate memory.\n");
        return 1;
    }
    if (!loadFile_nodes(nodes, graph) || !loadFile_edges(edges, graph)) {
        release(nodes, edges, graph, NULL);
        fprintf(stderr, "Unable to allocate memory while loading nodes/edges.\n");
        return 1;
    }

    Node *dst_node = graph_get_node(graph, strtol(dstNode, NULL, 10));

    if (!dst_node) {
        fprintf(stderr, "Wrong node id.\n");
        release(nodes, edges, graph, NULL);
        return 1;
    }

    Heap *heap = findPath(graph, strtol(srcNode, NULL, 10), dst_node);
    if (!heap) {
        release(nodes, edges, graph, NULL);
        return 1;
    }

    if (node_get_distance(dst_node) == UINT_MAX) {
        fprintf(stderr, "Path between nodes doesn't exist.\n");
        release(nodes, edges, graph, heap);
        return 1;
    }

    if (!outputFile)
        printPath(strtol(srcNode, NULL, 10), dst_node, stdout);
    else {
        FILE *file = NULL;
        if (!(file = fopen(outputFile, "w"))) {
            fprintf(stderr, "Error occurred while creating file.\n");
            release(nodes, edges, graph, heap);
            return 1;
        }
        printPath(strtol(srcNode, NULL, 10), dst_node, file);
        fclose(file);
    }
    release(nodes, edges, graph, heap);
    return 0;
}

void release(FILE *nodes, FILE *edges, Graph *graph, Heap *heap)
{
    if (nodes)
        fclose(nodes);
    if (edges)
        fclose(edges);
    if (graph)
        graph_free(graph);
    if (heap)
        heap_free(heap);
}

bool loadFile_nodes(FILE *file, Graph *graph)
{
    char line[201] = "";
    for (;;) {
        if ((fgets(line, 200, file)) == NULL) {
            break;
        }
        if (countChars(line, ',') != 6) {
            fprintf(stderr, "Invalid number of commas in line.\n");
            return false;
        }
        char *token = strtok(line, ",");
        if (!graph_insert_node(graph, strtol(token, NULL, 10)))
            return false;
    }
    return true;
}

bool loadFile_edges(FILE *file, Graph *graph)
{
    unsigned int src = 0;
    unsigned int dst = 0;
    int mintime = 0;
    char line[201] = "";
    for (;;) {
        if ((fgets(line, 200, file)) == NULL) {
            break;
        }
        if (countChars(line, ',') != 6) {
            fprintf(stderr, "Invalid number of commas in line.\n");
            return false;
        }
        char *token = strtok(line, ",");
        src = strtol(token, NULL, 10);
        token = strtok(NULL, ",");
        dst = strtol(token, NULL, 10);
        strtok(NULL, ",");
        token = strtok(NULL, ",");
        mintime = strtol(token, NULL, 10);
        if (!graph_insert_edge(graph, src, dst, mintime))
            return false;
    }
    return true;
}

int countChars(char *s, char c)
{
    return *s == '\0' ? 0 : countChars(s + 1, c) + (*s == c);
}

Heap *findPath(Graph *graph, unsigned int id, Node *dst)
{
    Heap *heap = heap_new_from_graph(graph);
    if (!heap) {
        fprintf(stderr, "Cannot allocate new memory.\n");
        return NULL;
    }
    Node *src_node = graph_get_node(graph, id);
    if (!src_node) {
        heap_free(heap);
        fprintf(stderr, "Invalid source node id.\n");
        return NULL;
    }
    heap_decrease_distance(heap, src_node, 0, NULL);
    Node *node = NULL;
    Node *temp_node = NULL;
    unsigned int x = 0;

    while (!heap_is_empty(heap)) {
        node = heap_extract_min(heap);
        x = node_get_distance(node);
        struct edge *temp_edges = node_get_edges(node);
        unsigned int d = x;
        if (x == UINT_MAX)
            break;
        for (unsigned short i = 0; i < node_get_n_outgoing(node); i++) {
            temp_node = temp_edges[i].destination;
            x = d + temp_edges[i].mindelay;
            if (x < node_get_distance(temp_node)) {
                heap_decrease_distance(heap, temp_node, x, node);
            }
        }
        if (dst == node) {
            break;
        }
    }
    return heap;
}

void printPath(unsigned int id, Node *dst, FILE *output)
{
    fprintf(output, "digraph {\n");
    if (node_get_id(dst) != id) {
        Node *node = dst;
        Node *prev_node = node_get_previous(node);
        while (prev_node) {
            int distance = node_get_distance(node) - node_get_distance(prev_node);
            fprintf(output, "\t%u -> %u [label=%u];\n", node_get_id(prev_node), node_get_id(node), distance);
            node = prev_node;
            prev_node = node_get_previous(prev_node);
        }
    }
    fprintf(output, "}\n");
}

