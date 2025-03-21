# HPC
Características del PC donde se realizaron las pruebas:
● Procesador: AMD Ryzen™ 7 5700X
● Cantidad de núcleos: 8
● Cantidad de subprocesos/hilos: 16
● Frecuencia básica del procesador: 3.4GHz
● Frecuencia Turbo máxima: 4.6GHz
● Ram 32GB DDR4 @ 3200 MHz
● NVME: 1TB R: 5000 MB/s - W: 4400 MB/s
● GPU: AMD RX 7800 XT
● Sistema operativo: Ubuntu 24.04.2 LTS


Casos de estudio de HPC
Caso 1: Multiplicacion de matrices.
Se utilizan tres programas distintos, secuencial, paralelismo con hilos, y paralelismo con procesos.
Para cada implementacion se recolectan datos de una matriz cuadrada de 500, 1000, 1500, 2000, 3500, 5000 celdas.
Se hacen 10 pruebas para cada una de estas dimensiones.
