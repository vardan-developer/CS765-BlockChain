## Simulation Workflow

The simulation follows these steps:

1. **Initialization**: The Simulator initializes the network, creating miners and setting up the initial blockchain state.
2. **Event Scheduling**: The Simulator schedules events based on predefined probabilities and network conditions.
3. **Event Processing**: Miners process events, such as creating blocks, validating transactions, and broadcasting updates to the network.
4. **State Update**: Miners update their local blockchain state based on the processed events.
5. **Analysis**: The Simulator collects and presents simulation results, providing insights into the network's behavior.

## Building and Running the Simulation

### Prerequisites

- **C++ Compiler**: A compiler that supports C++23 (e.g., g++).
- **Make**: A build automation tool.
- **Graphviz**: A tool for generating graph visualizations.

### Steps

1.  **Clone the Repository**:

    ```bash
    git clone <repository_url>
    cd <repository_directory>/Part1
    ```

2.  **Compile the Code**:

    ```bash
    make
    ```

    This command compiles the source code using the instructions in the `Makefile`.

3.  **Run the Simulation**:

    ```bash
    make run
    ```

    This command executes the simulation with default parameters and generates output files, including visualizations.

4.  **Analyze Graphs**:

    ```bash
    make analyze
    ```

    This command generates additional graphs for analysis.

5.  **Clean Up**:

    ```bash
    make clean
    ```

    This command removes compiled files and generated outputs.

6.  **Debug Build (Optional)**:

    ```bash
    make g
    ```

    This command builds the simulation with debugging symbols.

## Makefile Settings

The `Makefile` contains several configurable settings:

-   `CXX`: The C++ compiler used for building the project.
-   `CXXFLAGS`: Compiler flags, including the C++ standard and debugging options.
-   Simulation parameters such as `TOTAL_NODES`, `TTX_TIME`, and `BLK_TIME`.

## Contributors

-   [@Vardan Verma](https://github.com/vardan-developer)
-   [@Atharva Bendale](https://github.com/AtharvaBendale)
-   [@Vishal Bysani](https://github.com/Vishal-Bysani) 