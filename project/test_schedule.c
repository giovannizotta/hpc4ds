#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>

int main() {
#pragma omp parallel
    {
        omp_sched_t schedule;
        int chunk_size;
#pragma omp parallel for
        for (int i = 0; i < 1; i++) {
            if (omp_get_thread_num() == 0)
                omp_get_schedule(&schedule, &chunk_size);
        }
        if (omp_get_thread_num() == 0) {
            printf("Number of threads = %d / %d\n", omp_get_num_threads(),
                   omp_get_num_procs());

            char scheduleStr[15];
            switch (schedule) {
            case omp_sched_static:
                strcpy(scheduleStr, "static");
                break;
            case omp_sched_dynamic:
                strcpy(scheduleStr, "dynamic");
                break;
            case omp_sched_guided:
                strcpy(scheduleStr, "guided");
                break;
            case omp_sched_auto:
                strcpy(scheduleStr, "auto");
                break;
            default:
                strcpy(scheduleStr, "monotonic");
                break;
            }
            printf("Default schedule: %s, %d\n", scheduleStr, chunk_size);
        }
    }

    printf("----------------------------------\n");

    return 0;
}
