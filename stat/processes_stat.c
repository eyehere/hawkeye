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
#include "processes_stat.h"

typedef int (*pid_cb_t)(char *pid);

static int traverse_by_process_name(process_group_stat_t *process_group, char *process_name, int *total_process_count);
static int get_process_group_stat(single_process_stat_t *stat, char *pid);
static int process_group_stat_json(process_group_stat_t *process_group_stat, char *json_buffer, int size, int index);

int processes_stat_get(processes_stat_t *processes_stat, char **processes, int count)
{
    int i = 0;
    int total_process_count = 0;

    bzero(processes_stat, sizeof(processes_stat_t));

    if (count > MAX_PROCESS_GROUP_COUNT) {
        log_error("max process group count is %d, set to it", MAX_PROCESS_GROUP_COUNT);
        count = MAX_PROCESS_GROUP_COUNT;
    }

    for (i = 0; i < count; i++) {
        process_group_stat_t *process_group_stat = NULL;

        process_group_stat = &processes_stat->process_groups[i];
        bzero(process_group_stat, sizeof(process_group_stat_t));
        
        strncpy(processes_stat->process_groups[i].name, processes[i], MAX_PROCESS_NAME_SIZE - 1);
        if (!traverse_by_process_name(process_group_stat, processes[i], &total_process_count)) {
            log_error("traverse_by_process_name failed.");
        }

        processes_stat->process_group_count++;
        processes_stat->total_count = total_process_count;
    }
    return 1;
}

static int traverse_by_process_name(process_group_stat_t *process_group, char *process_name, int *total_process_count)
{
    DIR *dir = NULL;
    struct dirent *d;

    dir = opendir("/proc");
    if (NULL == dir) {
        log_error("can't open proc directory.");
        return 0;
    }

    *total_process_count = 0;
    while (NULL != (d = readdir(dir))) {
        char exe_path[MAX_PATH_SIZE] = {0};
        char path[MAX_PATH_SIZE]     = {0};
        int  len = 0;
        char *str = NULL;

        if (process_group->count >= MAX_PROCESS_COUNT) {
            log_error("max single process is %d, then break.", MAX_PROCESS_COUNT);
            break;
        }

        if (0 == strcmp(".", d->d_name) ||
            0 == strcmp("..", d->d_name)) {
            continue;
        }

        if (isdigit(*d->d_name)) {
            (*total_process_count)++;
        }

        len = snprintf(exe_path, sizeof(exe_path), "/proc/%s/exe", d->d_name);
        if (len >= (int)sizeof(exe_path)) {
            closedir(dir);
            log_error("%s is too long result in exe_path overflow.", d->d_name);
            return 0;
        }

        len = readlink(exe_path, path, sizeof(path) - 1); 
        if (len < 0) {
            continue;
        }
        
        str = strrchr(path, '/');
        if (NULL == str) {
            log_error("strrchr: /  for path failed.");
            continue;
        }
        str++;

        if (0 == strcmp(str, process_name)) {
            log_debug("pid: %s, process: %s.", d->d_name, process_name);
            if (get_process_group_stat(&process_group->processes[process_group->count], d->d_name)) {
                process_group->count++;
            }
            else {
                log_error("get process stat failed.");
            }
        }
    }
    
    closedir(dir);
    return 1;
}

static int get_process_group_stat(single_process_stat_t *stat, char *pid)
{
    char  path[MAX_PATH_SIZE] = {0}; 
    int   len = 0;
    FILE *fp = NULL;
    char  line[4096] = {0};
    char *str = NULL;

    bzero(stat, sizeof(single_process_stat_t));
    len = snprintf(path, sizeof(path), "/proc/%s/status", pid);
    if (len >= (int)sizeof(path)) {
        log_error("get_process_group_stat: path memory is too small."); 
        return 0;
    }

    fp = fopen(path, "r");
    if (NULL == fp) {
        log_error("open %s failed, error: %s.", path, strerror(errno));
        return 0;
    }

    strncpy(stat->pid, pid, MAX_PID_SIZE - 1); 
    while (fgets(line, sizeof(line) - 1, fp) != NULL) {
        if (!strncmp(line, "State:", 6)) {
            str = line + 6; 
            while (*str == ' ' || *str == '\t') {
                str++;
            }

            if (*str == 0) {
                log_error("State: can't find stat char.");
                fclose(fp);
                return 0;
            }
            stat->state = *str;
        }
        else if (!strncmp(line, "VmSize:", 7)) {
            char unit[32]; 
            str = line + 7;

            sscanf(str, "%llu %s", &stat->virt, unit);
            if (unit[0] == 'K' || unit[0] == 'k') {
            }
            else if (unit[0] == 'M' || unit[0] == 'm') {
                stat->virt = stat->virt * 1024;
            }
            else if (unit[0] == 'G' || unit[0] == 'g') {
                stat->virt = stat->virt * 1024 * 1024;
            }
            else {

            }
        }
        else if (!strncmp(line, "VmRSS:", 6)) {
            char unit[32]; 
            str = line + 6;

            sscanf(str, "%llu %s", &stat->res, unit);
            if (unit[0] == 'K' || unit[0] == 'k') {
            }
            else if (unit[0] == 'M' || unit[0] == 'm') {
                stat->res = stat->res * 1024;
            }
            else if (unit[0] == 'G' || unit[0] == 'g') {
                stat->res = stat->res * 1024 * 1024;
            }
            else {

            }
        }
        else {

        }

        bzero(line, sizeof(line));
    }

    fclose(fp);
    return 1;
}

int processes_stat_json(processes_stat_t *processes_stat, char *json_buffer, int size)
{
    int i         = 0;
    int offset    = 0;
    int else_size = size - 1;
    process_group_stat_t *process_group_stat = NULL;
    char *fmt = NULL;
    char buffer[4096] = {0};
    int len = 0;
     
    fmt = "process={\"count\":%d";
    len = snprintf(buffer, sizeof(buffer) - 1, fmt, processes_stat->total_count);
    if (len >= else_size) {
        log_error("json_buffer is tool small.");
        return 0;
    }
    memcpy(json_buffer + offset, buffer, len);
    offset    += len;
    else_size -= len;

    for (i = 0; i < processes_stat->process_group_count; i++) {
        bzero(buffer, sizeof(buffer));
        process_group_stat = &processes_stat->process_groups[i];

        len = process_group_stat_json(process_group_stat, buffer, sizeof(buffer), i);
        if (len <= 0) {
            log_error("process_group_stat_json failed.");
            return 0;
        }
        
        if (else_size < len) {
            log_error("json_buffer is too small.");
            return 0;
        }

        memcpy(json_buffer + offset, buffer, len);
        offset    += len;
        else_size -= len;
    }

    if (else_size > 0) {
        json_buffer[offset] = '}';
        offset    += 1;
        else_size -= 1;
    }

    return 1;
}

static int process_group_stat_json(process_group_stat_t *process_group_stat, char *json_buffer, int size, int index)
{
    int i         = 0;
    int offset    = 0;
    int else_size = size - 1;
    int len       = 0;
    char *fmt     = NULL;

    bzero(json_buffer, size); 

    if (process_group_stat->count == 0) {
        char buffer[4096] = {0};

        fmt = ",\"%d\":{\"name\":\"%s\",\"stat\":[]}";
        len = snprintf(buffer, sizeof(buffer) - 1, fmt, index + 1, process_group_stat->name);
        if (len >= else_size) {
            log_error("json_buffer is not enough.");
            return 0;
        }

        memcpy(json_buffer + offset, buffer, len);
        offset    += len;
        else_size -= len;
        return len;
    }

    for (i = 0; i < process_group_stat->count; i++) {
        char buffer[4096]           = {0};
        single_process_stat_t *stat = NULL;

        if (i == 0) {
            if (process_group_stat->count == 1) {
                fmt = ",\"%d\":{\"name\":\"%s\",\"stat\":[{\"pid\":\"%s\",\"state\":\"%c\",\"virt\":%llu,\"res\":%llu}]}";
            }
            else {
                fmt = ",\"%d\":{\"name\":\"%s\",\"stat\":[{\"pid\":\"%s\",\"state\":\"%c\",\"virt\":%llu,\"res\":%llu}";
            }
            stat = &process_group_stat->processes[i];
            len = snprintf(buffer, sizeof(buffer) - 1, fmt, index + 1, process_group_stat->name, 
                    stat->pid, stat->state, stat->virt, stat->res);
            if (len >= else_size) {
                log_error("json_buffer is not enough.");
                return 0;
            }
        }
        else if (i == process_group_stat->count - 1) {
            fmt = ",{\"pid\":\"%s\",\"state\":\"%c\",\"virt\":%llu,\"res\":%llu}]}";
            stat = &process_group_stat->processes[i];
            len = snprintf(buffer, sizeof(buffer) - 1, fmt, stat->pid, stat->state, stat->virt, stat->res);
            if (len >= else_size) {
                log_error("json_buffer is not enough.");
                return 0;
            }
        }
        else {
            fmt = ",{\"pid\":\"%s\",\"state\":\"%c\",\"virt\":%llu,\"res\":%llu}";
            stat = &process_group_stat->processes[i];
            len = snprintf(buffer, sizeof(buffer) - 1, fmt, stat->pid, stat->state, stat->virt, stat->res);
            if (len >= else_size) {
                log_error("json_buffer is not enough.");
                return 0;
            }
        }

        memcpy(json_buffer + offset, buffer, len);
        offset    += len;
        else_size -= len;
    }

    return offset;
}

#ifdef _DEBUG

int main(void) 
{
    char *process_names[] = {"nginx", "syslog-ng"};
    char buffer[4096] = {0};

    processes_stat_t stat;
    processes_stat_get(&stat, process_names, 2);
    processes_stat_json(&stat, buffer, sizeof(buffer));
    printf("%d----%s\n", strlen(buffer), buffer);
    return 1;
}

#endif
