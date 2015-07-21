/*
  +----------------------------------------------------------------------+
  | hawkeye                                                              |
  +----------------------------------------------------------------------+
  | this source file is subject to version 2.0 of the apache license,    |
  | that is bundled with this package in the file license, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/license-2.0.html                      |
  | if you did not receive a copy of the apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | yiming_6weijun@163.com so we can mail you a copy immediately.        |
  +----------------------------------------------------------------------+
  | author: weijun lu  <yiming_6weijun@163.com>                          |
  +----------------------------------------------------------------------+
*/
#include "memory_stat.h"

static float memory_usage_calculate(memory_stat_t *memory_stat);

int memory_stat_get(memory_stat_t *memory_stat)
{
	FILE *fp;
	char line[512] = {0};

    bzero(memory_stat, sizeof(memory_stat_t));

	if ((fp = fopen(PROC_MEMORY_STAT, "r")) == NULL) {
        log_error("open %s failed, error: %s.", PROC_MEMORY_STAT, strerror(errno));
		return 0;
	}

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "MemTotal:", 9)) {
			/* Read the total amount of memory in kB */
			sscanf(line + 9, "%lu", &memory_stat->total);
		}
		else if (!strncmp(line, "MemFree:", 8)) {
			/* Read the amount of free memory in kB */
			sscanf(line + 8, "%lu", &memory_stat->free);
		}
		else if (!strncmp(line, "Buffers:", 8)) {
			/* Read the amount of buffered memory in kB */
			sscanf(line + 8, "%lu", &memory_stat->buffered);
		}
		else if (!strncmp(line, "Cached:", 7)) {
			/* Read the amount of cached memory in kB */
			sscanf(line + 7, "%lu", &memory_stat->cached);
		}
		else if (!strncmp(line, "Active:", 7)) {
			/* Read the amount of Active memory in kB */
			sscanf(line + 7, "%lu", &memory_stat->active);
		}
		else if (!strncmp(line, "Inactive:", 9)) {
			/* Read the amount of Inactive memory in kB */
			sscanf(line + 9, "%lu", &memory_stat->inactive);
		}
		else if (!strncmp(line, "Slab:", 5)) {
			/* Read the amount of Slab memory in kB */
			sscanf(line + 5, "%lu", &memory_stat->slab);
		}
		else if (!strncmp(line, "SwapCached:", 11)) {
			/* Read the amount of cached swap in kB */
			sscanf(line + 11, "%lu", &memory_stat->swap_cached);
		}
		else if (!strncmp(line, "SwapTotal:", 10)) {
			/* Read the total amount of swap memory in kB */
			sscanf(line + 10, "%lu", &memory_stat->total_swap);
		}
		else if (!strncmp(line, "SwapFree:", 9)) {
			/* Read the amount of free swap memory in kB */
			sscanf(line + 9, "%lu", &memory_stat->free_swap);
		}
		else if (!strncmp(line, "Committed_AS:", 13)) {
			/* Read the amount of commited memory in kB */
			sscanf(line + 13, "%lu", &memory_stat->commited);
		}
	}

	fclose(fp);
	return 1;
}


int memory_stat_json(memory_stat_t *memory_stat, char *json_buffer, int size)
{
    char *fmt_str = NULL;
    int len = 0;
    
    fmt_str = "memory={\"free\":%lu,\"cached\":%lu,\"buffered\":%lu,\"total\":%lu,\"usage\":%.3f}";

    len = snprintf(json_buffer, size, fmt_str, 
                memory_stat->free,
                memory_stat->cached,
                memory_stat->buffered,
                memory_stat->total,
                memory_usage_calculate(memory_stat));
    if (len < size) {
        return 1;
    }
    else {
        log_error("memory_stat_json: json_buffer is too small.");
        return 0;
    }
}

static float memory_usage_calculate(memory_stat_t *memory_stat)
{
    return 1 - (float)(memory_stat->free + memory_stat->cached + memory_stat->buffered)/memory_stat->total;
}

void memory_stat_dump(memory_stat_t *memory_stat)
{
    log_info("memory stat:");
    log_info("MemoryUsage:  %.3f",  memory_usage_calculate(memory_stat));
    log_info("MemTotal:     %10d kb", memory_stat->total);
    log_info("MemFree:      %10d kb", memory_stat->free);
    log_info("Buffers:      %10d kb", memory_stat->buffered);
    log_info("Cached:       %10d kb", memory_stat->cached);
    log_info("Active:       %10d kb", memory_stat->active);
    log_info("Inactive:     %10d kb", memory_stat->inactive);
    log_info("Slab:         %10d kb", memory_stat->slab);
    log_info("SwapFree:     %10d kb", memory_stat->free_swap);
    log_info("SwapTotal:    %10d kb", memory_stat->total_swap);
    log_info("SwapCached:   %10d kb", memory_stat->swap_cached);
    log_info("Committed_AS: %10d kb", memory_stat->commited);
}

#ifdef _DEBUG 
int main(void) 
{
    memory_stat_t stat;

    memory_stat_get(&stat);
    memory_stat_dump(&stat);
    return 1;
}
#endif
