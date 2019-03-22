/* Header file for Hyper_Neon */

// Colour struct
struct RGB { 
    float r;
    float g;
    float b;
};

/* Predefined colors
 * Remember that high->no color and low->much color
 */
 
RGB red_max = {0,255,255};
RGB green_max = {255, 0, 255};
RGB blue_max = {255, 255, 0};

RGB yellow_max = {0, 0, 255};
RGB purple_max = {0, 255, 0};
RGB teal_max = {255, 0, 0};

RGB white_max = {0, 0, 0};
RGB off = {255, 255, 255};

static RGB all_colors[] = {
    red_max,
    yellow_max,
    green_max,
    purple_max,
    blue_max,
    teal_max    
};
