#include <glib.h>

gpointer func(gpointer data) {
    return NULL;
}

int main(int argc, char *argv[])
{
    g_thread_init(NULL);
    GThread *thread = g_thread_new("thread", func, NULL);
    g_thread_join(thread);
    return 0;
}
