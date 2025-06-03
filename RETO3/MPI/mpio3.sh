#!/bin/bash

echo "-----------------------------------------------------"
echo "Iniciando pruebas para jacobi1dMPIo3 (con optimización -O3)"
echo "-----------------------------------------------------"

# --- Configuración ---
NP=4
HOSTFILE="../mycluster.txt"
MPI_DIR_PATH="/home/cluser/RETO3/MPI" # Asegúrate que 'cluser' sea tu nombre de usuario
EXECUTABLE_NAME="jacobi1dMPIo3" # <--- Nombre del ejecutable optimizado
FULL_EXEC_PATH="$MPI_DIR_PATH/$EXECUTABLE_NAME"

OUT_DIR="$MPI_DIR_PATH/OUT"
OUT_FILE="$OUT_DIR/resultadoso3.txt" # <--- Archivo de salida diferente

mkdir -p "$OUT_DIR"

# Opcional: Limpiar el archivo de resultados anterior
# > "$OUT_FILE"

if [ ! -f "$FULL_EXEC_PATH" ]; then
    echo "Error: El ejecutable $FULL_EXEC_PATH no se encontró."
    echo "Por favor, compila jacobi1dMPI.c primero con: mpicc -o $EXECUTABLE_NAME jacobi1dMPI.c timing.c -DCLOCK_REALTIME -O3 -lm"
    exit 1
fi

if [ ! -f "$HOSTFILE" ]; then
    echo "Error: El archivo de hosts $HOSTFILE no se encontró en la ubicación esperada."
    exit 1
fi

# --- Casos de Prueba (N NSTEPS) ---
declare -a test_cases=(
    "100000 1000"
    "100000 10000"
    "1000000 1000"
    "1000000 10000"
    "10000000 10000"
    "10000000 100000"
)

# --- Bucle de Ejecución ---
for case_params in "${test_cases[@]}"; do
    read -r N NSTEPS <<< "$case_params"

    echo ""
    echo "Ejecutando $EXECUTABLE_NAME con N=$N, NSTEPS=$NSTEPS ..."
    
    mpiexec -n "$NP" --hostfile "$HOSTFILE" "$FULL_EXEC_PATH" "$N" "$NSTEPS" "$OUT_FILE"
    
    if [ $? -ne 0 ]; then
        echo "ERROR durante la ejecución de mpiexec para N=$N, NSTEPS=$NSTEPS con $EXECUTABLE_NAME."
        # exit 1 
    else
        echo "Completado: N=$N, NSTEPS=$NSTEPS"
    fi
    echo "----------------------------------------"
done

echo ""
echo "Todas las pruebas para $EXECUTABLE_NAME han finalizado."
echo "Los resultados se encuentran en: $OUT_FILE"
echo "-----------------------------------------------------"