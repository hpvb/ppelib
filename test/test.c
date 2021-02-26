#include <stdio.h>
#include <pelib/pelib.h>

int main() {
	printf("Hello world!\n");
	pelib_file* f = pelib_create();
	pelib_destroy(f);
}
