#include "gradient.h"

/* Returns -1, 0, or 1 depending on value of int */
static int sign_int(int v) {
    if (v > 0) return 1;
    if (v < 0) return -1;
    return 0;
}

/* Abs value */
static int abs_int(int v) {
    return (v < 0) ? -v : v;
}

/* Prevent going below 0 */
static int clamp_nonnegative(int v) {
    return (v < 0) ? 0 : v;
}

/*
   Try to guess which direction the mountain is going up
   by comparing the edges of the current view
*/
static void infer_direction_from_edges(float view[VIEW_SIZE][VIEW_SIZE], int *dx, int *dy) {
    float top = 0.0f, bottom = 0.0f, left = 0.0f, right = 0.0f;
    int top_n = 0, bottom_n = 0, left_n = 0, right_n = 0;

    /* Sum values on each edge */
    for (int i = 0; i < VIEW_SIZE; i++) {
        if (view[0][i] >= 0) { top += view[0][i]; top_n++; }
        if (view[VIEW_SIZE - 1][i] >= 0) { bottom += view[VIEW_SIZE - 1][i]; bottom_n++; }
        if (view[i][0] >= 0) { left += view[i][0]; left_n++; }
        if (view[i][VIEW_SIZE - 1] >= 0) { right += view[i][VIEW_SIZE - 1]; right_n++; }
    }

    /* Get averages */
    if (top_n > 0) top /= top_n;
    if (bottom_n > 0) bottom /= bottom_n;
    if (left_n > 0) left /= left_n;
    if (right_n > 0) right /= right_n;

    /* Compare edges to estimate slope direction */
    *dy = sign_int((int)((bottom - top) * 1000));
    *dx = sign_int((int)((right - left) * 1000));

    /* Default direction if everything is flat */
    if (*dx == 0 && *dy == 0) {
        *dx = 1;
        *dy = 0;
    }
}

/*
   Check if the center is strictly higher than all its neighbors.
   If not, we are probably on a plateau.
*/
static int center_strictly_beats_neighbors(float view[VIEW_SIZE][VIEW_SIZE]) {
    float c = view[VIEW_RADIUS][VIEW_RADIUS];

    if (c < 0.0f) return 0;

    for (int i = VIEW_RADIUS - 1; i <= VIEW_RADIUS + 1; i++) {
        for (int j = VIEW_RADIUS - 1; j <= VIEW_RADIUS + 1; j++) {
            if (i == VIEW_RADIUS && j == VIEW_RADIUS) continue;
            if (view[i][j] >= 0.0f && view[i][j] >= c) {
                return 0;
            }
        }
    }
    return 1;
}

/* Check if we have recently visited this position */
static int in_recent_history(int x, int y, int hx[8], int hy[8]) {
    for (int i = 0; i < 8; i++) {
        if (hx[i] == x && hy[i] == y) return 1;
    }
    return 0;
}

path_point find_highest_point() {
    float view[VIEW_SIZE][VIEW_SIZE];

    /* Start at (0,0) */
    int x = 0;
    int y = 0;

    /* Keep track of best point we have ever seen */
    int best_seen_x = 0;
    int best_seen_y = 0;
    float best_seen_val = -1.0f;

    /* Last direction we moved in */
    int last_dx = 1;
    int last_dy = 0;

    /* Count how many times we hit a plateau */
    int plateau_hits = 0;

    /* Count how many times we loop */
    int loop_hits = 0;

    /* Store last few positions to detect loops */
    int hist_x[8];
    int hist_y[8];

    for (int i = 0; i < 8; i++) {
        hist_x[i] = -999999;
        hist_y[i] = -999999;
    }

    path_point ret;
    ret.x = 0;
    ret.y = 0;

    /* Main search loop */
    for (int step = 0; step < 900; step++) {

        /* Get local view */
        generate_view(view, y, x);

        /* Check if we are looping */
        if (in_recent_history(x, y, hist_x, hist_y)) {
            loop_hits++;
        } else {
            loop_hits = 0;
        }

        /* Update history */
        for (int i = 7; i > 0; i--) {
            hist_x[i] = hist_x[i - 1];
            hist_y[i] = hist_y[i - 1];
        }
        hist_x[0] = x;
        hist_y[0] = y;

        /* Get value at center */
        float center = view[VIEW_RADIUS][VIEW_RADIUS];

        /* Update best seen point */
        if (center > best_seen_val) {
            best_seen_val = center;
            best_seen_x = x;
            best_seen_y = y;
        }

        /* Find highest point in current view */
        int best_i = VIEW_RADIUS;
        int best_j = VIEW_RADIUS;
        float best_val = center;

        int valid_count = 0;
        int invalid_count = 0;

        for (int i = 0; i < VIEW_SIZE; i++) {
            for (int j = 0; j < VIEW_SIZE; j++) {
                if (view[i][j] < 0.0f) {
                    invalid_count++;
                    continue;
                }

                valid_count++;

                if (view[i][j] > best_val) {
                    best_val = view[i][j];
                    best_i = i;
                    best_j = j;
                }
            }
        }

        /* Convert view coordinates to real coordinates */
        int target_x = x + (best_j - VIEW_RADIUS);
        int target_y = y + (best_i - VIEW_RADIUS);

        /* Update global best */
        if (best_val > best_seen_val) {
            best_seen_val = best_val;
            best_seen_x = target_x;
            best_seen_y = target_y;
        }

        /* If center is best = peak or plateau */
        if (best_i == VIEW_RADIUS && best_j == VIEW_RADIUS) {

            /* Only declare if clearly a peak */
            if (center_strictly_beats_neighbors(view)) {
                if (declare_peak(x, y)) {
                    ret.x = x;
                    ret.y = y;
                    return ret;
                }
            }

            plateau_hits++;

            /* If no direction, estimate one */
            if (last_dx == 0 && last_dy == 0) {
                infer_direction_from_edges(view, &last_dx, &last_dy);
            }

            /* Try different ways to escape plateau */
            if (plateau_hits == 1) {
                /* Go forward */
                x += last_dx * (2 * VIEW_SIZE);
                y += last_dy * (2 * VIEW_SIZE);
            }
            else if (plateau_hits == 2) {
                /* Go forward + sideways */
                int pdx = -last_dy;
                int pdy = last_dx;
                x += last_dx * (2 * VIEW_SIZE) + pdx * VIEW_SIZE;
                y += last_dy * (2 * VIEW_SIZE) + pdy * VIEW_SIZE;
            }
            else {
                /* Reset near best known point and try new direction */
                int pdx = last_dy;
                int pdy = -last_dx;

                x = best_seen_x + pdx * VIEW_SIZE;
                y = best_seen_y + pdy * VIEW_SIZE;

                infer_direction_from_edges(view, &last_dx, &last_dy);
                plateau_hits = 0;
            }

            x = clamp_nonnegative(x);
            y = clamp_nonnegative(y);
            continue;
        }

        /* Not plateau = normal movement */
        plateau_hits = 0;

        int move_dx = best_j - VIEW_RADIUS;
        int move_dy = best_i - VIEW_RADIUS;

        last_dx = sign_int(move_dx);
        last_dy = sign_int(move_dy);

        x += move_dx;
        y += move_dy;

        x = clamp_nonnegative(x);
        y = clamp_nonnegative(y);

        /* If out of bounds, go back to best known */
        if (invalid_count > valid_count) {
            x = best_seen_x;
            y = best_seen_y;
        }

        /* If stuck in loop, jump sideways */
        if (loop_hits >= 2) {
            int pdx = -last_dy;
            int pdy = last_dx;

            x = best_seen_x + pdx * VIEW_SIZE;
            y = best_seen_y + pdy * VIEW_SIZE;

            x = clamp_nonnegative(x);
            y = clamp_nonnegative(y);
            loop_hits = 0;
        }
    }

    /* Final fallback: return best seen point */
    ret.x = best_seen_x;
    ret.y = best_seen_y;
    return ret;
}