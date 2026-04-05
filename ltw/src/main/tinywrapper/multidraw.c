/**
 * Created by: artDev
 * Copyright (c) 2025 artDev, SerpentSpirale, CADIndie.
 * For use under LGPL-3.0
 */

#include <proc.h>
#include <egl.h>
#include "main.h"
#include "basevertex.h"

static inline void bind_buffer_if_needed(GLenum target, GLuint buffer) {
    int index = get_buffer_index(target);
    if(index != -1 && current_context->bound_buffers[index] == buffer) return;
    es3_functions.glBindBuffer(target, buffer);
    if(index != -1) current_context->bound_buffers[index] = buffer;
}

void glMultiDrawArrays( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount )
{
    // We'd need to merge each buffer attached to the VBO to properly achieve this. Nuh-uh. Aint no way im doin allat
    if(!current_context || primcount == 0) return;
    for (int i = 0; i < primcount; i++) {
        if (count[i] > 0)
            es3_functions.glDrawArrays(mode, first[i], count[i]);
    }
}

void glMultiDrawElements( GLenum mode, GLsizei *count, GLenum type, const void * const *indices, GLsizei primcount )
{
    if(!current_context || primcount == 0) return;
    GLint elementbuffer;
    es3_functions.glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementbuffer);
    GLsizei total = 0, offset = 0, typebytes = type_bytes(type);
    for (GLsizei i = 0; i < primcount; i++) {
        total += count[i];
    }
    if(total == 0) return;

    bind_buffer_if_needed(GL_COPY_WRITE_BUFFER, current_context->multidraw_element_buffer);
    es3_functions.glBufferData(GL_COPY_WRITE_BUFFER, total * typebytes, NULL, GL_STREAM_DRAW);
    for (GLsizei i = 0; i < primcount; i++) {
        GLsizei icount = count[i];
        if(icount == 0) continue;
        icount *= typebytes;
        if(elementbuffer != 0) {
            es3_functions.glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, (GLintptr)indices[i], offset, icount);
        } else {
            es3_functions.glBufferSubData(GL_COPY_WRITE_BUFFER, offset, icount, indices[i]);
        }
        offset += icount;
    }
    bind_buffer_if_needed(GL_ELEMENT_ARRAY_BUFFER, current_context->multidraw_element_buffer);
    es3_functions.glDrawElements(mode, total, type, 0);
    if(elementbuffer != 0) {
        bind_buffer_if_needed(GL_ELEMENT_ARRAY_BUFFER, (GLuint)elementbuffer);
    }
}
