#include "utils.h"

#include <sys/stat.h>
#include <unistd.h>

size_t fskip(FILE *f, size_t count) {
    // Try to seek if regular file
    struct stat st;
    if (fstat(fileno(f), &st) != -1 && st.st_mode & S_IFREG) {
        // Make sure we know our current position to not seek too far
        size_t pos = ftell(f);
        if (pos != -1) {
            size_t seek = MIN(count, st.st_size - pos);
            
            if (fseek(f, seek, SEEK_CUR)) {
                return seek;
            }
        }
    }
    
    // Fallback to consuming bytes
    byte buf[BUF_SIZE];
    size_t total = 0, read;
    do {
        // Read up to BUF_SIZE or remaining bytes to skip
        read = fread(buf, BYTE_SIZE, MIN(BUF_SIZE, count - total), f);
        total += read;
    } while (total < count && read > 0);
    
    return total;
}

size_t fzero(FILE *f, size_t count) {
    // TODO: Try using fseek to extend file
    
    // Write zero bytes manually
    byte buf[BUF_SIZE] = {0};
    size_t total = 0, written;
    do {
        // Write up to BUF_SIZE or remaining bytes to zero
        written = fwrite(buf, BYTE_SIZE, MIN(BUF_SIZE, count - total), f);
        total += written;
    } while (total < count && written > 0);
    
    return total;
}
