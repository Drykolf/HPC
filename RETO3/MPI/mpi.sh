#!/bin/bash

echo "-----------------------------------------------------"
echo "Iniciando pruebas para jacobi1dMPI (sin optimización -O3)"
echo "-----------------------------------------------------"

# --- Configuración ---
NP=4  # Número de procesos MPI (ajusta según tu clúster, 4 para head + 3 workers)

# Asumiendo que este script está en ~/RETO3/MPI/ y mycluster.txt está en ~/RETO3/
HOSTFILE="../mycluster.txt"

# Ruta absoluta al directorio donde están los ejecutables MPI
MPI_DIR_PATH="/home/cluser/RETO3/MPI" # Asegúrate que 'cluser' sea tu nombre de usuario
EXECUTABLE_NAME="jacobi1dMPI"
FULL_EXEC_PATH="$MPI_DIR_PATH/$EXECUTABLE_NAME"

# Directorio y archivo de salida (dentro de ~/RETO3/MPI/OUT/)
OUT_DIR="$MPI_DIR_PATH/OUT"
OUT_FILE="$OUT_DIR/resultados.txt"

# Crear directorio de salida si no existe
mkdir -p "$OUT_DIR"

# Opcional: Limpiar el archivo de resultados anterior para empezar de cero cada vez que corras el script
# Si quieres esto, descomenta la siguiente línea:
# > "$OUT_FILE"
# Nota: Tu programa C actualmente añade (append) al archivo de resultados.

# Verificar si el ejecutable existe
if [ ! -f "$FULL_EXEC_PATH" ]; then
    echo "Error: El ejecutable $FULL_EXEC_PATH no se encontró."
    echo "Por favor, compila jacobi1dMPI.c primero con: mpicc -o $EXECUTABLE_NAME jacobi1dMPI.c timing.c -DCLOCK_REALTIME -lm"
    exit 1
fi

# Verificar si el hostfile existe
if [ ! -f "$HOSTFILE" ]; then
    echo "Error: El archivo de hosts $HOSTFILE no se encontró en la ubicación esperada."
    exit 1
fi

# --- Casos de Prueba (N NSTEPS) ---
# Formato: "N_VALOR NSTEPS_VALOR"
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
    # Extraer N y NSTEPS del string del caso
    read -r N NSTEPS <<< "$case_params"

    echo ""
    echo "Ejecutando $EXECUTABLE_NAME con N=$N, NSTEPS=$NSTEPS ..."
    
    # Comando mpiexec: mpiexec [opciones] <ejecutable> <arg_N> <arg_NSTEPS> <arg_archivo_salida>
    mpiexec -n "$NP" --hostfile "$HOSTFILE" "$FULL_EXEC_PATH" "$N" "$NSTEPS" "$OUT_FILE"
    
    # Verificar si mpiexec tuvo éxito
    if [ $? -ne 0 ]; then
        echo "ERROR durante la ejecución de mpiexec para N=$N, NSTEPS=$NSTEPS con $EXECUTABLE_NAME."
        echo "Revisa los mensajes de error. Puedes detener el script aquí si lo deseas."
        # exit 1 # Descomenta si quieres que el script se detenga ante el primer error.
    else
        echo "Completado: N=$N, NSTEPS=$NSTEPS"
    fi
    echo "----------------------------------------"
done

echo ""
echo "Todas las pruebas para $EXECUTABLE_NAME han finalizado."
echo "Los resultados se encuentran en: $OUT_FILE"
echo "-----------------------------------------------------"