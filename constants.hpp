
#define TEST_ITERATIONS 16384*1
#define MAX_RAND_NUM 16u
#define AVX_WIDTH_FLOAT (int)(256/(sizeof(float)*8))
#define AVX512_WIDTH_FLOAT (int)(512/(sizeof(float)*8))
#define VECTOR_SIZE AVX512_WIDTH_FLOAT*4096*4
