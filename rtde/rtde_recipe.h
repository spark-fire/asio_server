
#define ROBOT_RTDE_INPUT_MAP(R, OFFSET)      \
    RRII(R##_rtde_input_max, OFFSET + 0, int, "")

#define ROBOT_RTDE_OUTPUT_MAP(R, OFFSET)     \
    RRII(R##_message, OFFSET + 0, "")

#define RTDE_INPUT_MAP                        \
    RRII(set_recipe, 100, RtdeRecipe, "Set ") \
    ROBOT_RTDE_INPUT_MAP(R1, 1000)            \
    ROBOT_RTDE_INPUT_MAP(R2, 2000)            \
    ROBOT_RTDE_INPUT_MAP(R3, 3000)            \
    ROBOT_RTDE_INPUT_MAP(R4, 4000)

#define RTDE_OUTPUT_MAP                                              \
    RRII(timestamp, 100, double,                                     \
         "Time elapsed since the controller was started [s]")        \
    RRII(line_number, 101, int, "line number set by setPlanContext") \
    RRII(runtime_state, 102, RuntimeState, "Program state")          \
    ROBOT_RTDE_OUTPUT_MAP(R1, 1000)                                  \
    ROBOT_RTDE_OUTPUT_MAP(R2, 2000)                                  \
    ROBOT_RTDE_OUTPUT_MAP(R3, 3000)                                  \
    ROBOT_RTDE_OUTPUT_MAP(R4, 4000)

#define RRII(i, n, ...) i = n,
enum RtdeInput
{
    RTDE_INPUT_MAP
};
enum RtdeOutput
{
    RTDE_OUTPUT_MAP
};
#undef RRII
