/**
 * @file    main.c
 * \mainpage HW04 Documentation
 *
 *
 * On the 'Files' page, there is a list of documented files with brief descriptions.
 *
*/

#include "graph.h"
#include "graph-private.h"
#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

int run(char *nodesFile, char *edgesFile, char *sourceNode, char *destNode, char *outputFile)
{
    FILE *nodes = NULL;
    FILE *edges = NULL;

    if (!(nodes = fopen(nodesFile, "r"))) {
        fprintf(stderr, "Cannot open nodes file '%s'. No such file or directory.\n", nodesFile);
        return 1;
    }
    if (!(edges = fopen(edgesFile, "r"))) {
        release(nodes, NULL, NULL, NULL);
        fprintf(stderr, "Cannot open edges file '%s'. No such file or directory.\n", edgesFile);
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

    Node *dest_node = graph_get_node(graph, strtol(destNode, NULL, 10));

    if (!dest_node) {
        fprintf(stderr, "Invalid destination node id.\n");
        release(nodes, edges, graph, NULL);
        return 1;
    }

    Heap *heap = findPath(graph, strtol(sourceNode, NULL, 10), dest_node);
    if (!heap) {
        release(nodes, edges, graph, NULL);
        return 1;
    }

    if (node_get_distance(dest_node) == UINT_MAX) {
        fprintf(stderr, "No path exists between these two nodes.\n");
        release(nodes, edges, graph, heap);
        return 1;
    }

    bool error = false;

    if (!outputFile)
        printPath(strtol(sourceNode, NULL, 10), dest_node, stdout);
    else {
        FILE *file = NULL;
        if (!(file = fopen(outputFile, "w"))) {
            fprintf(stderr, "Cannot create new file to print data in.\n");
            error = true;
        }
        printPath(strtol(sourceNode, NULL, 10), dest_node, file);
        fclose(file);
    }
    release(nodes, edges, graph, heap);
    if (error)
        return 1;
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
    unsigned int source = 0;
    unsigned int dest = 0;
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
        source = strtol(token, NULL, 10);
        token = strtok(NULL, ",");
        dest = strtol(token, NULL, 10);
        strtok(NULL, ",");
        token = strtok(NULL, ",");
        mintime = strtol(token, NULL, 10);
        if (!graph_insert_edge(graph, source, dest, mintime))
            return false;
    }
    return true;
}

int countChars(char *s, char c)
{
    return *s == '\0' ? 0 : countChars(s + 1, c) + (*s == c);
}

Heap *findPath(Graph *graph, unsigned int id, Node *dest)
{
    Heap *heap = heap_new_from_graph(graph);
    if (!heap) {
        fprintf(stderr, "Cannot allocate new memory.\n");
        return NULL;
    }
    Node *source_node = graph_get_node(graph, id);
    if (!source_node) {
        heap_free(heap);
        fprintf(stderr, "Invalid source node id.\n");
        return NULL;
    }
    heap_decrease_distance(heap, source_node, 0, NULL);
    Node *node = NULL;
    Node *temp = NULL;
    unsigned int alt = 0;

    while (!heap_is_empty(heap)) {
        node = heap_extract_min(heap);
        alt = node_get_distance(node);
        struct edge *temp_edges = node_get_edges(node);
        unsigned int d = alt;
        if (alt == UINT_MAX)
            break;
        for (unsigned short i = 0; i < node_get_n_outgoing(node); i++) {
            temp = temp_edges[i].destination;
            alt = d + temp_edges[i].mindelay;
            if (alt < node_get_distance(temp)) {
                heap_decrease_distance(heap, temp, alt, node);
            }
        }
        if (node == dest) {
            return heap;
        }
    }
    return heap;
}

void printPath(unsigned int id, Node *dest, FILE *output)
{
    fprintf(output, "digraph {\n");
    if (id != node_get_id(dest)) {
        Node *temp_node = dest;
        Node *temp_prev = node_get_previous(temp_node);
        while (temp_prev) {
            int distance = node_get_distance(temp_node) - node_get_distance(temp_prev);
            fprintf(output, "\t%u -> %u [label=%u];\n", node_get_id(temp_prev), node_get_id(temp_node), distance);
            temp_node = temp_prev;
            temp_prev = node_get_previous(temp_prev);
        }
    }
    fprintf(output, "}\n");
}

