/*
 * Emscripten builds link every module and robot into the game (there are no
 * shared libraries in the browser), so dlopen / dlsym look the modules up in
 * a table the game provides instead; the build prefixes each module's entry
 * points with its name (see SD_ADD_LIBRARY in cmake/macros.cmake).
 */
#ifndef _WASMDL_H_
#define _WASMDL_H_

#undef dlopen
#undef dlsym
#undef dlclose
#undef dlerror

extern "C" void* sd_wasm_dlopen(const char* path);
extern "C" void* sd_wasm_dlsym(void* handle, const char* name);
extern "C" int sd_wasm_dlclose(void* handle);
extern "C" const char* sd_wasm_dlerror();

#define dlopen(path, ...) sd_wasm_dlopen(path)
#define dlsym(handle, name) sd_wasm_dlsym(handle, name)
#define dlclose(handle) sd_wasm_dlclose(handle)
#define dlerror() sd_wasm_dlerror()

#endif // _WASMDL_H_
