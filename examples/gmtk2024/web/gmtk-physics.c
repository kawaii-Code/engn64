// in
typedef struct {
    Rect_Int8 before;
    Rect_Int8 after;
} Physics_Step;

// out
typedef struct {
    Entity    other;
    Rect_Int8 other_rect;
    uint32    param; // user data (- _ -)
} Physics_Conflict;

typedef struct {
    bool8            has_conflict;

    int32            conflicts_count;
    Physics_Conflict conflicts[16];
} Simulate_Step_Result;



bool8 rects_equal(Rect_Int8 r1, Rect_Int8 r2) {
    return r1.x == r2.x && r1.y == r2.y &&
           r1.width == r2.width && r1.height == r2.height;
}

bool8 rect_intersect_tilemap_out(Rect_Int8 rect, Tilemap *tilemap, Rect_Int8 *out_tile) {
    Tilemap_Solid_Blocks_Iterator it = {0};
    do {
        if (tilemap->tiles[it.y * TILEMAP_WIDTH + it.x] == TILEMAP_TILE_TYPE_BLOCK) {
            Rect_Int8 tile_rect = tilemap_tile_rect(it.x, it.y);
            if (rect_intersect_rect_int8(rect, tile_rect)) {
                *out_tile = tile_rect;
                return true;
            }
        }
    } while (advance_iterator(&it));

    return false;
}

Simulate_Step_Result simulate_step(Level *level, Physics_Step step) {
    Simulate_Step_Result result = {0};

    Rect_Int8 before = step.before;
    Rect_Int8 after  = step.after;

    Rect_Int8 tile;
    if (rect_intersect_tilemap_out(after, &level->tilemap, &tile)) {
        result.has_conflict = true;
        result.conflicts[result.conflicts_count++] = (Physics_Conflict) {
            .other  = ENTITY_TILE,
            .other_rect = tile,
        };
    }

    if (after.x + after.width  > 64 ||
        after.y + after.height > 64 ||
        after.x < 0)
    {
        result.has_conflict = true;
        result.conflicts[result.conflicts_count++] = (Physics_Conflict) {
            .other = SCREEN,
        };
    }

    for (uint32 i = 0; i < level->boxes_length; i++) {
        Box *box = &level->boxes[i];
        Rect_Int8 rect = box->rect;
        if (rects_equal(before, rect)) {
            continue;
        }
        if (rect_intersect_rect_int8(after, rect)) {
            result.has_conflict = true;
            result.conflicts[result.conflicts_count++] = (Physics_Conflict) {
                .other = ENTITY_BOX,
                .other_rect = rect,
                .param = i,
            };
        }
    }

    if (!rects_equal(level->player.rect, before) && rect_intersect_rect_int8(level->player.rect, after)) {
        result.has_conflict = true;
        result.conflicts[result.conflicts_count++] = (Physics_Conflict) {
            .other = ENTITY_PLAYER,
            .other_rect = level->player.rect,
        };
    }

    return result;
}
