/*
 Emergency Resources Allocation System - Data Structures in C
 Features: linked-list resources, priority queue (array), weighted graph,
 Dijkstra shortest path, FCFS for equal priority, text-file persistence.
 Compile: gcc emergency_ds.c -o emergency_ds
 Run:     ./emergency_ds
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_LOCATIONS 20
#define MAX_NAME 60
#define MAX_EMERGENCIES 100
#define INF (INT_MAX / 4)

typedef struct {
    int id;
    char name[MAX_NAME];
    int location;
    int available;
    char type[24];
} Resource;

typedef struct ResourceNode {
    Resource data;
    struct ResourceNode *next;
} ResourceNode;

typedef struct {
    int id;
    char description[MAX_NAME];
    int location;
    int priority;       /* 3 = High, 2 = Medium, 1 = Low */
    unsigned long long arrivalOrder;
    int assignedResourceId;
    int active;
} Emergency;

typedef struct {
    Emergency items[MAX_EMERGENCIES];
    int count;
    unsigned long long nextOrder;
    int nextId;
} EmergencyQueue;

typedef struct {
    int count;
    char names[MAX_LOCATIONS][MAX_NAME];
    int weights[MAX_LOCATIONS][MAX_LOCATIONS];
} Graph;

static ResourceNode *resources = NULL;
static EmergencyQueue queueData = { .count = 0, .nextOrder = 0, .nextId = 1 };
static Graph roadGraph;
static int nextResourceId = 1;

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

int readInt(const char *prompt, int minValue, int maxValue) {
    char line[80], *end;
    long value;
    for (;;) {
        readLine(prompt, line, sizeof(line));
        value = strtol(line, &end, 10);
        if (end != line && *end == '\0' && value >= minValue && value <= maxValue)
            return (int)value;
        printf("Enter a number from %d to %d.\n", minValue, maxValue);
    }
}

void initGraph(Graph *g) {
    int i, j;
    g->count = 0;
    for (i = 0; i < MAX_LOCATIONS; i++) {
        for (j = 0; j < MAX_LOCATIONS; j++)
            g->weights[i][j] = (i == j) ? 0 : INF;
    }
}

int findLocation(const Graph *g, const char *name) {
    int i;
    for (i = 0; i < g->count; i++)
        if (strcmp(g->names[i], name) == 0) return i;
    return -1;
}

int addLocation(Graph *g, const char *name) {
    int found = findLocation(g, name);
    if (found >= 0) return found;
    if (g->count >= MAX_LOCATIONS) {
        printf("Location limit reached.\n");
        return -1;
    }
    strncpy(g->names[g->count], name, MAX_NAME - 1);
    g->names[g->count][MAX_NAME - 1] = '\0';
    return g->count++;
}

void addRoad(void) {
    char a[MAX_NAME], b[MAX_NAME];
    int distance, u, v;
    if (roadGraph.count < 2) {
        printf("Add at least two locations first.\n");
        return;
    }
    printf("Available locations:\n");
    for (int i = 0; i < roadGraph.count; i++)
        printf("%d. %s\n", i, roadGraph.names[i]);
    readLine("From location name: ", a, sizeof(a));
    readLine("To location name: ", b, sizeof(b));
    u = findLocation(&roadGraph, a);
    v = findLocation(&roadGraph, b);
    if (u < 0 || v < 0 || u == v) {
        printf("Invalid locations. Names must match the list and be different.\n");
        return;
    }
    distance = readInt("Distance/weight (positive integer): ", 1, 1000000);
    roadGraph.weights[u][v] = distance;
    roadGraph.weights[v][u] = distance; /* undirected road */
    printf("Road added: %s <-> %s, weight %d.\n", a, b, distance);
}

void addLocationMenu(void) {
    char name[MAX_NAME];
    int idx;
    readLine("New location name: ", name, sizeof(name));
    if (name[0] == '\0') {
        printf("Location name cannot be empty.\n");
        return;
    }
    idx = addLocation(&roadGraph, name);
    if (idx >= 0) printf("Location '%s' is available with ID %d.\n", name, idx);
}

void listLocations(void) {
    if (roadGraph.count == 0) {
        printf("No locations added.\n");
        return;
    }
    printf("\nLocations:\n");
    for (int i = 0; i < roadGraph.count; i++)
        printf("  %d: %s\n", i, roadGraph.names[i]);
}

void addResource(void) {
    ResourceNode *node;
    char name[MAX_NAME];
    int typeChoice, location;
    Resource r;

    if (roadGraph.count == 0) {
        printf("Add locations before adding resources.\n");
        return;
    }
    readLine("Resource name (e.g. Ambulance 1): ", name, sizeof(name));
    if (name[0] == '\0') {
        printf("Resource name cannot be empty.\n");
        return;
    }
    printf("Resource type: 1. Ambulance  2. Rescue Team  3. Medical Team\n");
    typeChoice = readInt("Choose type: ", 1, 3);
    listLocations();
    location = readInt("Enter resource location ID: ", 0, roadGraph.count - 1);

    r.id = nextResourceId++;
    strncpy(r.name, name, MAX_NAME - 1);
    r.name[MAX_NAME - 1] = '\0';
    r.location = location;
    r.available = 1;
    if (typeChoice == 1) strcpy(r.type, "Ambulance");
    else if (typeChoice == 2) strcpy(r.type, "Rescue Team");
    else strcpy(r.type, "Medical Team");

    node = (ResourceNode *)malloc(sizeof(ResourceNode));
    if (!node) {
        printf("Memory allocation failed.\n");
        return;
    }
    node->data = r;
    node->next = resources;
    resources = node;
    printf("Resource added with ID %d.\n", r.id);
}

ResourceNode *findResourceNode(int id) {
    ResourceNode *p = resources;
    while (p) {
        if (p->data.id == id) return p;
        p = p->next;
    }
    return NULL;
}

void listResources(void) {
    ResourceNode *p = resources;
    if (!p) {
        printf("No resources registered.\n");
        return;
    }
    printf("\nResources:\n");
    while (p) {
        printf("ID:%d | %-14s | %-18s | Location:%s | %s\n",
               p->data.id, p->data.name, p->data.type,
               roadGraph.names[p->data.location],
               p->data.available ? "Available" : "Busy");
        p = p->next;
    }
}

void addEmergency(void) {
    Emergency e;
    char desc[MAX_NAME];
    int priority, location;
    if (queueData.count >= MAX_EMERGENCIES) {
        printf("Emergency queue is full.\n");
        return;
    }
    if (roadGraph.count == 0) {
        printf("Add locations before registering emergencies.\n");
        return;
    }
    readLine("Emergency description: ", desc, sizeof(desc));
    if (desc[0] == '\0') strcpy(desc, "Unspecified emergency");
    printf("Priority: 3. High  2. Medium  1. Low\n");
    priority = readInt("Choose priority: ", 1, 3);
    listLocations();
    location = readInt("Enter emergency location ID: ", 0, roadGraph.count - 1);

    e.id = queueData.nextId++;
    strncpy(e.description, desc, MAX_NAME - 1);
    e.description[MAX_NAME - 1] = '\0';
    e.location = location;
    e.priority = priority;
    e.arrivalOrder = queueData.nextOrder++;
    e.assignedResourceId = -1;
    e.active = 1;
    queueData.items[queueData.count++] = e;
    printf("Emergency #%d registered.\n", e.id);
}

int emergencyBefore(const Emergency *a, const Emergency *b) {
    if (a->priority != b->priority) return a->priority > b->priority;
    return a->arrivalOrder < b->arrivalOrder; /* FCFS tie-break */
}

int selectNextEmergencyIndex(void) {
    int best = -1;
    for (int i = 0; i < queueData.count; i++) {
        Emergency *e = &queueData.items[i];
        if (!e->active || e->assignedResourceId != -1) continue;
        if (best == -1 || emergencyBefore(e, &queueData.items[best])) best = i;
    }
    return best;
}

void listEmergencies(void) {
    int found = 0;
    printf("\nEmergencies (highest priority first):\n");
    for (int printed = 0; printed < queueData.count; printed++) {
        int best = -1;
        for (int i = 0; i < queueData.count; i++) {
            if (!queueData.items[i].active) continue;
            int alreadyPrinted = 0;
            /* Use active marker 2 temporarily to mark printed entries. */
            if (queueData.items[i].active == 2) alreadyPrinted = 1;
            if (alreadyPrinted) continue;
            if (best == -1 || emergencyBefore(&queueData.items[i], &queueData.items[best]))
                best = i;
        }
        if (best < 0) break;
        Emergency *e = &queueData.items[best];
        printf("#%d | %-28s | Priority:%s | Location:%s | %s",
               e->id, e->description,
               e->priority == 3 ? "High" : e->priority == 2 ? "Medium" : "Low",
               roadGraph.names[e->location],
               e->assignedResourceId == -1 ? "Waiting" : "Assigned");
        if (e->assignedResourceId != -1) printf(" (resource ID %d)", e->assignedResourceId);
        printf("\n");
        e->active = 2;
        found++;
    }
    for (int i = 0; i < queueData.count; i++)
        if (queueData.items[i].active == 2) queueData.items[i].active = 1;
    if (!found) printf("No active emergencies.\n");
}

int dijkstra(int source, int destination, int parent[MAX_LOCATIONS]) {
    int dist[MAX_LOCATIONS], visited[MAX_LOCATIONS] = {0};
    for (int i = 0; i < roadGraph.count; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[source] = 0;
    for (int step = 0; step < roadGraph.count; step++) {
        int u = -1, best = INF;
        for (int i = 0; i < roadGraph.count; i++)
            if (!visited[i] && dist[i] < best) { best = dist[i]; u = i; }
        if (u == -1) break;
        visited[u] = 1;
        for (int v = 0; v < roadGraph.count; v++) {
            if (!visited[v] && roadGraph.weights[u][v] < INF &&
                dist[u] + roadGraph.weights[u][v] < dist[v]) {
                dist[v] = dist[u] + roadGraph.weights[u][v];
                parent[v] = u;
            }
        }
    }
    if (dist[destination] >= INF) return -1;
    return dist[destination];
}

void printPath(int source, int destination, int parent[MAX_LOCATIONS]) {
    int path[MAX_LOCATIONS], n = 0, cur = destination;
    while (cur != -1 && n < MAX_LOCATIONS) {
        path[n++] = cur;
        if (cur == source) break;
        cur = parent[cur];
    }
    if (n == 0 || path[n - 1] != source) {
        printf("No route available.\n");
        return;
    }
    printf("Shortest route: ");
    for (int i = n - 1; i >= 0; i--) {
        printf("%s", roadGraph.names[path[i]]);
        if (i > 0) printf(" -> ");
    }
    printf("\n");
}

void allocateResource(void) {
    int ei = selectNextEmergencyIndex();
    ResourceNode *p, *best = NULL;
    int bestDistance = INF, bestParent[MAX_LOCATIONS];
    int bestSource = -1;

    if (ei < 0) {
        printf("No waiting emergencies.\n");
        return;
    }
    if (!resources) {
        printf("No resources registered.\n");
        return;
    }

    Emergency *e = &queueData.items[ei];
    for (p = resources; p; p = p->next) {
        int parent[MAX_LOCATIONS];
        int distance;
        if (!p->data.available) continue;
        distance = dijkstra(p->data.location, e->location, parent);
        if (distance >= 0 && distance < bestDistance) {
            bestDistance = distance;
            best = p;
            bestSource = p->data.location;
            memcpy(bestParent, parent, sizeof(bestParent));
        }
    }
    if (!best) {
        printf("No available resource can reach this emergency. Check roads or release a resource.\n");
        return;
    }
    best->data.available = 0;
    e->assignedResourceId = best->data.id;
    printf("\nAllocated emergency #%d (%s, %s priority)\n",
           e->id, e->description,
           e->priority == 3 ? "High" : e->priority == 2 ? "Medium" : "Low");
    printf("Assigned: %s (ID %d, %s)\n", best->data.name, best->data.id, best->data.type);
    printPath(bestSource, e->location, bestParent);
    printf("Total route weight: %d\n", bestDistance);
}

void releaseResource(void) {
    int id = readInt("Resource ID to mark available: ", 1, 1000000);
    ResourceNode *p = findResourceNode(id);
    if (!p) {
        printf("Resource not found.\n");
        return;
    }
    p->data.available = 1;
    for (int i = 0; i < queueData.count; i++) {
        if (queueData.items[i].active && queueData.items[i].assignedResourceId == id) {
            queueData.items[i].active = 0; /* completed emergency */
        }
    }
    printf("Resource %d is now available; its assigned emergency is marked completed.\n", id);
}

void saveData(void) {
    FILE *f = fopen("emergency_data.txt", "w");
    ResourceNode *p;2
    if (!f) {
        printf("Could not write emergency_data.txt\n");
        return;
    }
    fprintf(f, "LOCATIONS %d\n", roadGraph.count);
    for (int i = 0; i < roadGraph.count; i++) fprintf(f, "%s\n", roadGraph.names[i]);
    fprintf(f, "ROADS\n");
    for (int i = 0; i < roadGraph.count; i++)
        for (int j = i + 1; j < roadGraph.count; j++)
            if (roadGraph.weights[i][j] < INF)
                fprintf(f, "%d %d %d\n", i, j, roadGraph.weights[i][j]);
    fprintf(f, "RESOURCES\n");
    for (p = resources; p; p = p->next)
        fprintf(f, "%d|%s|%s|%d|%d\n", p->data.id, p->data.name, p->data.type,
                p->data.location, p->data.available);
    fprintf(f, "EMERGENCIES\n");
    for (int i = 0; i < queueData.count; i++) {
        Emergency *e = &queueData.items[i];
        fprintf(f, "%d|%s|%d|%d|%llu|%d|%d\n", e->id, e->description,
                e->location, e->priority, e->arrivalOrder, e->assignedResourceId, e->active);
    }
    fclose(f);
    printf("Data saved to emergency_data.txt (current program session data).\n");
}

void freeResources(void) {
    ResourceNode *p = resources;
    while (p) {
        ResourceNode *next = p->next;
        free(p);
        p = next;
    }
}

int main(void) {
    int choice;
    initGraph(&roadGraph);
    printf("============================================\n");
    printf(" EMERGENCY RESOURCES ALLOCATION SYSTEM (C)\n");
    printf("============================================\n");
    do {
        printf("\n1. Add location\n2. Add road\n3. List locations\n4. Add resource\n");
        printf("5. List resources\n6. Register emergency\n7. List emergencies\n");
        printf("8. Allocate next emergency\n9. Release/complete resource\n10. Save data\n0. Exit\n");
        choice = readInt("Choice: ", 0, 10);
        switch (choice) {
            case 1: addLocationMenu(); break;
            case 2: addRoad(); break;
            case 3: listLocations(); break;
            case 4: addResource(); break;
            case 5: listResources(); break;
            case 6: addEmergency(); break;
            case 7: listEmergencies(); break;
            case 8: allocateResource(); break;
            case 9: releaseResource(); break;
            case 10: saveData(); break;
            case 0: saveData(); printf("Exiting. Goodbye.\n"); break;
        }
    } while (choice != 0);
    freeResources();
    return 0;
}
