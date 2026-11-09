/* Company M31b // real GLES3 pixel-store regression matrix.
 * This is a standalone test executable, not production SceneFX source.
 * It runs with pinned Mesa surfaceless inside GitHub CI and reads back
 * actual GPU/llvmpipe texture pixels via an FBO.
 *
 * Test both CPU upload paths used by SceneFX: glTexImage2D and
 * glTexSubImage2D. Select a row-length/alignment pair reproducing actual
 * source stride, then restore previous GL alignment and skip state. Test
 * tight, full-pixel-padded, and byte-aligned rows. Blind alignment=1 is
 * NOT safe for three-byte RGB888 rows padded to four-byte boundaries.
 */
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { H = 7 };
struct format_case {
    const char *name;
    GLenum gl_format;
    GLenum gl_type;
    int bytes_per_pixel;
    int packed_type;
};
static const struct format_case formats[] = {
    { "RGB565", GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, 0 },
    { "RGBA4444", GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2, 1 },
    { "RGBA5551", GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2, 2 },
    { "RGB888", GL_RGB, GL_UNSIGNED_BYTE, 3, 3 },
    { "RGBA8888", GL_RGBA, GL_UNSIGNED_BYTE, 4, 4 },
};
static const int widths[] = {3, 4, 5, 7, 8, 13};
static int cases_passed = 0;
struct row_plan { GLint row_length; GLint alignment; };
static int try_plan(int width, int bpp, int stride, struct row_plan *plan) {
    if (!plan || width<=0 || bpp<=0 || stride<width*bpp)
        return 0;
    int rowlen=stride/bpp;
    if (rowlen<width) return 0;
    const int alignments[]={1,2,4,8};
    for (int i=0;i<4;i++) {
        int a=alignments[i];
        int bytes=rowlen*bpp;
        int rounded=((bytes+a-1)/a)*a;
        if (rounded==stride) {
            plan->row_length=rowlen;
            plan->alignment=a;
            return 1;
        }
    }
    return 0;
}

static void die(const char *what, const struct format_case *f, int width,
        int padded) {
    fprintf(stderr, "M31B_FAIL %s format=%s width=%d padded=%d "
            "gl_error=0x%x\n", what, f ? f->name : "none", width, padded,
            (unsigned int)glGetError());
    exit(2);
}
static void color_pattern(int x, int y, int phase, unsigned char *rgba) {
    rgba[0] = ((x + y + phase) & 1) ? 255 : 0;
    rgba[1] = ((x / 2 + y + phase) & 1) ? 255 : 0;
    rgba[2] = ((x + y / 2 + phase) & 1) ? 255 : 0;
    rgba[3] = 255;
}
static void store_pixel(unsigned char *dst, const struct format_case *f,
        const unsigned char *rgba) {
    const uint16_t r = rgba[0] ? 1 : 0;
    const uint16_t g = rgba[1] ? 1 : 0;
    const uint16_t b = rgba[2] ? 1 : 0;
    uint16_t p = 0;
    switch (f->packed_type) {
    case 0: p = (uint16_t)((r * 31 << 11) | (g * 63 << 5) | (b * 31)); break;
    case 1: p = (uint16_t)((r * 15 << 12) | (g * 15 << 8) |
                              (b * 15 << 4) | 15); break;
    case 2: p = (uint16_t)((r * 31 << 11) | (g * 31 << 6) |
                              (b * 31 << 1) | 1); break;
    case 3: dst[0] = rgba[0]; dst[1] = rgba[1]; dst[2] = rgba[2]; return;
    case 4: memcpy(dst, rgba, 4); return;
    default: exit(2);
    }
    memcpy(dst, &p, sizeof(p));
}
static unsigned char *make_buffer(const struct format_case *f, int width,
        int padded, int phase, int *stride_out) {
    int stride = (width + (padded==1 ? 1 : 0)) * f->bytes_per_pixel;
    if (padded==2) stride=(stride+3)&~3;
    if (stride <= 0 || padded<0 || padded>2)
        die("INVALID_ROW_STRIDE_MODE", f, width, padded);
    /* Extra initialized guards allow the negative-control misalignment to
     * read wrong pixels without leaving allocated memory. */
    size_t size = (size_t)stride * H + 128;
    unsigned char *p = malloc(size);
    if (!p) die("ALLOCATE", f, width, padded);
    memset(p, 0xa5, size);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char color[4];
            color_pattern(x, y, phase, color);
            store_pixel(p + (size_t)y * stride +
                    (size_t)x * f->bytes_per_pixel, f, color);
        }
    }
    *stride_out = stride;
    return p;
}
static int mismatches(GLuint texture, const struct format_case *f,
        int width, int padded, int phase, int update) {
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        die("FRAMEBUFFER_INCOMPLETE", f, width, padded);
    unsigned char *pixels = calloc((size_t)width * H, 4);
    if (!pixels) die("READBACK_ALLOCATE", f, width, padded);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, H, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (glGetError() != GL_NO_ERROR)
        die("GL_READBACK_FAILED", f, width, padded);
    int wrong = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < width; x++) {
            const int updated = update && x >= 1 && x < width-1 &&
                    y >= 2 && y < H-1;
            unsigned char want[4];
            color_pattern(x, y, updated ? 1 : phase, want);
            unsigned char *got = pixels + 4 * ((size_t)y*width + x);
            for (int k=0; k<4; k++) {
                if (got[k] != want[k]) {
                    wrong++;
                    break;
                }
            }
        }
    }
    free(pixels);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    return wrong;
}
static void run_case(const struct format_case *f, int width, int padded,
        int expected_initial_alignment) {
    int stride=0;
    unsigned char *data = make_buffer(f, width, padded, 0, &stride);
    GLuint tex=0;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, expected_initial_alignment);
    GLint prior = -1;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prior);
    if (prior != expected_initial_alignment)
        die("PRIOR_ALIGNMENT_INVALID", f, width, padded);

    /* M31b candidate: save / choose actual byte pitch / upload / restore. */
    struct row_plan upload={0};
    if (!try_plan(width,f->bytes_per_pixel,stride,&upload))
        die("CREATE_UNREPRESENTABLE_SOURCE_STRIDE",f,width,padded);
    glPixelStorei(GL_UNPACK_ALIGNMENT, upload.alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, upload.row_length);
    glTexImage2D(GL_TEXTURE_2D,0,(GLint)f->gl_format,width,H,0,
            f->gl_format,f->gl_type,data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, prior);
    GLint restored=-1;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &restored);
    if (glGetError()!=GL_NO_ERROR || restored!=prior)
        die("CREATE_ALIGNMENT_NOT_RESTORED",f,width,padded);

    if (mismatches(tex,f,width,padded,0,0))
        die("CREATE_TEXTURE_PIXELS_WRONG",f,width,padded);

    /* SceneFX proposed update path uses row length and skip rect coords. */
    int update_stride=0;
    unsigned char *update_data=make_buffer(f,width,padded,1,&update_stride);
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&prior);
    struct row_plan update={0};
    if (!try_plan(width,f->bytes_per_pixel,update_stride,&update))
        die("UPDATE_UNREPRESENTABLE_SOURCE_STRIDE",f,width,padded);
    glPixelStorei(GL_UNPACK_ALIGNMENT,update.alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,update.row_length);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS,1);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,2);
    glTexSubImage2D(GL_TEXTURE_2D,0,1,2,width-2,H-3,
            f->gl_format,f->gl_type,update_data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,prior);
    glGetIntegerv(GL_UNPACK_ALIGNMENT,&restored);
    if (glGetError()!=GL_NO_ERROR || restored!=prior)
        die("UPDATE_ALIGNMENT_NOT_RESTORED",f,width,padded);
    if (mismatches(tex,f,width,padded,0,1))
        die("UPDATE_TEXTURE_PIXELS_WRONG",f,width,padded);

    free(data);
    free(update_data);
    glBindTexture(GL_TEXTURE_2D,0);
    glDeleteTextures(1,&tex);
    cases_passed++;
}
static void require_negative_control(const struct format_case *f,
        int width, int expected_wrong) {
    int stride=0;
    unsigned char *data=make_buffer(f,width,0,0,&stride);
    GLuint tex=0;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT,4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,stride/f->bytes_per_pixel);
    glTexImage2D(GL_TEXTURE_2D,0,(GLint)f->gl_format,width,H,0,
            f->gl_format,f->gl_type,data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    if (glGetError()!=GL_NO_ERROR)die("NEGATIVE_CONTROL_GL_ERROR",f,width,0);
    int wrong = mismatches(tex,f,width,0,0,0);
    if ((wrong>0)!=expected_wrong)die("NEGATIVE_CONTROL_NOT_REPRODUCED",f,width,0);
    printf("M31B_GL_NEGATIVE_CONTROL format=%s width=%d pitch=%d "
           "default_align=4 wrong_pixels=%d expected_mismatch=%d PASS\n",
           f->name,width,stride,wrong,expected_wrong);
    glDeleteTextures(1,&tex);
    free(data);
}
static void refuse_blind_alignment_one_for_rgb888_padded(void) {
    const struct format_case *f=&formats[3]; /* 3-byte RGB888 */
    const int width=5;
    int stride=0;
    unsigned char *data=make_buffer(f,width,2,0,&stride);
    if (stride!=16)die("BAD_PADDED_RGB888_TEST",f,width,2);
    struct row_plan valid={0};
    if (!try_plan(width,3,stride,&valid) ||
            valid.alignment==1 || valid.row_length!=5)
        die("VALID_PADDED_RGB888_LAYOUT_NOT_DISCOVERED",f,width,2);
    GLuint tex=0;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,stride/3);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,H,0,
            GL_RGB,GL_UNSIGNED_BYTE,data);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    if (glGetError()!=GL_NO_ERROR)
        die("BLIND_ALIGNMENT_ONE_NEGATIVE_GL_ERROR",f,width,2);
    int wrong=mismatches(tex,f,width,2,0,0);
    if (wrong<=0)
        die("BLIND_ALIGNMENT_ONE_FALSE_ACCEPT",f,width,2);
    printf("M31B_BLIND_ALIGNMENT_ONE_NEGATIVE "
           "format=RGB888 width=5 source_pitch=16 "
           "naive_row_pitch=15 wrong_pixels=%d "
           "selected_unpack_alignment=%d PASS\n",
           wrong,valid.alignment);
    glDeleteTextures(1,&tex);
    free(data);
}
static void invalid_stride_contract(void) {
    struct row_plan p={0};
    if (try_plan(5,3,17,&p) || try_plan(5,3,14,&p) ||
            try_plan(5,3,-1,&p) || try_plan(0,3,16,&p) ||
            try_plan(5,0,16,&p)) {
        fprintf(stderr,"M31B_UNREPRESENTABLE_STRIDE_FALSE_ACCEPT\n");
        exit(2);
    }
}
int main(void) {
    if (sizeof(formats)/sizeof(formats[0])!=5)
        return 2;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display==EGL_NO_DISPLAY || !eglInitialize(display,NULL,NULL)) {
        fprintf(stderr,"M31B_GLES_SURFACELESS_INIT_REFUSED egl=0x%x\n",
                (unsigned int)eglGetError());
        return 2;
    }
    const EGLint attrs[]={
        EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,
        EGL_ALPHA_SIZE,8,EGL_NONE
    };
    EGLConfig config=NULL;
    EGLint count=0;
    if (!eglChooseConfig(display,attrs,&config,1,&count) || count!=1)
        { fprintf(stderr,"M31B_GLES3_CONFIG_UNAVAILABLE\n");return 2; }
    if (!eglBindAPI(EGL_OPENGL_ES_API))
        {fprintf(stderr,"M31B_EGL_ES_BIND_FAILED\n");return 2;}
    const EGLint pbuf[]={EGL_WIDTH,32,EGL_HEIGHT,32,EGL_NONE};
    EGLSurface surface=eglCreatePbufferSurface(display,config,pbuf);
    const EGLint ctxattr[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};
    EGLContext ctx=eglCreateContext(display,config,EGL_NO_CONTEXT,ctxattr);
    if (surface==EGL_NO_SURFACE || ctx==EGL_NO_CONTEXT ||
            !eglMakeCurrent(display,surface,surface,ctx))
        {fprintf(stderr,"M31B_GLES3_CONTEXT_UNAVAILABLE\n");return 2;}
    const int widths_len=(int)(sizeof(widths)/sizeof(widths[0]));
    const int formats_len=(int)(sizeof(formats)/sizeof(formats[0]));
    for (int f=0;f<formats_len;f++) {
        for (int w=0;w<widths_len;w++) {
            for (int padded=0;padded<=2;padded++) {
                run_case(&formats[f],widths[w],padded,
                         ((w+f+padded)&1)?4:8);
            }
        }
    }
    /* Deliberately demonstrate the failure mode, not just fixes. */
    require_negative_control(&formats[0],3,1);
    require_negative_control(&formats[0],4,0);
    require_negative_control(&formats[3],5,1);
    require_negative_control(&formats[4],5,0);
    refuse_blind_alignment_one_for_rgb888_padded();
    invalid_stride_contract();
    printf("SD_COMPANY_M31B_REAL_GLES_PIXELSTORE_MATRIX "
           "formats=%d widths=%d stride_variants=3 "
           "create_update_cases=%d native_GLES3=1 "
           "prior_alignment_restored=1 negative_controls=5 nonrepresentable_stride=REFUSED "
           "production_scenefx=UNMODIFIED PASS\n",
           formats_len,widths_len,cases_passed);
    eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    eglDestroyContext(display,ctx);
    eglDestroySurface(display,surface);
    eglTerminate(display);
    return cases_passed==90 ? 0 : 2;
}
