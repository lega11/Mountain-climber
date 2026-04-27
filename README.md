# Mountain-climber
🏔️ Mountain Peak Solver (Gradient-Based Search in C)

📌 Overview

This project implements a solution to the Mountain Search Problem, where the goal is to find the highest point (peak) in an unknown 2D landscape using limited visibility.

The landscape is hidden, and the program can only access it through a small local window using the function:

generate_view(view, y, x);

The objective is to locate the global maximum while minimizing the number of queries.

🧠 Strategy

The solution uses a gradient-based hill climbing approach with additional logic to handle tricky terrain such as plateaus and loops.

Core Idea:
Look at the current view
Move toward the highest visible point
Repeat until the peak is found
⚙️ Features
⛰️ 1. Gradient Climbing
The algorithm scans the local view and moves toward the highest visible value.
This efficiently follows the slope of the mountain.
🟫 2. Plateau Handling

Plateaus (flat regions) are detected when:

The center of the view is the highest point, but
It is not strictly higher than its neighbors

To escape plateaus:

First move forward
Then move sideways
Finally reset near the best-known location and try a new direction
🧭 3. Direction Estimation

If no clear direction is available, the program:

Compares averages of the edges of the view
Infers which direction is uphill
🔁 4. Loop Detection
The program stores recent positions
If it revisits the same positions, it detects a loop
It then forces a directional change to escape
📍 5. Best Point Tracking
The highest altitude ever seen is always tracked
If the search gets lost or goes out of bounds, it returns to this point
🔍 6. Peak Validation
The algorithm only calls declare_peak() when:
The center is strictly higher than its neighbors
This avoids wasting calls on plateaus
🧪 Performance
Efficient search with low number of queries
Handles large plateaus and noisy terrain
Avoids infinite loops and excessive backtracking
Performs consistently across multiple randomized landscapes
🏗️ Project Structure
.
├── gradient.c        # Provided: landscape generator & evaluator
├── gradient.h        # Provided: definitions and API
├── gradient_sol.c    # Your solution (this project)
└── README.md         # This file
🛠️ Compilation

Compile using GCC:

gcc gradient.c gradient_sol.c -lm -o gradient
▶️ Running the Program
./gradient

This runs the performance_eval() function, which:

Tests the solution on multiple random landscapes
Outputs the number of queries used for each attempt
Prints the average performance
🐞 Debugging

To test a specific failing case:

single_run(SEED);

Example:

single_run(27);

You can also:

Print the full matrix using print_matrix()
Print the path taken using user_path[]
📚 Concepts Used
Hill Climbing (Greedy Search)
Plateau Detection & Escape
Heuristic Direction Estimation
Loop Detection
Local Refinement
