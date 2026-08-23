/*
    ======================================================================================================
    this is code for a random seed in C that will be returned to Go for debugging and session (re)storing,
    makes use of CGO to "import it" into Go the "header" is Seed.go.
    IDE's may report errors regarding line 17 to 23 because they cannot find the header file for the other platform

    ======================================================================================================
*/

int get_random_number(void);

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#if defined(_WIN32)
#include <windows.h>
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif


typedef struct
{
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

static pcg32_random_t global_rng = {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL};


static bool get_os_entropy(void *buf, size_t len)
{
#if defined(_WIN32)
    HCRYPTPROV hProv;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
    {
        BOOL success = CryptGenRandom(hProv, (DWORD)len, (BYTE *)buf);
        CryptReleaseContext(hProv, 0);
        if (success)
            return true;
    }
    return false;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1)
        return false;

    size_t bytes_read = 0;
    while (bytes_read < len)
    {
        ssize_t result = read(fd, (char *)buf + bytes_read, len - bytes_read);
        if (result < 0)
        {
            if (errno == EINTR)
                continue;
            close(fd);
            return false;
        }
        bytes_read += result;
    }
    close(fd);
    return true;
#endif
}


void init_c_random(void)
{
    uint64_t os_seed[2];
    if (!get_os_entropy(os_seed, sizeof(os_seed)))
    {
        
        os_seed[0] = (uint64_t)time(NULL);
        os_seed[1] = (uint64_t)((uintptr_t)&os_seed);
    }

   
    global_rng.state = 0U;
    global_rng.inc = (os_seed[1] << 1ULL) | 1ULL;

    
    (void)get_random_number();
    global_rng.state += os_seed[0];
    (void)get_random_number();
}


int get_random_number(void)
{
    uint64_t oldstate = global_rng.state;

   
    global_rng.state = oldstate * 6364136223846793005ULL + global_rng.inc;

    
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18U) ^ oldstate) >> 27U);
    uint32_t rot = (uint32_t)(oldstate >> 59U);

    uint32_t raw_val = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    return (int)(raw_val & 0x7FFFFFFF);
}