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

#include "_cxts.h"
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

CvTest* CvTest::first = 0;
CvTest* CvTest::last = 0;
int CvTest::test_count = 0;

/*****************************************************************************************\
*                                Exception and memory handlers                            *
\*****************************************************************************************/

// a few platform-dependent declarations

#define CV_TS_NORMAL 0
#define CV_TS_BLUE   1
#define CV_TS_GREEN  2
#define CV_TS_RED    4

#ifdef WIN32
#include <windows.h>

#ifdef _MSC_VER
#include <eh.h>
#endif

static void cv_seh_translator( unsigned int /*u*/, EXCEPTION_POINTERS* pExp )
{
    int code = CvTS::FAIL_EXCEPTION;
    switch( pExp->ExceptionRecord->ExceptionCode )
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_IN_PAGE_ERROR:
        code = CvTS::FAIL_MEMORY_EXCEPTION;
        break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
        code = CvTS::FAIL_ARITHM_EXCEPTION;
        break;
    case EXCEPTION_BREAKPOINT:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_INVALID_DISPOSITION:
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_SINGLE_STEP:
        code = CvTS::FAIL_EXCEPTION;
    }
    throw code;
}


#define CV_TS_TRY_BLOCK_BEGIN                   \
    try {

#define CV_TS_TRY_BLOCK_END                     \
    } catch( int _code ) {                      \
        ts->set_failed_test_info( _code );      \
    }

static void change_color( int color )
{
    static int normal_attributes = -1;
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
    fflush(stdout);

    if( normal_attributes < 0 )
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo( hstdout, &info );
        normal_attributes = info.wAttributes;
    }

    SetConsoleTextAttribute( hstdout,
        (WORD)(color == CV_TS_NORMAL ? normal_attributes :
        ((color & CV_TS_BLUE ? FOREGROUND_BLUE : 0)|
        (color & CV_TS_GREEN ? FOREGROUND_GREEN : 0)|
        (color & CV_TS_RED ? FOREGROUND_RED : 0)|FOREGROUND_INTENSITY)) );
}

#else

#include <signal.h>

static const int cv_ts_sig_id[] = { SIGSEGV, SIGBUS, SIGFPE, SIGILL, -1 };

static jmp_buf cv_ts_jmp_mark;

void cv_signal_handler( int sig_code )
{
    int code = CvTS::FAIL_EXCEPTION;
    switch( sig_code )
    {
    case SIGFPE:
        code = CvTS::FAIL_ARITHM_EXCEPTION;
        break;
    case SIGSEGV:
    case SIGBUS:
        code = CvTS::FAIL_ARITHM_EXCEPTION;
        break;
    case SIGILL:
        code = CvTS::FAIL_EXCEPTION;
    }

    longjmp( cv_ts_jmp_mark, code );
}

#define CV_TS_TRY_BLOCK_BEGIN                   \
    {                                           \
        int _code = setjmp( cv_ts_jmp_mark );   \
        if( !_code ) {

#define CV_TS_TRY_BLOCK_END                     \
        }                                       \
        else  {                                 \
            ts->set_failed_test_info( _code );  \
        }                                       \
    }

static void change_color( int color )
{
    static const uchar ansi_tab[] = { 30, 34, 32, 36, 31, 35, 33, 37 };
    char buf[16];
    int code = 0;
    fflush( stdout );
    if( color != CV_TS_NORMAL )
        code = ansi_tab[color & (CV_TS_BLUE|CV_TS_GREEN|CV_TS_RED)];
    sprintf( buf, "\x1b[%dm", code );
    printf( buf );
}

#endif


/***************************** memory manager *****************************/

typedef struct CvTestAllocBlock
{
    struct CvTestAllocBlock* prev;
    struct CvTestAllocBlock* next;
    char* origin;
    char* data;
    size_t size;
    int index;
}
CvTestAllocBlock;


class CvTestMemoryManager
{
public:
    CvTestMemoryManager( CvTS* ts );
    virtual ~CvTestMemoryManager();

    virtual void clear_and_check( int min_index = -1 );
    virtual void start_tracking( int index_to_stop_at=-1 );
    virtual void stop_tracking_and_check();
    int get_alloc_index() { return index; }

    static void* alloc_proxy( size_t size, void* userdata );
    static int free_proxy( void* ptr, void* userdata );

protected:

    virtual void* alloc( size_t size );
    virtual int free( void* ptr );
    virtual int free_block( CvTestAllocBlock* block );

    int index;
    int track_blocks;
    int show_msg_box;
    int index_to_stop_at;
    const char* guard_pattern;
    int guard_size;
    int block_align;
    enum { MAX_MARKS = 1024 };
    int marks[MAX_MARKS];
    int marks_top;
    CvTS* ts;
    CvTestAllocBlock* first;
    CvTestAllocBlock* last;
};


void* CvTestMemoryManager::alloc_proxy( size_t size, void* userdata )
{
    return ((CvTestMemoryManager*)userdata)->alloc( size );
}


int CvTestMemoryManager::free_proxy( void* ptr, void* userdata )
{
    return ((CvTestMemoryManager*)userdata)->free( ptr );
}


CvTestMemoryManager::CvTestMemoryManager( CvTS* _test_system )
{
    ts = _test_system;
    guard_pattern = "THIS IS A GUARD PATTERN!";
    guard_size = strlen(guard_pattern);
    block_align = CV_MALLOC_ALIGN;
    track_blocks = 0;
    marks_top = 0;
    first = last = 0;
    index = 0;
    index_to_stop_at = -1;
    show_msg_box = 1;
}


CvTestMemoryManager::~CvTestMemoryManager()
{
    clear_and_check();
}


void CvTestMemoryManager::clear_and_check( int min_index )
{
    int alloc_index = -1;
    CvTestAllocBlock* block;
    int leak_size = 0, leak_block_count = 0, mem_size = 0;
    void* mem_addr = 0; 

    while( marks_top > 0 && marks[marks_top - 1] >= min_index )
        marks_top--;

    for( block = last; block != 0; )
    {
        CvTestAllocBlock* prev = block->prev;
        if( block->index < min_index )
            break;
        leak_size += block->size;
        leak_block_count++;
        alloc_index = block->index;
        mem_addr = block->data;
        mem_size = block->size;
        free_block( block );
        block = prev;
    }
    track_blocks--;
    if( leak_block_count > 0 )
    {
        ts->set_failed_test_info( CvTS::FAIL_MEMORY_LEAK, alloc_index );
        ts->printf( CvTS::LOG, "Memory leaks: %u blocks, %u bytes total\n"
                    "%s leaked block: %p, %u bytes\n",
                    leak_block_count, leak_size, leak_block_count > 1 ? "The first" : "The",
                    mem_addr, mem_size ); 
    }

    index = block ? block->index + 1 : 0;
}


void CvTestMemoryManager::start_tracking( int _index_to_stop_at )
{
    track_blocks--;
    marks[marks_top++] = index;
    assert( marks_top <= MAX_MARKS );
    track_blocks+=2;
    index_to_stop_at = _index_to_stop_at >= index ? _index_to_stop_at : -1;
}


void CvTestMemoryManager::stop_tracking_and_check()
{
    if( marks_top > 0 )
    {
        int min_index = marks[--marks_top];
        clear_and_check( min_index );
    }
}


int CvTestMemoryManager::free_block( CvTestAllocBlock* block )
{
    int code = 0;
    char* data = block->data;
    
    if( block->origin == 0 || ((size_t)block->origin & (sizeof(double)-1)) != 0 )
        code = CvTS::FAIL_MEMORY_CORRUPTION_BEGIN;
        
    if( memcmp( data - guard_size, guard_pattern, guard_size ) != 0 )
        code = CvTS::FAIL_MEMORY_CORRUPTION_BEGIN;
    else if( memcmp( data + block->size, guard_pattern, guard_size ) != 0 )
        code = CvTS::FAIL_MEMORY_CORRUPTION_END;

    if( code >= 0 )
    {
        if( block->prev )
            block->prev->next = block->next;
        else if( first == block )
            first = block->next;

        if( block->next )
            block->next->prev = block->prev;
        else if( last == block )
            last = block->prev;

        free( block->origin );
    }
    else
    {
        ts->set_failed_test_info( code, block->index );
        ts->printf( CvTS::LOG, "Corrupted block (%s): %p, %u bytes\n",
                    code == CvTS::FAIL_MEMORY_CORRUPTION_BEGIN ? "beginning" : "end",
                    block->data, block->size );
    }

    return code;
}


void* CvTestMemoryManager::alloc( size_t size )
{
    char* data;
    CvTestAllocBlock* block;
    size_t new_size = sizeof(*block) + size + guard_size*2 + block_align + sizeof(size_t)*2;
    char* ptr = (char*)malloc( new_size );

    if( !ptr )
        return 0;

    data = (char*)cvAlignPtr( ptr + sizeof(size_t) + sizeof(*block) + guard_size, block_align );
    block = (CvTestAllocBlock*)cvAlignPtr( data - guard_size -
            sizeof(size_t) - sizeof(*block), sizeof(size_t) );
    block->origin = ptr;
    block->data = data;
    block->size = 0;
    block->index = -1;
    block->next = block->prev = 0;
    memcpy( data - guard_size, guard_pattern, guard_size );
    memcpy( data + size, guard_pattern, guard_size );

    if( track_blocks > 0 )
    {
        track_blocks--;
        block->size = size;

        if( index == index_to_stop_at )
        {
            if( show_msg_box )
            {
        #ifdef WIN32
                MessageBox( NULL, "The block that is corrupted and/or not deallocated has been just allocated\n"
                            "Press Ok to start debugging", "Memory Manager", MB_ICONERROR|MB_OK|MB_SYSTEMMODAL );
        #endif
            }
            CV_DBG_BREAK();
        }

        block->index = index++;

        block->prev = last;
        block->next = 0;
        if( last )
            last = last->next = block;
        else
            first = last = block;

        track_blocks++;
    }

    return data;
}


int CvTestMemoryManager::free( void* ptr )
{
    char* data = (char*)ptr;
    CvTestAllocBlock* block = (CvTestAllocBlock*)
        cvAlignPtr( data - guard_size - sizeof(size_t) - sizeof(*block), sizeof(size_t) );

    int code = free_block( block );
    if( code < 0 && ts->is_debug_mode() )
        CV_DBG_BREAK();
    return 0;
}


/***************************** error handler *****************************/

#if 0
static int cvTestErrorCallback( int status, const char* func_name, const char* err_msg,
                         const char* file_name, int line, void* userdata )
{
    if( status < 0 && status != CV_StsBackTrace && status != CV_StsAutoTrace )
        ((CvTS*)userdata)->set_failed_test_info( CvTS::FAIL_ERROR_IN_CALLED_FUNC );

    // print error message
    return cvStdErrReport( status, func_name, err_msg, file_name, line, 0 );
}
#endif

/*****************************************************************************************\
*                                    Base Class for Tests                                 *
\*****************************************************************************************/

CvTest::CvTest( const char* _test_name, const char* _test_funcs, const char* _test_descr ) :
    name(_test_name ? _test_name : ""), tested_functions(_test_funcs ? _test_funcs : ""),
    description(_test_descr ? _test_descr : ""), ts(0)
{
    if( last )
        last->next = this;
    else
        first = this;
    last = this;
    test_count++;
    ts = 0;
}

CvTest::~CvTest()
{
    clear();
}


void CvTest::clear()
{
}


int CvTest::init( CvTS* _test_system )
{
    clear();
    ts = _test_system;
    return read_params( ts->get_file_storage() );
}


int CvTest::read_params( CvFileStorage* /*fs*/ )
{
    return 0;
}


int CvTest::write_default_params(CvFileStorage*)
{
    return 0;
}


int CvTest::get_timing_mode( int )
{
    return ts->get_timing_mode();
}


bool CvTest::can_do_fast_forward()
{
    return true;
}


void CvTest::safe_run( int start_from )
{
    CV_TS_TRY_BLOCK_BEGIN;

    run( start_from );

    CV_TS_TRY_BLOCK_END;
}


void CvTest::run( int start_from )
{
    int i, test_case_idx, count = get_test_case_count();
    int64 t_start = cvGetTickCount();
    double freq = cvGetTickFrequency();
    bool ff = can_do_fast_forward();
    int progress = 0;
    
    for( test_case_idx = ff && start_from >= 0 ? start_from : 0;
         count < 0 || test_case_idx < count; test_case_idx++ )
    {
        ts->update_context( this, test_case_idx, ff );
        int timing_mode = get_timing_mode( test_case_idx );
        int64 t00 = 0, t0, t1 = 0;
        double t_acc = 0;

        if( prepare_test_case( test_case_idx ) < 0 || ts->get_err_code() < 0 )
            return;

        for( i = 0; i < (timing_mode ? 10 : 1); i++ )
        {
            t0 = cvGetTickCount();
            run_func();
            t1 = cvGetTickCount();
            if( ts->get_err_code() < 0 )
                return;

            if( !timing_mode || i == 0 )
            {
                t_acc = (double)(t1 - t0);
                t00 = t0;
            }
            else
            {
                t0 = t1 - t0;
                
                if( timing_mode == CvTS::MIN_TIME )
                {
                    if( (double)t0 < t_acc )
                        t_acc = (double)t0;
                }
                else
                {
                    assert( timing_mode == CvTS::AVG_TIME );
                    t_acc += (double)t0;
                }
                
                if( t1 - t00 > freq*2000000 )
                    break;
            }
        }

        if( timing_mode )
        {
            if( timing_mode == CvTS::AVG_TIME )
                t_acc /= i;
            print_time( test_case_idx, t_acc );
        }

        if( validate_test_results( test_case_idx ) < 0 || ts->get_err_code() < 0 )
            return;

        progress = update_progress( progress, test_case_idx, count, (double)(t1 - t_start)/(freq*1000) );
    }
}


int CvTest::get_test_case_count()
{
    return -1;
}


int CvTest::prepare_test_case( int )
{
    return 0;
}


int CvTest::validate_test_results( int )
{
    return 0;
}


void CvTest::print_time( int /*test_case_idx*/, double /*time_usecs*/ )
{
}


int CvTest::update_progress( int progress, int test_case_idx, int count, double dt )
{
    int width = 60 - strlen(get_name());
    if( count > 0 )
    {
        int t = cvRound( ((double)test_case_idx * width)/count );
        if( t > progress )
        {
            ts->printf( CvTS::CONSOLE, "." );
            progress = t;
        }
    }
    else if( cvRound(dt*0.001) > progress )
    {
        ts->printf( CvTS::CONSOLE, "." );
        progress = cvRound(dt*0.001);
    }

    return progress;
}

/*****************************************************************************************\
*                                 Base Class for Test System                              *
\*****************************************************************************************/

/******************************** Constructors/Destructors ******************************/

CvTS::CvTS()
{
    start_time = 0;
    version = CV_TS_VERSION;
    memory_manager = 0;
    /*
    memory_manager = new CvTestMemoryManager(this);
    cvSetMemoryManager( CvTestMemoryManager::alloc_proxy,
                        CvTestMemoryManager::free_proxy,
                        memory_manager );*/
    ostrm_suffixes[SUMMARY_IDX] = ".sum";
    ostrm_suffixes[LOG_IDX] = ".log";
    ostrm_suffixes[CSV_IDX] = ".csv";
    ostrm_suffixes[CONSOLE_IDX] = 0;
    ostrm_base_name = 0;
    memset( output_streams, 0, sizeof(output_streams) );
    selected_tests = new CvTestPtrVec();
    failed_tests = new CvTestInfoVec();

    clear();
}


void CvTS::clear()
{
    int i;
    CvTest* test;
    for( test = get_first_test(); test != 0; test = test->get_next() )
        test->clear();

    for( i = 0; i <= CONSOLE_IDX; i++ )
    {
        if( i == LOG_IDX )
            fflush( stderr );
        else if( i == CONSOLE_IDX )
            fflush( stdout );

        if( i < CONSOLE_IDX && output_streams[i].f )
        {
            fclose( output_streams[i].f );
            output_streams[i].f = 0;
        }
        
        if( i == LOG_IDX && output_streams[i].default_handle > 0 )
        {
            dup2( output_streams[i].default_handle, 2 );
            output_streams[i].default_handle = 0;
        }
        output_streams[i].enable = 1;
    }
    cvReleaseFileStorage( &fs );
    selected_tests->clear();
    failed_tests->clear();
    if( ostrm_base_name )
    {
        free( ostrm_base_name );
        ostrm_base_name = 0;
    }
    params.rng_seed = (uint64)-1;
    params.debug_mode = 1;
    params.print_only_failed = 0;
    params.skip_header = 0;
    params.timing_mode = NO_TIME;

    if( memory_manager )
        memory_manager->clear_and_check();
}


CvTS::~CvTS()
{
    clear();
    delete selected_tests;
    delete failed_tests;
    cvSetMemoryManager( 0, 0 );
}


const char* CvTS::str_from_code( int code )
{
    switch( code )
    {
    case OK: return "Ok";
    case FAIL_GENERIC: return "Generic/Unknown";
    case FAIL_MISSING_TEST_DATA: return "No test data";
    case FAIL_ERROR_IN_CALLED_FUNC: return "cvError invoked";
    case FAIL_EXCEPTION: return "Hardware/OS exception";
    case FAIL_MEMORY_EXCEPTION: return "Invalid memory access";
    case FAIL_ARITHM_EXCEPTION: return "Arithmetic exception";
    case FAIL_MEMORY_CORRUPTION_BEGIN: return "Corrupted memblock (beginning)";
    case FAIL_MEMORY_CORRUPTION_END: return "Corrupted memblock (end)";
    case FAIL_MEMORY_LEAK: return "Memory leak";
    case FAIL_INVALID_OUTPUT: return "Invalid function output";
    case FAIL_MISMATCH: return "Unexpected output";
    case FAIL_BAD_ACCURACY: return "Bad accuracy";
    case FAIL_HANG: return "Infinite loop(?)";
    case FAIL_BAD_ARG_CHECK: return "Incorrect handling of bad arguments";
    default: return "Generic/Unknown";
    }
}

/************************************** Running tests **********************************/

void CvTS::make_output_stream_base_name( const char* config_name )
{
    int k, len = strlen( config_name );

    if( ostrm_base_name )
        free( ostrm_base_name );

    for( k = len-1; k >= 0; k-- )
    {
        char c = config_name[k];
        if( c == '.' || c == '/' || c == '\\' || c == ':' )
            break;
    }

    if( k > 0 && config_name[k] == '.' )
        len = k;
    
    ostrm_base_name = (char*)malloc( len + 1 );
    memcpy( ostrm_base_name, config_name, len );
    ostrm_base_name[len] = '\0';
}


void CvTS::set_handlers( bool on )
{
    if( on )
    {
        cvSetErrMode( CV_ErrModeParent );
        cvRedirectError( cvStdErrReport );
    #ifdef WIN32
        #ifdef _MSC_VER
        _set_se_translator( cv_seh_translator );
        #endif
    #else
        for( int i = 0; cv_ts_sig_id[i] >= 0; i++ )
            signal( cv_ts_sig_id[i], cv_signal_handler );
    #endif
    }
    else
    {
        cvSetErrMode( CV_ErrModeLeaf );
        cvRedirectError( cvGuiBoxReport );
    #ifdef WIN32
        #ifdef _MSC_VER
        _set_se_translator( 0 );
        #endif
    #else
        for( int i = 0; cv_ts_sig_id[i] >= 0; i++ )
            signal( cv_ts_sig_id[i], SIG_DFL );
    #endif
    }
}

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

int CvTS::run( int argc, char** argv )
{
    time( &start_time );
    
    int i, write_params = 0;
    CvTest* test;

    // 0. reset all the parameters
    clear();

    // 1. parse command line options
    for( i = 1; i < argc; i++ )
    {
        if( argv[i] && argv[i][0] != '-' )
        {
            config_name = argv[i];
            break;
        }
        else
        {
            if( strcmp( argv[i], "-w" ) == 0 )
                write_params = 1;
        }
    }

    if( write_params )
    {
        if( !config_name )
        {
            printf( LOG, "ERROR: output config name is not specified\n" );
            return -1;
        }
        fs = cvOpenFileStorage( config_name, 0, CV_STORAGE_WRITE );
        if( !fs )
        {
            printf( LOG, "ERROR: could not open config file %s", config_name );
            return -1;
        }
        cvWriteComment( fs, CV_TS_VERSION " config file", 0 );
        cvStartWriteStruct( fs, "common", CV_NODE_MAP );
        write_default_params( fs );
        cvEndWriteStruct( fs );

        for( test = get_first_test(); test != 0; test = test->get_next() )
        {
            cvStartWriteStruct( fs, test->get_name(), CV_NODE_MAP );
            test->write_default_params( fs );
            cvEndWriteStruct( fs );
            test->clear();
        }
        cvReleaseFileStorage( &fs );
        return 0;
    }

    if( !config_name )
        printf( LOG, "WARNING: config name is not specified, using default parameters\n" );
    else
    {
        // 2. read common parameters of test system
        fs = cvOpenFileStorage( config_name, 0, CV_STORAGE_READ );
        if( !fs )
        {
            printf( LOG, "ERROR: could not open config file %s", config_name );
            return -1;
        }
    }

    if( read_params(fs) < 0 )
        return -1;

    if( !ostrm_base_name )
        make_output_stream_base_name( config_name ? config_name : argv[0] );

    ostream_testname_mask = -1; // disable printing test names at initial stage

    // 3. open file streams
    for( i = 0; i < CONSOLE_IDX; i++ )
    {
        char filename[MAX_PATH];
        sprintf( filename, "%s%s", ostrm_base_name, ostrm_suffixes[i] );
        output_streams[i].f = fopen( filename, "wt" );
        if( !output_streams[i].f )
        {
            printf( LOG, "ERROR: could not open %s\n", filename );
            return -1;
        }

        if( i == LOG_IDX )
        {
            // redirect stderr to log file
            fflush( stderr );
            output_streams[i].default_handle = dup(2);
            dup2( fileno(output_streams[i].f), 2 );
        }
    }

    // 4. traverse through the list of all registered tests.
    // Initialize the selected tests and put them into the separate sequence
    for( test = get_first_test(); test != 0; test = test->get_next() )
    {
        if( filter(test) )
        {
            if( test->init(this) >= 0 )
                selected_tests->push( test );
            else
                printf( LOG, "WARNING: an error occured during test %s initialization\n", test->get_name() );
        }
    }

    // 5. setup all the neccessary handlers and print header
    set_handlers( !params.debug_mode );

    if( !params.skip_header )
        print_summary_header( SUMMARY + LOG + CONSOLE );
    rng = params.rng_seed;
    update_context( 0, -1, true );

    // 6. run all the tests
    for( i = 0; i < selected_tests->size(); i++ )
    {
        CvTest* test = (CvTest*)selected_tests->at(i);
        int code;
        CvTestInfo temp;

        if( memory_manager )
            memory_manager->start_tracking();
        update_context( test, -1, true );
        ostream_testname_mask = 0; // reset "test name was printed" flags

        temp = current_test_info;
        test->safe_run(0);
        if( get_err_code() >= 0 )
        {
            update_context( test, -1, false );
            current_test_info.rng_seed = temp.rng_seed;
            current_test_info.base_alloc_index = temp.base_alloc_index;
        }
        test->clear();
        if( memory_manager )
            memory_manager->stop_tracking_and_check();

        code = get_err_code();
        if( code >= 0 )
        {
            if( !params.print_only_failed )
            {
                printf( SUMMARY + CONSOLE, "\t" );
                change_color( CV_TS_GREEN );
                printf( SUMMARY + CONSOLE, "Ok\n" );
                change_color( CV_TS_NORMAL );
            }
        }
        else
        {
            printf( SUMMARY + CONSOLE, "\t" );
            change_color( CV_TS_RED );
            printf( SUMMARY + CONSOLE, "FAIL(%s)\n", str_from_code(code) );
            change_color( CV_TS_NORMAL );
            printf( LOG, "context: test case = %d, seed = %08x%08x\n",
                    current_test_info.test_case_idx,
                    (unsigned)(current_test_info.rng_seed>>32),
                    (unsigned)(current_test_info.rng_seed));
            failed_tests->push(current_test_info);
            if( params.rerun_immediately )
                break;
        }
    }

    ostream_testname_mask = -1;
    print_summary_tailer( SUMMARY + CONSOLE + LOG );

    if( !params.debug_mode && (params.rerun_failed || params.rerun_immediately) )
    {
        set_handlers(0);
        update_context( 0, -1, true );
        for( i = 0; i < failed_tests->size(); i++ )
        {
            CvTestInfo info = failed_tests->at(i);
            if( (info.code == FAIL_MEMORY_CORRUPTION_BEGIN ||
                info.code == FAIL_MEMORY_CORRUPTION_END ||
                info.code == FAIL_MEMORY_LEAK) && memory_manager )
                memory_manager->start_tracking( info.alloc_index - info.base_alloc_index
                                                + memory_manager->get_alloc_index() );
            rng = info.rng_seed;
            test->safe_run( info.test_case_idx );
        }
    }

    clear();

    return 0;
}


int CvTS::read_params( CvFileStorage* fs )
{
    CvFileNode* node = fs ? cvGetFileNodeByName( fs, 0, "common" ) : 0;
    params.debug_mode = cvReadIntByName( fs, node, "debug_mode", 1 ) != 0;
    params.skip_header = cvReadIntByName( fs, node, "skip_header", 0 ) != 0;
    params.print_only_failed = cvReadIntByName( fs, node, "print_only_failed", 0 ) != 0;
    params.rerun_failed = cvReadIntByName( fs, node, "rerun_failed", 0 ) != 0;
    params.rerun_immediately = cvReadIntByName( fs, node, "rerun_immediately", 0 ) != 0;
    const char* str = cvReadStringByName( fs, node, "filter_mode", "tests" );
    params.test_filter_mode = strcmp( str, "functions" ) == 0 ? CHOOSE_FUNCTIONS : CHOOSE_TESTS;
    str = cvReadStringByName( fs, node, "timing_mode", "none" );
    params.timing_mode = strcmp( str, "average" ) == 0 || strcmp( str, "avg" ) == 0 ? AVG_TIME :
                         strcmp( str, "minimum" ) == 0 || strcmp( str, "min" ) == 0 ||
                         strcmp( str, "yes" ) == 0 ? MIN_TIME : NO_TIME;
    params.test_filter_pattern = cvReadStringByName( fs, node, params.test_filter_mode == CHOOSE_FUNCTIONS ?
                                                     "functions" : "tests", "" );
    params.resource_path = cvReadStringByName( fs, node, "." );
    str = cvReadStringByName( fs, node, "seed", 0 );
    params.rng_seed = 0;
    if( str && strlen(str) == 16 )
    {
        params.rng_seed = 0;
        for( int i = 0; i < 16; i++ )
        {
            int c = tolower(str[i]);
            if( !isxdigit(c) )
            {
                params.rng_seed = 0;
                break;
            }
            params.rng_seed = params.rng_seed * 16 +
                (str[i] < 'a' ? str[i] - '0' : str[i] - 'a' + 10);
        }
    }
    
    if( params.rng_seed == 0 )
        params.rng_seed = cvGetTickCount();

    str = cvReadStringByName( fs, node, "output_file_base_name", 0 );
    if( str )
        make_output_stream_base_name( str );

    return 0;
}


void CvTS::write_default_params( CvFileStorage* fs )
{
    read_params(0); // fill parameters with default values

    cvWriteInt( fs, "debug_mode", params.debug_mode );
    cvWriteInt( fs, "skip_header", params.skip_header );
    cvWriteInt( fs, "print_only_failed", params.print_only_failed );
    cvWriteInt( fs, "rerun_failed", params.rerun_failed );
    cvWriteInt( fs, "rerun_immediately", params.rerun_immediately );
    cvWriteString( fs, "filter_mode", params.test_filter_mode == CHOOSE_FUNCTIONS ? "functions" : "tests" );
    cvWriteString( fs, "timing_mode", params.timing_mode == NO_TIME ? "none" :
                   params.timing_mode == MIN_TIME ? "min" : "avg" );
    // test_filter, seed & output_file_base_name are not written
}


void CvTS::enable_output_streams( int stream_mask, int value )
{
    for( int i = 0; i < MAX_IDX; i++ )
        if( stream_mask & (1 << i) )
            output_streams[i].enable = value != 0;
}


void CvTS::update_context( CvTest* test, int test_case_idx, bool update_ts_context )
{
    current_test_info.test = test;
    current_test_info.test_case_idx = test_case_idx;
    current_test_info.alloc_index = 0;
    current_test_info.code = 0;
    cvSetErrStatus( CV_StsOk );
    if( update_ts_context )
    {
        current_test_info.rng_seed = rng;
        current_test_info.base_alloc_index = memory_manager ?
            memory_manager->get_alloc_index() : 0;
    }
}


void CvTS::set_failed_test_info( int fail_code, int alloc_index )
{
    if( fail_code == FAIL_MEMORY_CORRUPTION_BEGIN ||
        fail_code == FAIL_MEMORY_CORRUPTION_END ||
        current_test_info.code >= 0 )
    {
        current_test_info.code = fail_code;
        current_test_info.alloc_index = alloc_index;
    }
}


const char* CvTS::get_libs_info( const char** addon_modules )
{
    const char* all_info = 0;
    cvGetModuleInfo( 0, &all_info, addon_modules );
    return all_info;
}

void CvTS::print_summary_header( int streams )
{
    printf( streams, "Engine: %s\n", version );
    time_t t1;
    time( &t1 );
    struct tm *t2 = localtime( &t1 );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );
    printf( streams, "Execution Date & Time: %s\n", buf );
    printf( streams, "Config File: %s\n", config_name );
    const char* addon_modules = 0;
    const char* lib_verinfo = get_libs_info( &addon_modules );
    printf( streams, "Tested Libraries: %s\n", lib_verinfo );
    printf( streams, "Optimized Low-level Addons: %s\n", addon_modules );
    printf( streams, "=================================================\n");
}
    
void CvTS::print_summary_tailer( int streams )
{
    printf( streams, "=================================================\n");
    if( selected_tests && failed_tests )
    {
        time_t end_time;
        time( &end_time );
        double total_time = difftime( end_time, start_time );
        printf( streams, "Summary: %d out of %d tests failed\n",
            failed_tests->size(), selected_tests->size() );
        int minutes = cvFloor(total_time/60.);
        int seconds = cvRound(total_time - minutes*60);
        int hours = minutes / 60;
        minutes %= 60;
        printf( streams, "Running time: %02d:%02d:%02d\n", hours, minutes, seconds );
    }
}


void CvTS::vprintf( int streams, const char* fmt, va_list l )
{
    if( streams )
    {
        char str[1 << 14];
        vsprintf( str, fmt, l );

        for( int i = 0; i < MAX_IDX; i++ )
        {
            if( (streams & (1 << i)) && output_streams[i].enable )
            {
                FILE* f = i == CONSOLE_IDX ? stdout :
                          i == LOG_IDX ? stderr : output_streams[i].f;
                if( f )
                {
                    if( !(ostream_testname_mask & (1 << i)) && current_test_info.test )
                    {
                        fprintf( f, "-------------------------------------------------\n" );
                        fprintf( f, "%s: ", current_test_info.test->get_name() );
                        fflush( f );
                        ostream_testname_mask |= 1 << i;
                    }
                    fputs( str, f );
		    if( i == CONSOLE_IDX )
			fflush(f);
                }
            }
        }
    }
}


void CvTS::printf( int streams, const char* fmt, ... )
{
    if( streams )
    {
        va_list l;
        va_start( l, fmt );
        vprintf( streams, fmt, l );
        va_end( l );
    }
}


static char* cv_strnstr( const char* str, int len,
                         const char* pattern,
                         int pattern_len = -1,
                         int whole_word = 1 )
{
    int i;

    if( len < 0 && pattern_len < 0 )
        return (char*)strstr( str, pattern );

    if( len < 0 )
        len = strlen( str );

    if( pattern_len < 0 )
        pattern_len = strlen( pattern );

    for( i = 0; i < len - pattern_len + 1; i++ )
    {
        int j = i + pattern_len;
        if( str[i] == pattern[0] &&
            memcmp( str + i, pattern, pattern_len ) == 0 &&
            (!whole_word ||
            ((i == 0 || !isalnum(str[i-1]) && str[i-1] != '_') &&
             (j == len || !isalnum(str[j]) && str[j] != '_'))))
            return (char*)(str + i);
    }

    return 0;
}


int CvTS::filter( CvTest* test )
{
    const char* pattern = params.test_filter_pattern;
    if( !pattern || strcmp( pattern, "" ) == 0 || strcmp( pattern, "*" ) == 0 )
        return 1;

    if( params.test_filter_mode == CHOOSE_TESTS )
    {
        int found = 0;
        
        while( pattern && *pattern )
        {
            char *ptr, *endptr = (char*)strchr( pattern, ',' );
            int len, have_wildcard;
            int t_name_len;

            if( endptr )
                *endptr = '\0';

            ptr = (char*)strchr( pattern, '*' );
            if( ptr )
            {
                len = ptr - pattern;
                have_wildcard = 1;
            }
            else
            {
                len = strlen( pattern );
                have_wildcard = 0;
            }

            t_name_len = strlen( test->get_name() );
            found = (t_name_len == len || have_wildcard && t_name_len > len) &&
                    (len == 0 || memcmp( test->get_name(), pattern, len ) == 0);
            if( endptr )
            {
                *endptr = ',';
                pattern = endptr + 1;
                while( isspace(*pattern) )
                    pattern++;
            }

            if( found || !endptr )
                break;
        }

        return found;
    }
    else
    {
        assert( params.test_filter_mode == CHOOSE_FUNCTIONS );
        int glob_len = strlen( pattern );
        const char* ptr = test->get_func_list();
        const char *tmp_ptr;

        while( ptr && *ptr )
        {
            const char* endptr = ptr - 1;
            const char* name_ptr;
            const char* name_first_match;
            int name_len;
            char c;

            do c = *++endptr;
            while( isspace(c) );

            if( !c )
                break;

            assert( isalpha(c) );
            name_ptr = endptr;

            do c = *++endptr;
            while( isalnum(c) || c == '_' );

            if( c == ':' ) // class
            {
                assert( endptr[1] == ':' );
                endptr = endptr + 2;
                name_len = endptr - name_ptr;

                // find the first occurence of the class name
                // in pattern
                name_first_match = cv_strnstr( pattern,
                                      glob_len, name_ptr, name_len, 1 );

                if( *endptr == '*' )
                {
                    if( name_first_match )
                        return 1;
                }
                else
                {
                    assert( *endptr == '{' ); // a list of methods

                    if( !name_first_match )
                    {
                        // skip all the methods, if there is no such a class name
                        // in pattern
                        endptr = strchr( endptr, '}' );
                        assert( endptr != 0 );
                        endptr--;
                    }

                    for( ;; )
                    {
                        const char* method_name_ptr;
                        int method_name_len;

                        do c = *++endptr;
                        while( isspace(c) );

                        if( c == '}' )
                            break;
                        assert( isalpha(c) );

                        method_name_ptr = endptr;
                    
                        do c = *++endptr;
                        while( isalnum(c) || c == '_' );
                    
                        method_name_len = endptr - method_name_ptr;
                    
                        // search for class_name::* or
                        // class_name::{...method_name...}
                        tmp_ptr = name_first_match;
                        do
                        {
                            const char* tmp_ptr2;
                            tmp_ptr += name_len;
                            if( *tmp_ptr == '*' )
                                return 1;
                            assert( *tmp_ptr == '{' );
                            tmp_ptr2 = strchr( tmp_ptr, '}' );
                            assert( tmp_ptr2 );

                            if( cv_strnstr( tmp_ptr, tmp_ptr2 - tmp_ptr + 1,
                                             method_name_ptr, method_name_len, 1 ))
                                return 1;

                            tmp_ptr = cv_strnstr( tmp_ptr2, glob_len -
                                                   (tmp_ptr2 - pattern),
                                                   name_ptr, name_len, 1 );
                        }
                        while( tmp_ptr );

                        endptr--;
                        do c = *++endptr;
                        while( isspace(c) );

                        if( c != ',' )
                            endptr--;
                    }
                }
            }
            else
            {
                assert( !c || isspace(c) || c == ',' );
                name_len = endptr - name_ptr;
                tmp_ptr = pattern;

                for(;;)
                {
                    const char *tmp_ptr2, *tmp_ptr3;

                    tmp_ptr = cv_strnstr( tmp_ptr, glob_len -
                        (tmp_ptr - pattern), name_ptr, name_len, 1 );

                    if( !tmp_ptr )
                        break;

                    // make sure it is not a method
                    tmp_ptr2 = strchr( tmp_ptr, '}' );
                    if( !tmp_ptr2 )
                        return 1;

                    tmp_ptr3 = strchr( tmp_ptr, '{' );
                    if( tmp_ptr3 < tmp_ptr2 )
                        return 1;

                    tmp_ptr = tmp_ptr2 + 1;
                }

                endptr--;
            }

            do c = *++endptr;
            while( isspace(c) );

            if( c == ',' )
                endptr++;
            ptr = endptr;
        }

        return 0;
    }
}

/* End of file. */
