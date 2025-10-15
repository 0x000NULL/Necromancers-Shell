/**
 * @file world_map.c
 * @brief Implementation of world map visualization system
 */

#include "world_map.h"
#include "../../utils/logger.h"
#include "../../utils/hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_MAP_WIDTH 120
#define MAX_MAP_HEIGHT 40

/**
 * @brief World map structure
 */
struct WorldMap {
    TerritoryManager* territory;    /**< Territory manager (not owned) */
    LocationGraph* graph;           /**< Location graph (not owned) */
    HashTable* location_data;       /**< Key: location_id, Value: LocationMapData* */
};

/* Helper structures for iteration callbacks */

typedef struct {
    uint32_t* results;
    size_t count;
    size_t max_results;
    MapRegion filter_region;
} RegionFilterContext;

typedef struct {
    uint32_t* results;
    size_t count;
    size_t max_results;
    MapCoordinates center_coords;
    uint16_t radius;
    uint32_t center_id;
} RadiusFilterContext;

typedef struct {
    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;
    bool first;
} BoundsContext;

typedef struct {
    char** grid;
    uint16_t width;
    uint16_t height;
    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;
    int16_t world_width;
    int16_t world_height;
    uint32_t current_location_id;
    uint32_t highlight_location_id;
    bool show_undiscovered;
    const WorldMap* map;
    const MapRenderOptions* opts;
} RenderContext;

/* Helper functions */

static char get_default_symbol(LocationType type) {
    switch (type) {
        case LOCATION_TYPE_GRAVEYARD:    return 'G';
        case LOCATION_TYPE_BATTLEFIELD:  return 'B';
        case LOCATION_TYPE_VILLAGE:      return 'V';
        case LOCATION_TYPE_CRYPT:        return 'C';
        case LOCATION_TYPE_RITUAL_SITE:  return 'R';
        default:                         return '?';
    }
}

static LocationMapData* get_or_create_map_data(WorldMap* map, uint32_t location_id) {
    char key[32];
    snprintf(key, sizeof(key), "%u", location_id);

    LocationMapData* data = hash_table_get(map->location_data, key);
    if (data) {
        return data;
    }

    /* Create new map data */
    data = malloc(sizeof(LocationMapData));
    if (!data) return NULL;

    data->location_id = location_id;
    data->coords.x = 0;
    data->coords.y = 0;
    data->region = MAP_REGION_STARTING_GROUNDS;
    data->discovered = false;

    /* Get location to determine symbol */
    Location* loc = territory_manager_get_location(map->territory, location_id);
    if (loc) {
        data->symbol = get_default_symbol(loc->type);
        data->discovered = (loc->status == LOCATION_STATUS_DISCOVERED);
    } else {
        data->symbol = '?';
    }

    hash_table_put(map->location_data, key, data);
    return data;
}

/* Callback functions for hash_table_foreach */

static void free_location_data_callback(const char* key, void* value, void* userdata) {
    (void)key;
    (void)userdata;
    free(value);
}

static void filter_by_region_callback(const char* key, void* value, void* userdata) {
    (void)key;
    RegionFilterContext* ctx = (RegionFilterContext*)userdata;
    LocationMapData* data = (LocationMapData*)value;

    if (data && data->region == ctx->filter_region && ctx->count < ctx->max_results) {
        ctx->results[ctx->count++] = data->location_id;
    }
}

static void filter_by_radius_callback(const char* key, void* value, void* userdata) {
    (void)key;
    RadiusFilterContext* ctx = (RadiusFilterContext*)userdata;
    LocationMapData* data = (LocationMapData*)value;

    if (!data || data->location_id == ctx->center_id || ctx->count >= ctx->max_results) {
        return;
    }

    /* Manhattan distance */
    int16_t dx = abs(data->coords.x - ctx->center_coords.x);
    int16_t dy = abs(data->coords.y - ctx->center_coords.y);
    uint16_t distance = dx + dy;

    if (distance <= ctx->radius) {
        ctx->results[ctx->count++] = data->location_id;
    }
}

static void calculate_bounds_callback(const char* key, void* value, void* userdata) {
    (void)key;
    BoundsContext* ctx = (BoundsContext*)userdata;
    LocationMapData* data = (LocationMapData*)value;

    if (!data) return;

    if (ctx->first) {
        ctx->min_x = ctx->max_x = data->coords.x;
        ctx->min_y = ctx->max_y = data->coords.y;
        ctx->first = false;
    } else {
        if (data->coords.x < ctx->min_x) ctx->min_x = data->coords.x;
        if (data->coords.x > ctx->max_x) ctx->max_x = data->coords.x;
        if (data->coords.y < ctx->min_y) ctx->min_y = data->coords.y;
        if (data->coords.y > ctx->max_y) ctx->max_y = data->coords.y;
    }
}

static void plot_location_callback(const char* key, void* value, void* userdata) {
    (void)key;
    RenderContext* ctx = (RenderContext*)userdata;
    LocationMapData* data = (LocationMapData*)value;

    if (!data) return;

    /* Skip undiscovered if option disabled */
    if (!ctx->show_undiscovered && !data->discovered) return;

    /* Transform world coordinates to map coordinates */
    int16_t map_x = ((data->coords.x - ctx->min_x) * (ctx->width - 3)) / ctx->world_width + 1;
    int16_t map_y = ((data->coords.y - ctx->min_y) * (ctx->height - 3)) / ctx->world_height + 1;

    /* Clamp to valid range */
    if (map_x < 1) { map_x = 1; }
    if (map_x >= (int16_t)(ctx->width - 1)) { map_x = ctx->width - 2; }
    if (map_y < 1) { map_y = 1; }
    if (map_y >= (int16_t)(ctx->height - 1)) { map_y = ctx->height - 2; }

    /* Determine symbol */
    char symbol = data->symbol;
    if (!data->discovered && ctx->show_undiscovered) {
        symbol = '?';
    }
    if (data->location_id == ctx->current_location_id) {
        symbol = '@';
    }
    if (data->location_id == ctx->highlight_location_id) {
        symbol = '*';
    }

    ctx->grid[map_y][map_x] = symbol;
}

static void draw_connections_callback(const char* key, void* value, void* userdata) {
    (void)key;
    RenderContext* ctx = (RenderContext*)userdata;
    LocationMapData* data = (LocationMapData*)value;

    if (!data) return;

    /* Get neighbors */
    uint32_t neighbors[20];
    size_t neighbor_count = location_graph_get_neighbors(ctx->map->graph,
                                                          data->location_id,
                                                          neighbors, 20);

    for (size_t j = 0; j < neighbor_count; j++) {
        MapCoordinates neighbor_coords;
        if (!world_map_get_coordinates(ctx->map, neighbors[j], &neighbor_coords)) {
            continue;
        }

        /* Transform coordinates */
        int16_t x1 = ((data->coords.x - ctx->min_x) * (ctx->width - 3)) / ctx->world_width + 1;
        int16_t y1 = ((data->coords.y - ctx->min_y) * (ctx->height - 3)) / ctx->world_height + 1;
        int16_t x2 = ((neighbor_coords.x - ctx->min_x) * (ctx->width - 3)) / ctx->world_width + 1;
        int16_t y2 = ((neighbor_coords.y - ctx->min_y) * (ctx->height - 3)) / ctx->world_height + 1;

        /* Clamp */
        if (x1 < 1) { x1 = 1; }
        if (x1 >= (int16_t)(ctx->width - 1)) { x1 = ctx->width - 2; }
        if (y1 < 1) { y1 = 1; }
        if (y1 >= (int16_t)(ctx->height - 1)) { y1 = ctx->height - 2; }
        if (x2 < 1) { x2 = 1; }
        if (x2 >= (int16_t)(ctx->width - 1)) { x2 = ctx->width - 2; }
        if (y2 < 1) { y2 = 1; }
        if (y2 >= (int16_t)(ctx->height - 1)) { y2 = ctx->height - 2; }

        /* Draw simple line (only if not too far) */
        int16_t dx = abs(x2 - x1);
        int16_t dy = abs(y2 - y1);
        if (dx + dy <= 5) {
            /* Draw midpoint dot */
            int16_t mx = (x1 + x2) / 2;
            int16_t my = (y1 + y2) / 2;
            if (ctx->grid[my][mx] == ' ') {
                ctx->grid[my][mx] = '.';
            }
        }
    }
}

/* Public API */

MapRenderOptions map_render_options_default(void) {
    MapRenderOptions opts;
    opts.width = 60;
    opts.height = 24;
    opts.show_undiscovered = true;
    opts.show_connections = true;
    opts.show_region_labels = false;
    opts.show_legend = true;
    opts.highlight_location_id = 0;
    opts.highlight_path = NULL;
    opts.highlight_path_length = 0;
    return opts;
}

WorldMap* world_map_create(TerritoryManager* territory, LocationGraph* graph) {
    if (!territory || !graph) {
        LOG_ERROR("world_map_create: NULL parameter");
        return NULL;
    }

    WorldMap* map = malloc(sizeof(WorldMap));
    if (!map) {
        LOG_ERROR("world_map_create: Failed to allocate WorldMap");
        return NULL;
    }

    map->territory = territory;
    map->graph = graph;
    map->location_data = hash_table_create(100);

    if (!map->location_data) {
        free(map);
        LOG_ERROR("world_map_create: Failed to create hash table");
        return NULL;
    }

    LOG_DEBUG("world_map_create: Created world map");
    return map;
}

void world_map_destroy(WorldMap* map) {
    if (!map) return;

    /* Free all location map data */
    if (map->location_data) {
        /* Iterate and free all values */
        hash_table_foreach(map->location_data, free_location_data_callback, NULL);
        hash_table_destroy(map->location_data);
    }

    free(map);
    LOG_DEBUG("world_map_destroy: Destroyed world map");
}

bool world_map_set_coordinates(WorldMap* map, uint32_t location_id,
                                int16_t x, int16_t y) {
    if (!map) return false;

    LocationMapData* data = get_or_create_map_data(map, location_id);
    if (!data) return false;

    data->coords.x = x;
    data->coords.y = y;

    LOG_DEBUG("world_map_set_coordinates: Set location %u to (%d, %d)",
              location_id, x, y);
    return true;
}

bool world_map_get_coordinates(const WorldMap* map, uint32_t location_id,
                                MapCoordinates* coords) {
    if (!map) return false;

    char key[32];
    snprintf(key, sizeof(key), "%u", location_id);

    LocationMapData* data = hash_table_get(map->location_data, key);
    if (!data) return false;

    if (coords) {
        *coords = data->coords;
    }

    return true;
}

bool world_map_set_region(WorldMap* map, uint32_t location_id, MapRegion region) {
    if (!map || region >= MAP_REGION_COUNT) return false;

    LocationMapData* data = get_or_create_map_data(map, location_id);
    if (!data) return false;

    data->region = region;
    LOG_DEBUG("world_map_set_region: Set location %u to region %d",
              location_id, region);
    return true;
}

MapRegion world_map_get_region(const WorldMap* map, uint32_t location_id) {
    if (!map) return MAP_REGION_STARTING_GROUNDS;

    char key[32];
    snprintf(key, sizeof(key), "%u", location_id);

    LocationMapData* data = hash_table_get(map->location_data, key);
    if (!data) return MAP_REGION_STARTING_GROUNDS;

    return data->region;
}

bool world_map_set_symbol(WorldMap* map, uint32_t location_id, char symbol) {
    if (!map) return false;

    LocationMapData* data = get_or_create_map_data(map, location_id);
    if (!data) return false;

    data->symbol = symbol;
    return true;
}

size_t world_map_get_locations_in_region(const WorldMap* map, MapRegion region,
                                          uint32_t* results, size_t max_results) {
    if (!map || !results || region >= MAP_REGION_COUNT) return 0;

    RegionFilterContext ctx = {
        .results = results,
        .count = 0,
        .max_results = max_results,
        .filter_region = region
    };

    hash_table_foreach(map->location_data, filter_by_region_callback, &ctx);
    return ctx.count;
}

size_t world_map_get_locations_in_radius(const WorldMap* map,
                                          uint32_t center_id,
                                          uint16_t radius,
                                          uint32_t* results,
                                          size_t max_results) {
    if (!map || !results) return 0;

    MapCoordinates center_coords;
    if (!world_map_get_coordinates(map, center_id, &center_coords)) {
        return 0;
    }

    RadiusFilterContext ctx = {
        .results = results,
        .count = 0,
        .max_results = max_results,
        .center_coords = center_coords,
        .radius = radius,
        .center_id = center_id
    };

    hash_table_foreach(map->location_data, filter_by_radius_callback, &ctx);
    return ctx.count;
}

bool world_map_get_bounds(const WorldMap* map,
                           int16_t* min_x, int16_t* max_x,
                           int16_t* min_y, int16_t* max_y) {
    if (!map) return false;

    if (hash_table_size(map->location_data) == 0) {
        return false;
    }

    BoundsContext ctx = {
        .min_x = 0,
        .max_x = 0,
        .min_y = 0,
        .max_y = 0,
        .first = true
    };

    hash_table_foreach(map->location_data, calculate_bounds_callback, &ctx);

    if (ctx.first) {
        return false;  /* No valid locations found */
    }

    if (min_x) *min_x = ctx.min_x;
    if (max_x) *max_x = ctx.max_x;
    if (min_y) *min_y = ctx.min_y;
    if (max_y) *max_y = ctx.max_y;

    return true;
}

size_t world_map_render(const WorldMap* map,
                         uint32_t current_location_id,
                         const MapRenderOptions* options,
                         char* buffer,
                         size_t buffer_size) {
    if (!map || !buffer || buffer_size == 0) return 0;

    MapRenderOptions opts = options ? *options : map_render_options_default();

    /* Clamp dimensions */
    if (opts.width > MAX_MAP_WIDTH) opts.width = MAX_MAP_WIDTH;
    if (opts.height > MAX_MAP_HEIGHT) opts.height = MAX_MAP_HEIGHT;

    /* Get bounding box */
    int16_t min_x, max_x, min_y, max_y;
    if (!world_map_get_bounds(map, &min_x, &max_x, &min_y, &max_y)) {
        strncpy(buffer, "[Empty map - no locations]", buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return strlen(buffer);
    }

    /* Add padding */
    min_x -= 2; max_x += 2;
    min_y -= 2; max_y += 2;

    /* Calculate scale */
    int16_t world_width = max_x - min_x;
    int16_t world_height = max_y - min_y;

    if (world_width == 0) world_width = 1;
    if (world_height == 0) world_height = 1;

    /* Create map grid */
    char** grid = malloc(sizeof(char*) * opts.height);
    if (!grid) return 0;

    for (uint16_t y = 0; y < opts.height; y++) {
        grid[y] = malloc(opts.width + 1);
        if (!grid[y]) {
            for (uint16_t j = 0; j < y; j++) free(grid[j]);
            free(grid);
            return 0;
        }
        memset(grid[y], ' ', opts.width);
        grid[y][opts.width] = '\0';
    }

    /* Draw border */
    for (uint16_t x = 0; x < opts.width; x++) {
        grid[0][x] = '-';
        grid[opts.height - 1][x] = '-';
    }
    for (uint16_t y = 0; y < opts.height; y++) {
        grid[y][0] = '|';
        grid[y][opts.width - 1] = '|';
    }
    grid[0][0] = '+';
    grid[0][opts.width - 1] = '+';
    grid[opts.height - 1][0] = '+';
    grid[opts.height - 1][opts.width - 1] = '+';

    /* Plot locations */
    RenderContext render_ctx = {
        .grid = grid,
        .width = opts.width,
        .height = opts.height,
        .min_x = min_x,
        .max_x = max_x,
        .min_y = min_y,
        .max_y = max_y,
        .world_width = world_width,
        .world_height = world_height,
        .current_location_id = current_location_id,
        .highlight_location_id = opts.highlight_location_id,
        .show_undiscovered = opts.show_undiscovered,
        .map = map,
        .opts = &opts
    };

    hash_table_foreach(map->location_data, plot_location_callback, &render_ctx);

    /* Draw connections */
    if (opts.show_connections) {
        hash_table_foreach(map->location_data, draw_connections_callback, &render_ctx);
    }

    /* Assemble buffer */
    size_t written = 0;
    for (uint16_t y = 0; y < opts.height && written < buffer_size - 1; y++) {
        size_t line_len = strlen(grid[y]);
        size_t copy_len = line_len;
        if (written + copy_len + 1 >= buffer_size) {
            copy_len = buffer_size - written - 2;
        }

        memcpy(buffer + written, grid[y], copy_len);
        written += copy_len;

        if (written < buffer_size - 1) {
            buffer[written++] = '\n';
        }
    }

    /* Add legend if enabled */
    if (opts.show_legend && written < buffer_size - 100) {
        const char* legend = "\n[Legend: @ = You, G = Graveyard, B = Battlefield, V = Village, C = Crypt, R = Ritual Site, ? = Undiscovered]\n";
        size_t legend_len = strlen(legend);
        if (written + legend_len < buffer_size) {
            strcpy(buffer + written, legend);
            written += legend_len;
        }
    }

    buffer[written] = '\0';

    /* Free grid */
    for (uint16_t y = 0; y < opts.height; y++) {
        free(grid[y]);
    }
    free(grid);

    return written;
}

size_t world_map_get_legend(char* buffer, size_t buffer_size) {
    const char* legend =
        "Map Legend:\n"
        "  @ = Your current location\n"
        "  G = Graveyard\n"
        "  B = Battlefield\n"
        "  V = Village\n"
        "  C = Crypt\n"
        "  R = Ritual Site\n"
        "  ? = Undiscovered location\n"
        "  . = Connection path\n";

    strncpy(buffer, legend, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return strlen(buffer);
}

const char* world_map_region_name(MapRegion region) {
    switch (region) {
        case MAP_REGION_STARTING_GROUNDS: return "Starting Grounds";
        case MAP_REGION_EASTERN_WASTES:   return "Eastern Wastes";
        case MAP_REGION_SOUTHERN_VILLAGES: return "Southern Villages";
        case MAP_REGION_WESTERN_CRYPTS:   return "Western Crypts";
        case MAP_REGION_NORTHERN_RITUAL:  return "Northern Ritual Sites";
        case MAP_REGION_CENTRAL_NEXUS:    return "Central Nexus";
        default:                          return "Unknown Region";
    }
}

bool world_map_auto_layout(WorldMap* map, uint32_t iterations) {
    if (!map) return false;

    /* Simple circular layout algorithm */
    uint32_t locations[100];
    size_t count = location_graph_get_all_locations(map->graph, locations, 100);

    if (count == 0) return true;

    /* Arrange in a circle */
    double angle_step = (2.0 * M_PI) / count;
    double radius = (count * 3.0); /* Scale radius with location count */

    for (size_t i = 0; i < count; i++) {
        double angle = i * angle_step;
        int16_t x = (int16_t)(cos(angle) * radius);
        int16_t y = (int16_t)(sin(angle) * radius);

        world_map_set_coordinates(map, locations[i], x, y);
    }

    LOG_INFO("world_map_auto_layout: Laid out %zu locations in circular pattern",
             count);

    (void)iterations; /* Unused for simple layout */
    return true;
}
