/*- FUNCS.C ----------------------------------------------------------------*/
#if defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

/*- terminate the program reporting an error -------------------------------*/

static void ProgError( char *errstr, ...)
{
   va_list args;

   va_start( args, errstr);
   fprintf(stderr, "\nProgram Error: *** ");
   vfprintf( stderr, errstr, args);
   fprintf(stderr, " ***\n");
   va_end( args);
   exit( 5);
}

/*- allocate memory with error checking ------------------------------------*/

static INLINE void *GetMemory( size_t size)
{
   void *ret = malloc( size);
   if (!ret)
      ProgError( "out of memory (cannot allocate %u bytes)", size);
   return ret;
}

/*- reallocate memory with error checking ----------------------------------*/

static INLINE void *ResizeMemory( void *old, size_t size)
{
   void *ret = realloc( old, size);
   if (!ret)
      ProgError( "out of memory (cannot reallocate %u bytes)", size);
   return ret;
}


/*--------------------------------------------------------------------------*/
