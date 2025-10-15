/**
 * @file location_graph.c
 * @brief Implementation of location graph system
 */

#include "location_graph.h"
#include "../../utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_CONNECTIONS 1000
#define MAX_LOCATIONS 200

/**
 * @brief Adjacency list node
 */
typedef struct AdjListNode {
    uint32_t to_location_id;
    uint8_t travel_time_hours;
    uint8_t danger_level;
    bool requires_unlock;
    char unlock_requirement[64];
    struct AdjListNode* next;
} AdjListNode;

/**
 * @brief Location graph structure
 */
struct LocationGraph {
    AdjListNode** adjacency_lists;  /**< Array of adjacency lists */
    uint32_t* location_ids;         /**< Array of unique location IDs */
    size_t location_count;          /**< Number of unique locations */
    size_t location_capacity;       /**< Capacity of location_ids array */
    size_t connection_count;        /**< Total number of connections */
};

/**
 * @brief Priority queue node for Dijkstra's algorithm
 */
typedef struct PQNode {
    uint32_t location_id;
    uint32_t cost;
} PQNode;

/**
 * @brief Simple min-heap priority queue
 */
typedef struct {
    PQNode* nodes;
    size_t size;
    size_t capacity;
} PriorityQueue;

/* Priority queue helper functions */
static PriorityQueue* pq_create(size_t capacity) {
    PriorityQueue* pq = malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;

    pq->nodes = malloc(sizeof(PQNode) * capacity);
    if (!pq->nodes) {
        free(pq);
        return NULL;
    }

    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

static void pq_destroy(PriorityQueue* pq) {
    if (!pq) return;
    free(pq->nodes);
    free(pq);
}

static void pq_swap(PQNode* a, PQNode* b) {
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

static void pq_heapify_up(PriorityQueue* pq, size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (pq->nodes[index].cost >= pq->nodes[parent].cost) break;
        pq_swap(&pq->nodes[index], &pq->nodes[parent]);
        index = parent;
    }
}

static void pq_heapify_down(PriorityQueue* pq, size_t index) {
    while (index < pq->size) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t smallest = index;

        if (left < pq->size && pq->nodes[left].cost < pq->nodes[smallest].cost) {
            smallest = left;
        }
        if (right < pq->size && pq->nodes[right].cost < pq->nodes[smallest].cost) {
            smallest = right;
        }

        if (smallest == index) break;

        pq_swap(&pq->nodes[index], &pq->nodes[smallest]);
        index = smallest;
    }
}

static bool pq_push(PriorityQueue* pq, uint32_t location_id, uint32_t cost) {
    if (pq->size >= pq->capacity) return false;

    pq->nodes[pq->size].location_id = location_id;
    pq->nodes[pq->size].cost = cost;
    pq_heapify_up(pq, pq->size);
    pq->size++;
    return true;
}

static bool pq_pop(PriorityQueue* pq, PQNode* node) {
    if (pq->size == 0) return false;

    *node = pq->nodes[0];
    pq->nodes[0] = pq->nodes[pq->size - 1];
    pq->size--;
    if (pq->size > 0) {
        pq_heapify_down(pq, 0);
    }
    return true;
}

static bool pq_is_empty(const PriorityQueue* pq) {
    return pq->size == 0;
}

/* Graph helper functions */
static int find_location_index(const LocationGraph* graph, uint32_t location_id) {
    for (size_t i = 0; i < graph->location_count; i++) {
        if (graph->location_ids[i] == location_id) {
            return (int)i;
        }
    }
    return -1;
}

static bool add_location_id(LocationGraph* graph, uint32_t location_id) {
    if (find_location_index(graph, location_id) >= 0) {
        return true; /* Already exists */
    }

    if (graph->location_count >= graph->location_capacity) {
        /* Grow capacity */
        size_t new_capacity = graph->location_capacity * 2;
        uint32_t* new_ids = realloc(graph->location_ids,
                                     sizeof(uint32_t) * new_capacity);
        if (!new_ids) return false;

        AdjListNode** new_lists = realloc(graph->adjacency_lists,
                                          sizeof(AdjListNode*) * new_capacity);
        if (!new_lists) {
            free(new_ids);
            return false;
        }

        /* Initialize new adjacency lists to NULL */
        for (size_t i = graph->location_capacity; i < new_capacity; i++) {
            new_lists[i] = NULL;
        }

        graph->location_ids = new_ids;
        graph->adjacency_lists = new_lists;
        graph->location_capacity = new_capacity;
    }

    graph->location_ids[graph->location_count] = location_id;
    graph->adjacency_lists[graph->location_count] = NULL;
    graph->location_count++;
    return true;
}

/* Public API Implementation */

LocationGraph* location_graph_create(void) {
    LocationGraph* graph = malloc(sizeof(LocationGraph));
    if (!graph) {
        LOG_ERROR("location_graph_create: Failed to allocate graph");
        return NULL;
    }

    graph->location_capacity = 50; /* Initial capacity */
    graph->location_ids = malloc(sizeof(uint32_t) * graph->location_capacity);
    if (!graph->location_ids) {
        free(graph);
        LOG_ERROR("location_graph_create: Failed to allocate location_ids");
        return NULL;
    }

    graph->adjacency_lists = malloc(sizeof(AdjListNode*) * graph->location_capacity);
    if (!graph->adjacency_lists) {
        free(graph->location_ids);
        free(graph);
        LOG_ERROR("location_graph_create: Failed to allocate adjacency_lists");
        return NULL;
    }

    for (size_t i = 0; i < graph->location_capacity; i++) {
        graph->adjacency_lists[i] = NULL;
    }

    graph->location_count = 0;
    graph->connection_count = 0;

    LOG_DEBUG("location_graph_create: Created graph with capacity %zu",
               graph->location_capacity);
    return graph;
}

void location_graph_destroy(LocationGraph* graph) {
    if (!graph) return;

    /* Free all adjacency list nodes */
    for (size_t i = 0; i < graph->location_count; i++) {
        AdjListNode* node = graph->adjacency_lists[i];
        while (node) {
            AdjListNode* next = node->next;
            free(node);
            node = next;
        }
    }

    free(graph->adjacency_lists);
    free(graph->location_ids);
    free(graph);

    LOG_DEBUG( "location_graph_destroy: Graph destroyed");
}

bool location_graph_add_connection(LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id,
                                    uint8_t travel_time,
                                    uint8_t danger_level) {
    if (!graph) {
        LOG_ERROR( "location_graph_add_connection: NULL graph");
        return false;
    }

    if (travel_time == 0) {
        LOG_WARN( "location_graph_add_connection: Travel time cannot be 0");
        return false;
    }

    /* Ensure both locations are registered */
    if (!add_location_id(graph, from_id) || !add_location_id(graph, to_id)) {
        LOG_ERROR( "location_graph_add_connection: Failed to add location IDs");
        return false;
    }

    int from_index = find_location_index(graph, from_id);
    if (from_index < 0) {
        LOG_ERROR( "location_graph_add_connection: Invalid from_id %u", from_id);
        return false;
    }

    /* Create new adjacency list node */
    AdjListNode* new_node = malloc(sizeof(AdjListNode));
    if (!new_node) {
        LOG_ERROR( "location_graph_add_connection: Failed to allocate node");
        return false;
    }

    new_node->to_location_id = to_id;
    new_node->travel_time_hours = travel_time;
    new_node->danger_level = danger_level;
    new_node->requires_unlock = false;
    new_node->unlock_requirement[0] = '\0';

    /* Insert at head of adjacency list */
    new_node->next = graph->adjacency_lists[from_index];
    graph->adjacency_lists[from_index] = new_node;

    graph->connection_count++;

    LOG_DEBUG( "location_graph_add_connection: Added %u -> %u (time=%u, danger=%u)",
               from_id, to_id, travel_time, danger_level);
    return true;
}

bool location_graph_add_bidirectional(LocationGraph* graph,
                                       uint32_t location_a,
                                       uint32_t location_b,
                                       uint8_t travel_time,
                                       uint8_t danger_level) {
    if (!location_graph_add_connection(graph, location_a, location_b,
                                        travel_time, danger_level)) {
        return false;
    }

    if (!location_graph_add_connection(graph, location_b, location_a,
                                        travel_time, danger_level)) {
        return false;
    }

    return true;
}

bool location_graph_set_unlock_requirement(LocationGraph* graph,
                                            uint32_t from_id,
                                            uint32_t to_id,
                                            const char* requirement) {
    if (!graph || !requirement) {
        LOG_ERROR( "location_graph_set_unlock_requirement: NULL parameter");
        return false;
    }

    int from_index = find_location_index(graph, from_id);
    if (from_index < 0) {
        LOG_ERROR( "location_graph_set_unlock_requirement: Invalid from_id %u",
                   from_id);
        return false;
    }

    /* Find the connection */
    AdjListNode* node = graph->adjacency_lists[from_index];
    while (node) {
        if (node->to_location_id == to_id) {
            node->requires_unlock = true;
            strncpy(node->unlock_requirement, requirement,
                    sizeof(node->unlock_requirement) - 1);
            node->unlock_requirement[sizeof(node->unlock_requirement) - 1] = '\0';

            LOG_DEBUG(
                       "location_graph_set_unlock_requirement: Set %u -> %u requires '%s'",
                       from_id, to_id, requirement);
            return true;
        }
        node = node->next;
    }

    LOG_WARN( "location_graph_set_unlock_requirement: Connection %u -> %u not found",
               from_id, to_id);
    return false;
}

bool location_graph_has_connection(const LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id) {
    if (!graph) return false;

    int from_index = find_location_index(graph, from_id);
    if (from_index < 0) return false;

    AdjListNode* node = graph->adjacency_lists[from_index];
    while (node) {
        if (node->to_location_id == to_id) {
            return true;
        }
        node = node->next;
    }

    return false;
}

size_t location_graph_get_neighbors(const LocationGraph* graph,
                                     uint32_t location_id,
                                     uint32_t* neighbors,
                                     size_t max_neighbors) {
    if (!graph || !neighbors) return 0;

    int index = find_location_index(graph, location_id);
    if (index < 0) return 0;

    size_t count = 0;
    AdjListNode* node = graph->adjacency_lists[index];
    while (node && count < max_neighbors) {
        neighbors[count++] = node->to_location_id;
        node = node->next;
    }

    return count;
}

bool location_graph_get_connection(const LocationGraph* graph,
                                    uint32_t from_id,
                                    uint32_t to_id,
                                    LocationConnection* connection) {
    if (!graph) return false;

    int from_index = find_location_index(graph, from_id);
    if (from_index < 0) return false;

    AdjListNode* node = graph->adjacency_lists[from_index];
    while (node) {
        if (node->to_location_id == to_id) {
            if (connection) {
                connection->from_location_id = from_id;
                connection->to_location_id = to_id;
                connection->travel_time_hours = node->travel_time_hours;
                connection->danger_level = node->danger_level;
                connection->requires_unlock = node->requires_unlock;
                strncpy(connection->unlock_requirement, node->unlock_requirement,
                        sizeof(connection->unlock_requirement) - 1);
                connection->unlock_requirement[sizeof(connection->unlock_requirement) - 1] = '\0';
            }
            return true;
        }
        node = node->next;
    }

    return false;
}

bool location_graph_find_path(const LocationGraph* graph,
                               uint32_t from_id,
                               uint32_t to_id,
                               PathfindingResult* result) {
    if (!graph || !result) {
        LOG_ERROR( "location_graph_find_path: NULL parameter");
        return false;
    }

    /* Initialize result */
    result->path = NULL;
    result->path_length = 0;
    result->total_travel_time = 0;
    result->total_danger = 0;
    result->path_found = false;

    /* Check if locations exist */
    if (find_location_index(graph, from_id) < 0 ||
        find_location_index(graph, to_id) < 0) {
        LOG_WARN( "location_graph_find_path: Invalid location ID");
        return true; /* Not an error, just no path */
    }

    /* Same location */
    if (from_id == to_id) {
        result->path = malloc(sizeof(uint32_t));
        if (!result->path) return false;
        result->path[0] = from_id;
        result->path_length = 1;
        result->path_found = true;
        return true;
    }

    /* Dijkstra's algorithm */
    PriorityQueue* pq = pq_create(graph->location_count);
    if (!pq) {
        LOG_ERROR( "location_graph_find_path: Failed to create priority queue");
        return false;
    }

    uint32_t* distances = malloc(sizeof(uint32_t) * graph->location_count);
    uint32_t* predecessors = malloc(sizeof(uint32_t) * graph->location_count);
    bool* visited = calloc(graph->location_count, sizeof(bool));

    if (!distances || !predecessors || !visited) {
        LOG_ERROR( "location_graph_find_path: Memory allocation failed");
        free(distances);
        free(predecessors);
        free(visited);
        pq_destroy(pq);
        return false;
    }

    /* Initialize distances to infinity */
    for (size_t i = 0; i < graph->location_count; i++) {
        distances[i] = UINT32_MAX;
        predecessors[i] = UINT32_MAX;
    }

    int start_index = find_location_index(graph, from_id);
    distances[start_index] = 0;
    pq_push(pq, from_id, 0);

    bool found = false;
    while (!pq_is_empty(pq)) {
        PQNode current;
        pq_pop(pq, &current);

        int current_index = find_location_index(graph, current.location_id);
        if (current_index < 0) continue;

        if (visited[current_index]) continue;
        visited[current_index] = true;

        if (current.location_id == to_id) {
            found = true;
            break;
        }

        /* Explore neighbors */
        AdjListNode* node = graph->adjacency_lists[current_index];
        while (node) {
            int neighbor_index = find_location_index(graph, node->to_location_id);
            if (neighbor_index < 0) {
                node = node->next;
                continue;
            }

            if (visited[neighbor_index]) {
                node = node->next;
                continue;
            }

            /* Skip locked connections (for now, we ignore unlock requirements) */
            /* In a full implementation, this would check player's unlocks */

            uint32_t new_dist = distances[current_index] + node->travel_time_hours;
            if (new_dist < distances[neighbor_index]) {
                distances[neighbor_index] = new_dist;
                predecessors[neighbor_index] = current_index;
                pq_push(pq, node->to_location_id, new_dist);
            }

            node = node->next;
        }
    }

    if (found) {
        /* Reconstruct path */
        int target_index = find_location_index(graph, to_id);

        /* Count path length */
        size_t path_length = 0;
        int index = target_index;
        while (index != start_index && predecessors[index] != UINT32_MAX) {
            path_length++;
            index = predecessors[index];
        }
        path_length++; /* Include starting location */

        /* Allocate and fill path array */
        result->path = malloc(sizeof(uint32_t) * path_length);
        if (!result->path) {
            free(distances);
            free(predecessors);
            free(visited);
            pq_destroy(pq);
            return false;
        }

        /* Fill path in reverse */
        index = target_index;
        for (int i = (int)path_length - 1; i >= 0; i--) {
            result->path[i] = graph->location_ids[index];
            if (i > 0) {
                index = predecessors[index];
            }
        }

        result->path_length = path_length;
        result->total_travel_time = distances[target_index];
        result->path_found = true;

        /* Calculate total danger */
        for (size_t i = 0; i < path_length - 1; i++) {
            LocationConnection conn;
            if (location_graph_get_connection(graph, result->path[i],
                                               result->path[i + 1], &conn)) {
                result->total_danger += conn.danger_level;
            }
        }
    }

    free(distances);
    free(predecessors);
    free(visited);
    pq_destroy(pq);

    return true;
}

void pathfinding_result_free(PathfindingResult* result) {
    if (!result) return;
    free(result->path);
    result->path = NULL;
    result->path_length = 0;
}

bool location_graph_is_reachable(const LocationGraph* graph,
                                  uint32_t from_id,
                                  uint32_t to_id) {
    if (!graph) return false;

    /* Use pathfinding to check reachability */
    PathfindingResult result;
    if (!location_graph_find_path(graph, from_id, to_id, &result)) {
        return false;
    }

    bool reachable = result.path_found;
    pathfinding_result_free(&result);
    return reachable;
}

size_t location_graph_get_connection_count(const LocationGraph* graph) {
    return graph ? graph->connection_count : 0;
}

size_t location_graph_get_all_locations(const LocationGraph* graph,
                                         uint32_t* locations,
                                         size_t max_locations) {
    if (!graph || !locations) return 0;

    size_t count = graph->location_count < max_locations ?
                   graph->location_count : max_locations;

    for (size_t i = 0; i < count; i++) {
        locations[i] = graph->location_ids[i];
    }

    return count;
}

bool location_graph_validate_connectivity(const LocationGraph* graph,
                                           uint32_t starting_location_id) {
    if (!graph) return false;

    if (graph->location_count == 0) return true; /* Empty graph is valid */

    /* Check if all locations are reachable from start */
    for (size_t i = 0; i < graph->location_count; i++) {
        uint32_t location_id = graph->location_ids[i];
        if (location_id == starting_location_id) continue;

        if (!location_graph_is_reachable(graph, starting_location_id, location_id)) {
            LOG_WARN(
                       "location_graph_validate_connectivity: Location %u unreachable from %u",
                       location_id, starting_location_id);
            return false;
        }
    }

    LOG_INFO( "location_graph_validate_connectivity: All %zu locations reachable",
               graph->location_count);
    return true;
}
