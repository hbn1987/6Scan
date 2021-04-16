#include "6scan.h"

Traceroute::Traceroute(ScanConfig *_config, Stats *_stats) : config(_config), stats(_stats)
{
    dstport = config->dstport;
    gettimeofday(&start, NULL);
    // RFC2822 timestring
    struct tm *p = localtime(&(start.tv_sec));
    char s[1000];
    strftime(s, 1000, "%a, %d %b %Y %T %z", p);
    config->set("Start", s, true);
    pthread_mutex_init(&recv_lock, NULL);
}

Traceroute::~Traceroute() {
    gettimeofday(&start, NULL);
    fflush(NULL);
    pthread_cancel(recv_thread);
    if (config->out)
        fclose(config->out);
}

uint32_t
Traceroute::elapsed() {
    gettimeofday(&now, NULL);
    return tsdiffus(&now, &start); 
}

void
Traceroute::lock() {
    pthread_mutex_lock(&recv_lock);
}

void
Traceroute::unlock() {
    pthread_mutex_unlock(&recv_lock);
}
