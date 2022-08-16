#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static const char Key[]  = "STarYZgr3DL11";
static const int  KeyLen = 13;
unsigned int crc32b(unsigned char *message) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

uint32_t calculate(char *data, uint64_t sz)
{
    return crc32b(data);
}

int main(int argc, char **argv)
{
    FILE *in, *out;
    char *data, *outname, *filename, *ext, *cs1, *calc, *cs2;
    size_t sz;
    uint32_t checksum;
    
    if (argc != 2)
    {
        printf("Usage: %s in.sav\n", argv[0]);
        return 0;
    }
    
    if ((in = fopen(argv[1], "rb")) == NULL)
    {
        perror("Error");
        return 1;
    }
    
    filename = malloc(sizeof(argv[1]));
    strcpy(filename, argv[1]);
    ext = strchr(filename, '.');
    filename[(int)(ext - filename)] = '\0';
    
    outname = strcat(filename, ".json");
    
    if ((out = fopen(outname, "wb")) == NULL)
    {
        perror("Error");
        fclose(in);
        return 1;
    }
    
    printf("Writing %s to %s...\n", argv[1], outname);
    
    fseek(in, 0, SEEK_END);
    sz = ftell(in) - 12;
    data = malloc(sz);
    calc = malloc(4);
    cs1 = malloc(8);
    cs2 = malloc(4);
    rewind(in);
    int t = fread(data, 1, sz, in);
    printf("Total read: %zu| SZ: %i\n", t, sz);
    fread(cs1, 1, 8, in);
    fread(cs2, 1, 4, in);
    fclose(in);
    
    
    for (int i = 0; i < sz; i++)
        data[i] ^= Key[i % KeyLen];
    
    checksum = calculate(data, (uint64_t)sz);
    
    printf("calc checksum: %X\n", checksum);
    for(int i = 0; i < 4; i++)
    {
        calc[i] = checksum >> (i*8);
        printf("%d-", (unsigned char)calc[i]);
    }
    
    // remove checksum from end of file to get proper json
    
    fwrite(data, 1, sz, out);
    
    printf("\nchecksum1 %i...\n", sizeof cs1);
    for (int i = 0; i < 8; i++) {
        printf("%d-", (unsigned char)cs1[i]);
    }
    
    printf("\nchecksum2 %i...\n", sizeof cs2);
    for (int i = 0; i < 4; i++) {
        printf("%d-", (unsigned char)cs2[i]);
    }
    
    int sum_c = 0;
    for (int i = 0; i < 4; i++)
    {
        sum_c += cs1[i] << (i*8);
        
    }
    printf("\nTotal checksum %X", sum_c);
    
    free(data);
    free(cs1);
	free(calc);
    fclose(out);
    
    puts("\nDone!");
    
    return 0;
}