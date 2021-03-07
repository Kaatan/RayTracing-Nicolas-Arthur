@echo off
echo OMP 200 images > results/benchmark.csv
echo Threads, Temps, FPS >> results/benchmark.csv
FOR /L %%i IN (1,1,16) DO (
	SET OMP_NUM_THREADS=%%i
	rt.exe -bm -aa >> results/benchmark.csv
)