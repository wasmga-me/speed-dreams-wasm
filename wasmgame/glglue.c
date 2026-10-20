/* The ARB spellings of two multitexture calls the game makes, which gl4es
 * only has under their core names. */
#ifdef __cplusplus
extern "C" {
#endif
void gl4es_glMultiTexCoord2fv(unsigned target, const float *v);
void gl4es_glClientActiveTexture(unsigned texture);

void gl4es_glMultiTexCoord2fvARB(unsigned target, const float *v) { gl4es_glMultiTexCoord2fv(target, v); }
void gl4es_glClientActiveTextureARB(unsigned texture) { gl4es_glClientActiveTexture(texture); }
#ifdef __cplusplus
}
#endif
