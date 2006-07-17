/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "_highgui.h"

#ifndef WIN32

#ifdef HAVE_GTK

#include "gtk/gtk.h"
#include "gdk/gdkkeysyms.h"
#include <stdio.h>

/*#if _MSC_VER >= 1200
#pragma warning( disable: 4505 )
#pragma comment(lib,"gtk-win32-2.0.lib")
#pragma comment(lib,"glib-2.0.lib")
#pragma comment(lib,"gobject-2.0.lib")
#pragma comment(lib,"gdk-win32-2.0.lib")
#pragma comment(lib,"gdk_pixbuf-2.0.lib")
#endif*/

struct CvWindow;

typedef struct CvTrackbar
{
    int signature;
    GtkWidget* widget;
    char* name;
    CvTrackbar* next;
    CvWindow* parent;
    int* data;
    int pos;
    int maxval;
    CvTrackbarCallback notify;
}
CvTrackbar;


typedef struct CvWindow
{
    int signature;
    GtkWidget* area;
    GtkWidget* frame;
    GtkWidget* paned;
    char* name;
    CvWindow* prev;
    CvWindow* next;
    
    CvMat* image;
    CvMat* dst_image;
    int converted;
    int last_key;
    int flags;

    CvMouseCallback on_mouse;
    void* on_mouse_param;

    struct
    {
        int pos;
        int rows;
        CvTrackbar* first;
    }
    toolbar;
}
CvWindow;


//#ifndef NDEBUG
#define Assert(exp)                                             \
if( !(exp) )                                                    \
{                                                               \
    printf("Assertion: %s  %s: %d\n", #exp, __FILE__, __LINE__);\
    assert(exp);                                                \
}

static void icvResizeWindow( CvWindow * window, int width, int height );
static void icvPutImage( CvWindow* window );
static gboolean icvOnClose( GtkWidget* widget, GdkEvent* event, gpointer user_data );
static gboolean icvOnExpose( GtkWidget* widget,
                GdkEventExpose *event, gpointer user_data );
static gboolean icvOnKeyPress( GtkWidget* widget, GdkEventKey* event, gpointer user_data );
static void icvOnTrackbar( GtkWidget* widget, gpointer user_data );
static gboolean icvOnMouse( GtkWidget *widget, GdkEvent *event, gpointer user_data );

#ifdef HAVE_GTHREAD
int thread_started=0;
static gpointer icvWindowThreadLoop();
GMutex*				   last_key_mutex;
GCond*				   cond_have_key;
GMutex*				   window_mutex;
GThread*			   window_thread;
GtkWidget*             cvTopLevelWidget = 0;
#endif

static int             last_key = -1;
static CvWindow* hg_windows = 0;

CV_IMPL int cvInitSystem( int argc, char** argv )
{
    static int wasInitialized = 0;    

    // check initialization status
    if( !wasInitialized )
    {
        hg_windows = 0;

        gtk_init( &argc, &argv );
        wasInitialized = 1;
    }

    return 0;
}

CV_IMPL int cvStartWindowThread(){
#ifdef HAVE_GTHREAD
	cvInitSystem(0,NULL);
    if (!thread_started) {
	if (!g_thread_supported ()) {
	    /* the GThread system wasn't inited, so init it */
	    g_thread_init(NULL);
	}
	
	// this mutex protects the window resources 
	window_mutex = g_mutex_new();
	
	// protects the 'last key pressed' variable
	last_key_mutex = g_mutex_new();
	
	// conditional that indicates a key has been pressed
	cond_have_key = g_cond_new();

	// this is the window update thread
	window_thread = g_thread_create((GThreadFunc) icvWindowThreadLoop,
					NULL, TRUE, NULL);
    }
    thread_started = window_thread!=NULL;
    return thread_started;
#else
    return 0;
#endif
}

#ifdef HAVE_GTHREAD
gpointer icvWindowThreadLoop(){
	while(1){
		g_mutex_lock(window_mutex);
		gtk_main_iteration_do(FALSE);
		g_mutex_unlock(window_mutex);

		// little sleep 
		g_usleep(500);

		g_thread_yield();
	}
	return NULL;
}
#endif

static CvWindow* icvFindWindowByName( const char* name )
{
    CvWindow* window = hg_windows;
    while( window != 0 && strcmp(name, window->name) != 0 )
        window = window->next;

    return window;
}

static CvWindow* icvWindowByWidget( GtkWidget* widget )
{
    CvWindow* window = hg_windows;

    while( window != 0 && window->area != widget &&
           window->frame != widget && window->paned != widget )
        window = window->next;

    return window;
}


static void
icvUpdateWindowSize( const CvWindow* window )
{
    int width = 0, height = 0;
    CvTrackbar* t;

    if( !window->frame )
        return;
    gtk_window_get_size( GTK_WINDOW(window->frame), &width, &height );
    if( window->image )
    {
        width = MAX(window->image->width,width);
        height = window->image->height;
    }

    //gtk_widget_translate_coordinates( window->area, window->paned, 0, 0, &dw, &dh );
    //height += dh;

    for( t = window->toolbar.first; t != 0; t = t->next )
        height += 50;
    
    //if( window->flags & CV_WINDOW_AUTOSIZE )
    //    gtk_window_set_resizable( GTK_WINDOW(window->frame), TRUE );
    gtk_window_resize( GTK_WINDOW(window->frame), width, height );
    //if( window->flags & CV_WINDOW_AUTOSIZE )
    //    gtk_window_set_resizable( GTK_WINDOW(window->frame), FALSE );
}



CV_IMPL int cvNamedWindow( const char* name, int flags )
{
    int result = 0;
    CV_FUNCNAME( "cvNamedWindow" );

    __BEGIN__;

    CvWindow* window;
    int len;

    cvInitSystem(1,(char**)&name);
    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name string" );

    // Check the name in the storage
    if( icvFindWindowByName( name ) != 0 )
    {
        result = 1;
        EXIT;
    }

    len = strlen(name);
    CV_CALL( window = (CvWindow*)cvAlloc(sizeof(CvWindow) + len + 1));
    memset( window, 0, sizeof(*window));
    window->name = (char*)(window + 1);
    memcpy( window->name, name, len + 1 );
    window->flags = flags;
    window->signature = CV_WINDOW_MAGIC_VAL;
    window->image = 0;
    window->last_key = 0;
    window->on_mouse = 0;
    window->on_mouse_param = 0;
    memset( &window->toolbar, 0, sizeof(window->toolbar));
    window->next = hg_windows;
    window->prev = 0;

#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_lock(window_mutex);
#endif

	
    window->frame = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    window->paned = gtk_vbox_new( FALSE, 0 );
    //window->trackbars = 0;
    window->area = gtk_drawing_area_new();
    gtk_box_pack_end( GTK_BOX(window->paned), window->area, TRUE, TRUE, 0 );
    gtk_widget_show( window->area );
    gtk_container_add( GTK_CONTAINER(window->frame), window->paned );
    gtk_widget_show( window->paned );
    gtk_widget_set_size_request( window->frame, 100, 100 );
    //gtk_window_set_transient_for( window->area, GTK_WINDOW(window->frame) );
    gtk_signal_connect( GTK_OBJECT(window->area), "expose-event",
                        GTK_SIGNAL_FUNC(icvOnExpose), window );
    gtk_signal_connect( GTK_OBJECT(window->frame), "key-press-event",
                        GTK_SIGNAL_FUNC(icvOnKeyPress), window );
    gtk_signal_connect( GTK_OBJECT(window->area), "button-press-event",
                        GTK_SIGNAL_FUNC(icvOnMouse), window );
    gtk_signal_connect( GTK_OBJECT(window->area), "button-release-event",
                        GTK_SIGNAL_FUNC(icvOnMouse), window );
    gtk_signal_connect( GTK_OBJECT(window->area), "motion-notify-event",
                        GTK_SIGNAL_FUNC(icvOnMouse), window );
    gtk_signal_connect( GTK_OBJECT(window->frame), "delete-event",
                        GTK_SIGNAL_FUNC(icvOnClose), window );
    gtk_widget_set_events( window->area, GDK_EXPOSURE_MASK | GDK_BUTTON_RELEASE_MASK |
                                         GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK );

    gtk_widget_show( window->frame );
    gtk_window_set_title( GTK_WINDOW(window->frame), name );
    //gtk_window_set_resizable( GTK_WINDOW(window->frame), (flags & CV_WINDOW_AUTOSIZE) == 0 );

    if( hg_windows )
        hg_windows->prev = window;
    hg_windows = window;

#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif

    result = 1;
    __END__;

    return result;
}


static void icvDeleteWindow( CvWindow* window )
{
    CvTrackbar* trackbar;
    
    if( window->prev )
        window->prev->next = window->next;
    else
        hg_windows = window->next;

    if( window->next )
        window->next->prev = window->prev;

    window->prev = window->next = 0;
	
    cvReleaseMat( &window->image );
    cvReleaseMat( &window->dst_image );
    
	gtk_widget_destroy( window->frame );
	
    for( trackbar = window->toolbar.first; trackbar != 0; )
    {
        CvTrackbar* next = trackbar->next;
        cvFree( &trackbar );
        trackbar = next;
    }

    cvFree( &window );
#ifdef HAVE_GTHREAD
	// if last window, send key press signal 
	// to jump out of any waiting cvWaitKey's
	if(hg_windows==0 && thread_started){
		g_cond_broadcast(cond_have_key);
	}
#endif
}


CV_IMPL void cvDestroyWindow( const char* name )
{
    CV_FUNCNAME( "cvDestroyWindow" );
    
    __BEGIN__;

    CvWindow* window;

    if(!name)
        CV_ERROR( CV_StsNullPtr, "NULL name string" );

    window = icvFindWindowByName( name );
    if( !window )
        EXIT;

#ifdef HAVE_GTHREAD
	// note that it is possible for the update thread to run this function
	// if there is a call to cvShowImage in a mouse callback
	// (this would produce a deadlock on window_mutex)
    if(thread_started  && g_thread_self()!=window_thread) g_mutex_lock(window_mutex);
#endif
	icvDeleteWindow( window );
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif
    __END__;
}


CV_IMPL void
cvDestroyAllWindows( void )
{
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_lock(window_mutex);
#endif
    while( hg_windows )
    {
        CvWindow* window = hg_windows;
        icvDeleteWindow( window );
    }
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif
}


CV_IMPL void
cvShowImage( const char* name, const CvArr* arr )
{
    CV_FUNCNAME( "cvShowImage" );

    __BEGIN__;
   	
    CvWindow* window;
    int origin = 0;
    CvMat stub, *image;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

#ifdef HAVE_GTHREAD
    if( thread_started && g_thread_self()!=window_thread ) g_mutex_lock(window_mutex);
#endif

    window = icvFindWindowByName(name);
    if( !window || !arr )
        EXIT; // keep silence here.

	
    if( CV_IS_IMAGE_HDR( arr ))
        origin = ((IplImage*)arr)->origin;
    
    CV_CALL( image = cvGetMat( arr, &stub ));
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif

    if( !window->image )
        icvResizeWindow( window, image->cols, image->rows );

    if( window->image &&
        !CV_ARE_SIZES_EQ(window->image, image) )
        cvReleaseMat( &window->image );

    if( !window->image )
        window->image = cvCreateMat( image->rows, image->cols, CV_8UC3 );

    cvConvertImage( image, window->image,
        (origin != 0 ? CV_CVTIMG_FLIP : 0) + CV_CVTIMG_SWAP_RB );
	icvPutImage( window );
    icvUpdateWindowSize( window );
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif

    __END__;
}

void icvResizeWindow( CvWindow * window, int width, int height ){
    gtk_window_resize( GTK_WINDOW(window->frame), width, height );
}

CV_IMPL void cvResizeWindow(const char* name, int width, int height )
{
    CV_FUNCNAME( "cvResizeWindow" );

    __BEGIN__;
    
    CvWindow* window;
    //CvTrackbar* trackbar;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

    window = icvFindWindowByName(name);
    if(!window)
        EXIT;

    // TODO: take into account frame borders and trackbars
#ifdef HAVE_GTHREAD
    if(thread_started && g_thread_self()!=window_thread) g_mutex_lock(window_mutex);
#endif
	icvResizeWindow( window, width, height );
#ifdef HAVE_GTHREAD
    if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif
    //gtk_widget_set_size_request( window->area, width, height );

    __END__;
}


CV_IMPL void cvMoveWindow( const char* name, int x, int y )
{
    CV_FUNCNAME( "cvMoveWindow" );

    __BEGIN__;

    CvWindow* window;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

    window = icvFindWindowByName(name);
    if(!window)
        EXIT;
#ifdef HAVE_GTHREAD
    if(thread_started && g_thread_self()!=window_thread) g_mutex_lock(window_mutex);
#endif
    gtk_window_move( GTK_WINDOW(window->frame), x, y );
#ifdef HAVE_GTHREAD
    if(thread_started && g_thread_self()!=window_thread) g_mutex_unlock(window_mutex);
#endif

    __END__;
}


static CvTrackbar*
icvFindTrackbarByName( const CvWindow* window, const char* name )
{
    CvTrackbar* trackbar = window->toolbar.first;

    for( ; trackbar != 0 && strcmp( trackbar->name, name ) != 0; trackbar = trackbar->next )
        ;

    return trackbar;
}


CV_IMPL int
cvCreateTrackbar( const char* trackbar_name, const char* window_name,
                  int* val, int count, CvTrackbarCallback on_notify )
{
    int result = 0;

    CV_FUNCNAME( "cvCreateTrackbar" );

    __BEGIN__;
    
    /*char slider_name[32];*/
    CvWindow* window = 0;
    CvTrackbar* trackbar = 0;

    if( !window_name || !trackbar_name )
        CV_ERROR( CV_StsNullPtr, "NULL window or trackbar name" );

    if( count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Bad trackbar maximal value" );

    window = icvFindWindowByName(window_name);
    if( !window )
        EXIT;

    trackbar = icvFindTrackbarByName(window,trackbar_name);
    if( !trackbar )
    {
        int len = strlen(trackbar_name);
        trackbar = (CvTrackbar*)cvAlloc(sizeof(CvTrackbar) + len + 1);
        memset( trackbar, 0, sizeof(*trackbar));
        trackbar->signature = CV_TRACKBAR_MAGIC_VAL;
        trackbar->name = (char*)(trackbar+1);
        memcpy( trackbar->name, trackbar_name, len + 1 );
        trackbar->parent = window;
        trackbar->next = window->toolbar.first;
        window->toolbar.first = trackbar;
        
        GtkWidget* hscale_box = gtk_hbox_new( FALSE, 10 );
        GtkWidget* hscale_label = gtk_label_new( trackbar_name );
        GtkWidget* hscale = gtk_hscale_new_with_range( 0, count, 1 );
        gtk_range_set_update_policy( GTK_RANGE(hscale), GTK_UPDATE_CONTINUOUS );
        gtk_scale_set_digits( GTK_SCALE(hscale), 0 );
        //gtk_scale_set_value_pos( hscale, GTK_POS_TOP );
        gtk_scale_set_draw_value( GTK_SCALE(hscale), TRUE );

        trackbar->widget = hscale;
        gtk_box_pack_start( GTK_BOX(hscale_box), hscale_label, FALSE, FALSE, 5 );
        gtk_widget_show( hscale_label );
        gtk_box_pack_start( GTK_BOX(hscale_box), hscale, TRUE, TRUE, 5 );
        gtk_widget_show( hscale );
        gtk_box_pack_start( GTK_BOX(window->paned), hscale_box, FALSE, FALSE, 5 );
        gtk_widget_show( hscale_box );

        icvUpdateWindowSize( window );
    }
        
    if( val )
    {
        int value = *val;
        if( value < 0 )
            value = 0;
        if( value > count )
            value = count;
        gtk_range_set_value( GTK_RANGE(trackbar->widget), value );
        trackbar->pos = value;
        trackbar->data = val;
    }
        
    trackbar->maxval = count;
    trackbar->notify = on_notify;
    gtk_signal_connect( GTK_OBJECT(trackbar->widget), "value-changed",
                        GTK_SIGNAL_FUNC(icvOnTrackbar), trackbar );

    if( (window->flags & CV_WINDOW_AUTOSIZE) && (window->image) )
        gtk_widget_set_size_request( window->frame, window->image->width, window->image->height );

    result = 1;

    __END__;

    return result;
}


CV_IMPL void
cvSetMouseCallback( const char* window_name, CvMouseCallback on_mouse, void* param )
{
    CV_FUNCNAME( "cvSetMouseCallback" );

    __BEGIN__;
    
    CvWindow* window = 0;

    if( !window_name )
        CV_ERROR( CV_StsNullPtr, "NULL window name" );

    window = icvFindWindowByName(window_name);
    if( !window )
        EXIT;

    window->on_mouse = on_mouse;
    window->on_mouse_param = param;

    __END__;
}


CV_IMPL int cvGetTrackbarPos( const char* trackbar_name, const char* window_name )
{
    int pos = -1;
    
    CV_FUNCNAME( "cvGetTrackbarPos" );

    __BEGIN__;

    CvWindow* window;
    CvTrackbar* trackbar = 0;

    if( trackbar_name == 0 || window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL trackbar or window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        trackbar = icvFindTrackbarByName( window, trackbar_name );

    if( trackbar )
        pos = trackbar->pos;

    __END__;

    return pos;
}


CV_IMPL void cvSetTrackbarPos( const char* trackbar_name, const char* window_name, int pos )
{
    CV_FUNCNAME( "cvSetTrackbarPos" );

    __BEGIN__;

    CvWindow* window;
    CvTrackbar* trackbar = 0;

    if( trackbar_name == 0 || window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL trackbar or window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        trackbar = icvFindTrackbarByName( window, trackbar_name );

    if( trackbar )
    {
        if( pos < 0 )
            pos = 0;

        if( pos > trackbar->maxval )
            pos = trackbar->maxval;
    }
#ifdef HAVE_GTHREAD
	if(thread_started) g_mutex_lock(window_mutex);
#endif
	
    gtk_range_set_value( GTK_RANGE(trackbar->widget), pos );

#ifdef HAVE_GTHREAD
	if(thread_started) g_mutex_unlock(window_mutex);
#endif

    __END__;
}


CV_IMPL void* cvGetWindowHandle( const char* window_name )
{
    void* widget = 0;
    
    CV_FUNCNAME( "cvGetWindowHandle" );

    __BEGIN__;

    CvWindow* window;

    if( window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        widget = (void*)window->area;

    __END__;

    return widget;
}
    

CV_IMPL const char* cvGetWindowName( void* window_handle )
{
    const char* window_name = "";
    
    CV_FUNCNAME( "cvGetWindowName" );

    __BEGIN__;

    CvWindow* window;

    if( window_handle == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL window" );

    window = icvWindowByWidget( (GtkWidget*)window_handle );
    if( window )
        window_name = window->name;

    __END__;

    return window_name;
}


/* draw image to frame */
static void icvPutImage( CvWindow* window )
{
    Assert( window != 0 );
    if( window->image == 0 )
        return;
    gdk_draw_rgb_image( window->area->window, window->area->style->fg_gc[GTK_STATE_NORMAL],
                        0, 0, window->image->cols, window->image->rows,
                        GDK_RGB_DITHER_MAX, window->image->data.ptr, window->image->step );
}                                                   
                                                    

static gboolean icvOnExpose( GtkWidget *widget,
                GdkEventExpose* /*event*/, gpointer user_data )
{
    CvWindow* window = (CvWindow*)user_data;
    if( window->signature == CV_WINDOW_MAGIC_VAL &&
        window->area == widget && window->image != 0 )
    {
	
        icvPutImage( window );
		
        return TRUE;
    }

    return FALSE;
}


static gboolean icvOnKeyPress( GtkWidget * /*widget*/,
                GdkEventKey* event, gpointer /*user_data*/ )
{
    int code = 0;
    
    switch( event->keyval )
    {
    case GDK_Escape:
        code = 27;
        break;
    case GDK_Return:
    case GDK_Linefeed:
        code = '\n';
        break;
    case GDK_Tab:
        code = '\t';
	break;
    default:
        code = event->keyval;
    }

    code |= event->state << 16;

#ifdef HAVE_GTHREAD
	if(thread_started) g_mutex_lock(last_key_mutex);
#endif
	
	last_key = code;
	
#ifdef HAVE_GTHREAD
	if(thread_started){
		// signal any waiting threads
		g_cond_broadcast(cond_have_key);
		g_mutex_unlock(last_key_mutex);
	}
#endif

    return FALSE;
}


static void icvOnTrackbar( GtkWidget* widget, gpointer user_data )
{
    int pos = cvRound( gtk_range_get_value(GTK_RANGE(widget)));
    CvTrackbar* trackbar = (CvTrackbar*)user_data;

    if( trackbar && trackbar->signature == CV_TRACKBAR_MAGIC_VAL &&
        trackbar->widget == widget )
    {
        trackbar->pos = pos;
        if( trackbar->data )
            *trackbar->data = pos;
        if( trackbar->notify )
            trackbar->notify(pos);
    }
}

static gboolean icvOnClose( GtkWidget* widget, GdkEvent* /*event*/, gpointer user_data )
{
    CvWindow* window = (CvWindow*)user_data;
    if( window->signature == CV_WINDOW_MAGIC_VAL &&
        window->frame == widget )
	{
        icvDeleteWindow(window);
	}
    return TRUE;
}


static gboolean icvOnMouse( GtkWidget *widget, GdkEvent *event, gpointer user_data )
{
    CvWindow* window = (CvWindow*)user_data;
    CvPoint pt = {-1,-1};
    int cv_event = -1, state = 0;

    if( window->signature != CV_WINDOW_MAGIC_VAL ||
        window->area != widget || !window->image || !window->on_mouse )
        return FALSE;

    if( event->type == GDK_MOTION_NOTIFY )
    {
        GdkEventMotion* event_motion = (GdkEventMotion*)event;
        
        cv_event = CV_EVENT_MOUSEMOVE;
        pt.x = cvRound(event_motion->x);
        pt.y = cvRound(event_motion->y);
        state = event_motion->state;
    }
    else if( event->type == GDK_BUTTON_PRESS ||
             event->type == GDK_BUTTON_RELEASE ||
             event->type == GDK_2BUTTON_PRESS )
    {
        GdkEventButton* event_button = (GdkEventButton*)event;
        
        pt.x = cvRound(event_button->x);
        pt.y = cvRound(event_button->y);

        if( event_button->type == GDK_BUTTON_PRESS )
        {
            cv_event = event_button->button == 1 ? CV_EVENT_LBUTTONDOWN :
                       event_button->button == 2 ? CV_EVENT_MBUTTONDOWN :
                       event_button->button == 3 ? CV_EVENT_RBUTTONDOWN : 0;
        }
        else if( event_button->type == GDK_BUTTON_RELEASE )
        {
            cv_event = event_button->button == 1 ? CV_EVENT_LBUTTONUP :
                       event_button->button == 2 ? CV_EVENT_MBUTTONUP :
                       event_button->button == 3 ? CV_EVENT_RBUTTONUP : 0;
        }
        else if( event_button->type == GDK_2BUTTON_PRESS )
        {
            cv_event = event_button->button == 1 ? CV_EVENT_LBUTTONDBLCLK :
                       event_button->button == 2 ? CV_EVENT_MBUTTONDBLCLK :
                       event_button->button == 3 ? CV_EVENT_RBUTTONDBLCLK : 0;
        }
        state = event_button->state;
    }

    if( cv_event >= 0 &&
        (unsigned)pt.x < (unsigned)(window->image->width) &&
        (unsigned)pt.y < (unsigned)(window->image->height) )
    {
        int flags = (state & GDK_SHIFT_MASK ? CV_EVENT_FLAG_SHIFTKEY : 0) |
                    (state & GDK_CONTROL_MASK ? CV_EVENT_FLAG_CTRLKEY : 0) |
                    (state & (GDK_MOD1_MASK|GDK_MOD2_MASK) ? CV_EVENT_FLAG_ALTKEY : 0) |
                    (state & GDK_BUTTON1_MASK ? CV_EVENT_FLAG_LBUTTON : 0) |
                    (state & GDK_BUTTON2_MASK ? CV_EVENT_FLAG_MBUTTON : 0) |
                    (state & GDK_BUTTON3_MASK ? CV_EVENT_FLAG_RBUTTON : 0);
        window->on_mouse( cv_event, pt.x, pt.y, flags, window->on_mouse_param );
    }
    
    return FALSE;    
}


static gboolean icvAlarm( gpointer user_data )
{
    *(int*)user_data = 1;
    return FALSE;
}


CV_IMPL int cvWaitKey( int delay )
{
#ifdef HAVE_GTHREAD
	if(thread_started && g_thread_self()!=window_thread){
		gboolean expired;
		int my_last_key;

		// wait for signal or timeout if delay > 0
		if(delay>0){	
			GTimeVal timer;
			g_get_current_time(&timer);
			g_time_val_add(&timer, delay);
			expired = !g_cond_timed_wait(cond_have_key, last_key_mutex, &timer);
		}
		else{
			g_cond_wait(cond_have_key, last_key_mutex);
			expired=false;
		}
		my_last_key = last_key;
		g_mutex_unlock(last_key_mutex);
		if(expired || hg_windows==0){
			return -1;
		}
		return my_last_key;
	}
	else{
#endif
		int expired = 0;
		guint timer = 0;
		if( delay > 0 )
			timer = g_timeout_add( delay, icvAlarm, &expired );
		last_key = -1;
		while( gtk_main_iteration_do(TRUE) && last_key < 0 && !expired && hg_windows != 0 )
			;

		if( delay > 0 && !expired )
			g_source_remove(timer);
#ifdef HAVE_GTHREAD
	}
#endif
	return last_key;
}


#else // #ifndef HAVE_GTK

#define CV_NO_GTK_ERROR(funcname) \
    cvError( CV_StsError, funcname, \
    "The function is not implemented. " \
    "Rebuild the library with GTK+ 2.x support", \
    __FILE__, __LINE__ )

CV_IMPL int cvNamedWindow( const char*, int )
{
    CV_NO_GTK_ERROR("cvNamedWindow");
    return -1;
}    

CV_IMPL void cvDestroyWindow( const char* )
{
    CV_NO_GTK_ERROR( "cvDestroyWindow" );
}

CV_IMPL void
cvDestroyAllWindows( void )
{
    CV_NO_GTK_ERROR( "cvDestroyAllWindows" );
}

CV_IMPL void
cvShowImage( const char*, const CvArr* )
{
    CV_NO_GTK_ERROR( "cvShowImage" );
}

CV_IMPL void cvResizeWindow( const char*, int, int )
{
    CV_NO_GTK_ERROR( "cvResizeWindow" );
}

CV_IMPL void cvMoveWindow( const char*, int, int )
{
    CV_NO_GTK_ERROR( "cvMoveWindow" );
}

CV_IMPL int
cvCreateTrackbar( const char*, const char*,
                  int*, int, CvTrackbarCallback )
{
    CV_NO_GTK_ERROR( "cvCreateTrackbar" );
    return -1;
}

CV_IMPL void
cvSetMouseCallback( const char*, CvMouseCallback, void* )
{
    CV_NO_GTK_ERROR( "cvSetMouseCallback" );
}

CV_IMPL int cvGetTrackbarPos( const char*, const char* )
{
    CV_NO_GTK_ERROR( "cvGetTrackbarPos" );
    return -1;
}

CV_IMPL void cvSetTrackbarPos( const char*, const char*, int )
{
    CV_NO_GTK_ERROR( "cvSetTrackbarPos" );
}

CV_IMPL void* cvGetWindowHandle( const char* )
{
    CV_NO_GTK_ERROR( "cvGetWindowHandle" );
    return 0;
}
    
CV_IMPL const char* cvGetWindowName( void* )
{
    CV_NO_GTK_ERROR( "cvGetWindowName" );
    return 0;
}

CV_IMPL int cvWaitKey( int )
{
    CV_NO_GTK_ERROR( "cvWaitKey" );
    return -1;
}

CV_IMPL int cvInitSystem( int argc, char** argv )
{

    CV_NO_GTK_ERROR( "cvInitSystem" );
    return -1;
}

CV_IMPL int cvStartWindowThread()
{

    CV_NO_GTK_ERROR( "cvStartWindowThread" );
    return -1;
}

#endif
#endif

/* End of file. */
