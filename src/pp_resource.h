/*
 * Copyright © 2013-2014  Rinat Ibragimov
 *
 * This file is part of FreshPlayerPlugin.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FPP_PP_RESOURCE_H
#define FPP_PP_RESOURCE_H

#include <ppapi/c/pp_completion_callback.h>
#include <ppapi/c/pp_rect.h>
#include <ppapi/c/pp_resource.h>
#include <ppapi/c/private/pp_private_font_charset.h>
#include <ppapi/c/ppb_image_data.h>
#include <ppapi/c/ppb_input_event.h>
#include <ppapi/c/ppb_audio_config.h>
#include <ppapi/c/ppb_audio.h>

#include <stdlib.h>
#include <X11/Xlib.h>
#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <glib.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <pango/pangocairo.h>
#include <cairo.h>
#include <asoundlib.h>
#include <gtk/gtk.h>


#define free_and_nullify(obj, field)    \
    do {                                \
        free(obj->field);               \
        obj->field = NULL;              \
    } while (0)

#define nullsafe_strdup(s)       ((s) ? strdup(s) : NULL)

#define PP_MakeCCB(func, user_data) PP_MakeCompletionCallback(func, user_data)


enum pp_resource_type_e {
    PP_RESOURCE_UNKNOWN = 0,
    PP_RESOURCE_URL_LOADER,
    PP_RESOURCE_URL_REQUEST_INFO,
    PP_RESOURCE_URL_RESPONSE_INFO,
    PP_RESOURCE_VIEW,
    PP_RESOURCE_GRAPHICS3D,
    PP_RESOURCE_IMAGE_DATA,
    PP_RESOURCE_GRAPHICS2D,
    PP_RESOURCE_NETWORK_MONITOR,
    PP_RESOURCE_BROWSER_FONT,
    PP_RESOURCE_AUDIO_CONFIG,
    PP_RESOURCE_AUDIO,
    PP_RESOURCE_INPUT_EVENT,
    PP_RESOURCE_FLASH_FONT_FILE,
    PP_RESOURCE_PRINTING,
    PP_RESOURCE_VIDEO_CAPTURE,
    PP_RESOURCE_AUDIO_INPUT,
    PP_RESOURCE_FLASH_MENU,
    PP_RESOURCE_FLASH_MESSAGE_LOOP,
    PP_RESOURCE_TCP_SOCKET,
    PP_RESOURCE_FILE_REF,
    PP_RESOURCE_FILE_IO,
    PP_RESOURCE_MESSAGE_LOOP,
    PP_RESOURCE_FLASH_DRM,

    PP_RESOURCE_TYPES_COUNT,        // must be the last item in the list
};

enum pp_request_method_e {
    PP_METHOD_UNKNOWN,
    PP_METHOD_GET,
    PP_METHOD_POST,
};

enum file_ref_type_e {
    PP_FILE_REF_TYPE_NAME,
    PP_FILE_REF_TYPE_FD,
};

struct np_proxy_object_s {
    NPObject npobj;
    struct PP_Var ppobj;
};

struct pp_instance_s {
    const struct PPP_Instance_1_1  *ppp_instance_1_1;
    const struct PPP_InputEvent_0_1 *ppp_input_event;
    const struct PPP_Instance_Private_0_1 *ppp_instance_private;
    NPObject                       *scriptable_obj;
    NPObject                       *np_window_obj;
    NPObject                       *np_plugin_element_obj;
    uint32_t                        event_mask;
    uint32_t                        filtered_event_mask;
    Window                          wnd;
    PP_Instance                     id;
    NPP                             npp;
    uint32_t                        is_fullframe;
    uint32_t                        is_fullscreen;
    uint32_t                        is_transparent;
    uint32_t                        incognito_mode;
    volatile gint                   instance_loaded;
    uint32_t                        ignore_focus_events_cnt; ///< number of focus events to ignore

    // full screen
    pthread_t                       fs_thread;
    Window                          fs_wnd;

    // geometry
    uint32_t                        width;
    uint32_t                        height;

    int                             argc;
    char                          **argn;
    char                          **argv;

    struct PP_Var                   instance_url;
    struct PP_Var                   document_url;
    pthread_t                       main_thread;
    pthread_barrier_t               main_thread_barrier;

    // graphics2d and graphics3d
    PP_Resource                     graphics;
    struct PP_CompletionCallback    graphics_ccb;
    pthread_barrier_t               graphics_barrier;
    uint32_t                        graphics_in_progress;
};


#define COMMON_STRUCTURE_FIELDS                 \
    int                     resource_type;      \
    int                     ref_cnt;            \
    struct pp_instance_s   *instance;           \
    PP_Resource             self_id;            \
    pthread_mutex_t         lock;


struct pp_resource_generic_s {
    COMMON_STRUCTURE_FIELDS
};

struct pp_url_loader_s {
    COMMON_STRUCTURE_FIELDS
    char                   *status_line;    ///< HTTP/1.1 200 OK
    char                   *headers;        ///< response headers
    int                     http_code;      ///< HTTP response code
    int                     fd;             ///< file used to store response
    size_t                  read_pos;       ///< reading position
    enum pp_request_method_e method;        ///< GET/POST
    char                   *url;            ///< request URL
    char                   *redirect_url;   ///< value of the Location header if this is
                                            ///< a redirection response
    int                     finished_loading;   ///< if whole stream loaded already
    int32_t                 response_size;  ///< Content-Length value of -1 if absent
    int                     stream_to_file; ///< whenever streaming to file is allowed
    struct PP_CompletionCallback    stream_to_file_ccb; ///< callback to call on end of streaming

    char                   *request_headers;
    PP_Bool                 follow_redirects;   ///< handle redirections internally
    PP_Bool                 record_download_progress;
    PP_Bool                 record_upload_progress;
    char                   *custom_referrer_url;
    PP_Bool                 allow_cross_origin_requests;
    PP_Bool                 allow_credentials;
    char                   *custom_content_transfer_encoding;
    char                   *custom_user_agent;
    char                   *target;
    char                   *post_data;
    size_t                  post_len;
    GList                  *read_tasks;     ///< list of url_loader_read_task_s
    NPStream               *np_stream;      ///< associated NPStream
    struct PP_CompletionCallback    ccb;    ///< callback to call on headers arrival
};

struct url_loader_read_task_s {
    void                           *buffer;
    int32_t                         bytes_to_read;
    struct PP_CompletionCallback    ccb;
};

struct pp_url_request_info_s {
    COMMON_STRUCTURE_FIELDS
    enum pp_request_method_e    method;
    char                       *url;
    char                       *headers;
    unsigned int                is_immediate_javascript; // data in url is javascript line
    PP_Bool                     stream_to_file;
    PP_Bool                     follow_redirects;
    PP_Bool                     record_download_progress;
    PP_Bool                     record_upload_progress;
    char                       *custom_referrer_url;
    PP_Bool                     allow_cross_origin_requests;
    PP_Bool                     allow_credentials;
    char                       *custom_content_transfer_encoding;
    int32_t                     prefetch_buffer_upper_threshold;
    int32_t                     prefetch_buffer_lower_threshold;
    char                       *custom_user_agent;
    size_t                      post_len;
    void                       *post_data;
};

struct pp_url_response_info_s {
    COMMON_STRUCTURE_FIELDS
    PP_Resource                 url_loader_id;
    struct pp_url_loader_s     *url_loader;
};

struct pp_view_s {
    COMMON_STRUCTURE_FIELDS
    struct PP_Rect      rect;
};

struct pp_graphics3d_s {
    COMMON_STRUCTURE_FIELDS
    EGLContext      glc;
    EGLConfig       egl_config;
    EGLContext      glc_t;          ///< presentation context for transparent instances
    EGLConfig       egl_config_t;   ///< EGLConfig for glc_t
    Pixmap          pixmap;
    EGLSurface      egl_surf;
    int32_t         width;
    int32_t         height;
    GHashTable     *sub_maps;
    GLuint          tex_front;      ///< transparency helper texture, plugin data
    GLuint          tex_back;       ///< transparency helper texture, previous drawable contents

    struct {
        GLuint      id;
        GLuint      attrib_pos;
        GLint       uniform_tex_back;
        GLint       uniform_tex_front;
    } prog;                                     ///< alpha blending GL program
};

struct pp_image_data_s {
    COMMON_STRUCTURE_FIELDS
    int32_t             width;
    int32_t             height;
    int32_t             stride;
    char               *data;
    PP_ImageDataFormat  format;
    cairo_surface_t    *cairo_surf;
};

struct pp_graphics2d_s {
    COMMON_STRUCTURE_FIELDS
    int                 is_always_opaque;
    int32_t             width;
    int32_t             height;
    int32_t             stride;
    double              scale;
    int32_t             scaled_width;
    int32_t             scaled_height;
    int32_t             scaled_stride;
    char               *data;
    char               *second_buffer;
    cairo_surface_t    *cairo_surf;
    GList              *task_list;
};

struct pp_network_monitor_s {
    COMMON_STRUCTURE_FIELDS
};

struct pp_browser_font_s {
    COMMON_STRUCTURE_FIELDS
    PangoFont              *font;
    PangoFontDescription   *font_desc;
    int32_t                 letter_spacing;
    int32_t                 word_spacing;
    int32_t                 family;
};

struct pp_audio_config_s {
    COMMON_STRUCTURE_FIELDS
    PP_AudioSampleRate      sample_rate;
    uint32_t                sample_frame_count;
};

struct pp_audio_s {
    COMMON_STRUCTURE_FIELDS
    uint32_t                sample_rate;
    uint32_t                sample_frame_count;
    uint32_t                period_size;
    snd_pcm_t              *ph;
    void                   *audio_buffer;
    pthread_t               thread;
    uint32_t                playing;
    uint32_t                shutdown;
    PPB_Audio_Callback_1_0  callback_1_0;
    PPB_Audio_Callback      callback_1_1;
    void                   *user_data;
};

struct pp_input_event_s {
    COMMON_STRUCTURE_FIELDS
    uint32_t                    event_class;
    PP_InputEvent_Type          type;
    PP_TimeTicks                time_stamp;
    uint32_t                    modifiers;
    PP_InputEvent_MouseButton   mouse_button;
    struct PP_Point             mouse_position;
    int32_t                     click_count;
    struct PP_Point             mouse_movement;
    struct PP_FloatPoint        wheel_delta;
    struct PP_FloatPoint        wheel_ticks;
    PP_Bool                     scroll_by_page;
    uint32_t                    key_code;
    struct PP_Var               character_text;
    struct PP_Var               code;
};

struct pp_flash_font_file_s {
    COMMON_STRUCTURE_FIELDS
    PangoFont              *font;
    FT_Face                 ft_face;
    PP_PrivateFontCharset   charset;
};

struct pp_printing_s {
    COMMON_STRUCTURE_FIELDS
};

struct pp_video_capture_s {
    COMMON_STRUCTURE_FIELDS
};

struct pp_audio_input_s {
    COMMON_STRUCTURE_FIELDS
};

struct pp_flash_menu_s {
    COMMON_STRUCTURE_FIELDS
    GtkWidget              *menu;
};

struct pp_flash_message_loop_s {
    COMMON_STRUCTURE_FIELDS
    int             running;
    PP_Resource     message_loop;
    int             depth;
};

struct pp_tcp_socket_s {
    COMMON_STRUCTURE_FIELDS
    int             sock;
    unsigned int    is_connected;
    unsigned int    destroyed;
    unsigned int    seen_eof;
};

struct pp_file_ref_s {
    COMMON_STRUCTURE_FIELDS
    int                     fd;
    enum file_ref_type_e    type;
};

struct pp_file_io_s {
    COMMON_STRUCTURE_FIELDS
    int             fd;
};

struct pp_message_loop_s {
    COMMON_STRUCTURE_FIELDS
    GAsyncQueue            *async_q;
    GQueue                 *int_q;
    int                     running;
    int                     teardown;
    int                     depth;
};

struct pp_flash_drm_s {
    COMMON_STRUCTURE_FIELDS
};

union pp_largest_u {
    struct pp_instance_s            s02;
    struct pp_resource_generic_s    s03;
    struct pp_url_loader_s          s04;
    struct pp_url_request_info_s    s05;
    struct pp_url_response_info_s   s06;
    struct pp_view_s                s07;
    struct pp_graphics3d_s          s08;
    struct pp_image_data_s          s09;
    struct pp_graphics2d_s          s10;
    struct pp_network_monitor_s     s11;
    struct pp_browser_font_s        s12;
    struct pp_audio_config_s        s13;
    struct pp_audio_s               s14;
    struct pp_input_event_s         s15;
    struct pp_flash_font_file_s     s16;
    struct pp_printing_s            s17;
    struct pp_video_capture_s       s18;
    struct pp_audio_input_s         s19;
    struct pp_flash_menu_s          s20;
    struct pp_flash_message_loop_s  s21;
    struct pp_tcp_socket_s          s22;
    struct pp_file_ref_s            s23;
    struct pp_file_io_s             s24;
    struct pp_message_loop_s        s25;
    struct pp_flash_drm_s           s26;
};

PP_Resource             pp_resource_allocate(enum pp_resource_type_e type,
                                             struct pp_instance_s *instance);
void                    pp_resource_expunge(PP_Resource resource);
void                   *pp_resource_acquire(PP_Resource resource, enum pp_resource_type_e type);
void                    pp_resource_release(PP_Resource resource);
enum pp_resource_type_e pp_resource_get_type(PP_Resource resource);
void                    pp_resource_ref(PP_Resource resource);
void                    pp_resource_unref(PP_Resource resource);


#endif // FPP_PP_RESOURCE_H
