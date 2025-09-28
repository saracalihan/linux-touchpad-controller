#ifndef MACRO_ENGINE_H
#define MACRO_ENGINE_H

void init_macro_engine();
void cleanup_macro_engine();
void* macro_engine_thread(void* arg);

#endif // MACRO_ENGINE_H
