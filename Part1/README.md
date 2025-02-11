# CS765 Project Part-1
# Simulation of a P2P Cryptocurrency Network

## Requirements

- **Compiler:** `g++` (supports C++23)
- **Graphviz:** Required for visualizing peer graphs and block trees (`dot` command)

## Compilation and Execution

**Note:** The `Makefile` must be run from the project root directory (not inside `src/`).

### 1. Compile the Code

To build the project, run:

```sh
make
```

This generates an executable named `sim`.

### 2. Run the Simulation

Execute the simulation with:

```sh
make run
```

This will:
- Create `logs/` and `graphs/` directories.
- Run `sim` with predefined parameters.
- Generate visualization graphs.

### 3. Debug Mode

To enable debugging, use after setting `DEBUG` variable inside `Makefile` to 1:

```sh
make g
```

This will compile the code with debug flags.

### 4. Cleanup

To remove compiled files, logs, and graphs run:

```sh
make clean
```

## Simulation Parameters
Manually run the execultable `sim` made after running `make`.:

```sh
./sim  --total-nodes <TOTAL_NODES> --z0 <SLOW> --z1 <LOW_CPU> --ttx-time <TTX_TIME> --blk-time <BLK_TIME> --blk-limit <BLK_LIMIT>
```

OR

You can customize the simulation parameters by modifying the `Makefile` variables:

- `TOTAL_NODES`: Number of nodes in the network (default: `50`).
- `SLOW`: Fraction of slow nodes (default: `0.6`).
- `LOW_CPU`: Fraction of nodes with low computational power (default: `0.4`).
- `TTX_TIME`: Transaction arrival time in ms (default: `10000`).
- `BLK_TIME`: Block mining interval in ms (default: `60000`).
- `BLK_LIMIT`: Max blocks which get into the blockchain tree (default: `10`).
- `TIME_LIMIT`: Total simulation runtime in ms (default: `10000000`).

At least one of `BLK_LIMIT` or `TIME_LIMIT` should be given when executable is run manually.

OR

You can provide customized simulation parameters dynamically by using the `-e` flag with `make run`. This allows you to override the default values set in the `Makefile` without modifying it.  

For example:  

```sh
make run -e TOTAL_NODES=100 SLOW=0.3 LOW_CPU=0.5 BLK_TIME=50000
```

This command will:  
- Set `TOTAL_NODES` to `100` instead of `50`.  
- Set `SLOW` to `0.3` instead of `0.6`.  
- Set `LOW_CPU` to `0.5` instead of `0.4`.  
- Set `BLK_TIME` to `50000` ms instead of `60000` ms.

## Output

- The main simulation output is stored in `miners.txt`.
- The log for every miner is generated in the `logs/` directory with file name `miner-<miner_ID>.logs`
- Blockchain trees' visualizations along with the peer graph are generated in the `graphs/` directory.


## Contributors

-   [@Vardan Verma](https://github.com/vardan-developer)
-   [@Atharva Bendale](https://github.com/AtharvaBendale)
-   [@Vishal Bysani](https://github.com/Vishal-Bysani) 