#include "resources.h"
#include <stdio.h>

void resources_init(Resources* resources) {
    if (!resources) {
        return;
    }

    resources->soul_energy = 0;
    resources->mana = 100;
    resources->mana_max = 100;
    resources->day_count = 0;
    resources->time_hours = 0;
}

bool resources_add_soul_energy(Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    resources->soul_energy += amount;
    return true;
}

bool resources_spend_soul_energy(Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    if (resources->soul_energy < amount) {
        return false;
    }

    resources->soul_energy -= amount;
    return true;
}

bool resources_has_soul_energy(const Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    return resources->soul_energy >= amount;
}

bool resources_add_mana(Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    resources->mana += amount;
    if (resources->mana > resources->mana_max) {
        resources->mana = resources->mana_max;
    }

    return true;
}

bool resources_spend_mana(Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    if (resources->mana < amount) {
        return false;
    }

    resources->mana -= amount;
    return true;
}

bool resources_has_mana(const Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    return resources->mana >= amount;
}

void resources_regenerate_mana(Resources* resources, uint32_t amount) {
    if (!resources) {
        return;
    }

    resources->mana += amount;
    if (resources->mana > resources->mana_max) {
        resources->mana = resources->mana_max;
    }
}

bool resources_increase_max_mana(Resources* resources, uint32_t amount) {
    if (!resources) {
        return false;
    }

    resources->mana_max += amount;
    return true;
}

void resources_advance_time(Resources* resources, uint32_t hours) {
    if (!resources) {
        return;
    }

    resources->time_hours += hours;

    /* Handle day rollover */
    while (resources->time_hours >= 24) {
        resources->time_hours -= 24;
        resources->day_count++;
    }
}

int resources_format_time(const Resources* resources, char* buffer, size_t buffer_size) {
    if (!resources || !buffer || buffer_size == 0) {
        return 0;
    }

    return snprintf(buffer, buffer_size, "Day %u, %02u:00",
                   resources->day_count, resources->time_hours);
}

const char* resources_get_time_of_day(const Resources* resources) {
    if (!resources) {
        return "unknown";
    }

    uint32_t hour = resources->time_hours;

    if (hour == 0) {
        return "midnight";
    } else if (hour >= 1 && hour < 6) {
        return "night";
    } else if (hour >= 6 && hour < 12) {
        return "morning";
    } else if (hour >= 12 && hour < 18) {
        return "afternoon";
    } else if (hour >= 18 && hour < 22) {
        return "evening";
    } else {
        return "night";
    }
}
