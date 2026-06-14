#!/bin/bash

# --- Configuration ------------------------------------------------------------
PROJECT_ROOT=$(realpath "$(dirname "$0")/..")
RESULTS_DIR="$PROJECT_ROOT/build/benchmark"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RUN_DIR="${RESULTS_DIR}/${TIMESTAMP}"
mkdir -p "${RUN_DIR}"
mkdir -p "${RESULTS_DIR}"

DATA_DIR="${PROJECT_ROOT}/test/data"
HFM_BIN="${PROJECT_ROOT}/build/src/hfm"

COMPRESSORS=("hfm" "gzip" "bzip2" "xz" "zstd")
HYPERFINE_WARMUP=3

# --- Helper Functions ---------------------------------------------------------
run_hyperfine() {
    local test_file="$1"
    local filename=$(basename "$test_file")
    local output_json="${RUN_DIR}/${filename}.json"
    local hyperfine_args=("--warmup" "$HYPERFINE_WARMUP" "--export-json" "$output_json")

    for comp in "${COMPRESSORS[@]}"; do
        case $comp in
            hfm)
                if [[ ! -x "$HFM_BIN" ]]; then
                    echo "Warning: HFM binary not found at $HFM_BIN. Skipping."
                    continue
                fi
                rm -f "${RUN_DIR}/${filename}.hfm"
                hyperfine_args+=("\"$HFM_BIN\" \"$test_file\" -o \"${RUN_DIR}/${filename}.hfm\"")
                ;;
            gzip)
                hyperfine_args+=("gzip -4 -c \"$test_file\" > \"${RUN_DIR}/${filename}.gz\"")
                ;;
            bzip2)
                hyperfine_args+=("bzip2 -4 -c \"$test_file\" > \"${RUN_DIR}/${filename}.bz2\"")
                ;;
            xz)
                hyperfine_args+=("xz -4 -c \"$test_file\" > \"${RUN_DIR}/${filename}.xz\"")
                ;;
            zstd)
                hyperfine_args+=("zstd -f -c \"$test_file\" > \"${RUN_DIR}/${filename}.zst\"")
                ;;
        esac
    done

    if [ ${#hyperfine_args[@]} -le 5 ]; then
        echo "No commands to run for $filename. Skipping."
        return
    fi

    echo "Running hyperfine for $filename..."
    hyperfine "${hyperfine_args[@]}"
}

# --- Main Execution -----------------------------------------------------------
echo "Starting benchmark at $(date)"
echo "Results will be saved in: ${RUN_DIR}" 

for test_file in "${DATA_DIR}"/*; do
    if [ -f "$test_file" ]; then
        run_hyperfine "$test_file"
        echo "-----------------------------------"
    fi
done

cp $DATA_DIR/* $RUN_DIR/ 

echo "Benchmark finished at $(date)."
