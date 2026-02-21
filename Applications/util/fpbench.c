#include <stdio.h>
#include <stdint.h>

/*
 * fpbench: simple floating-point throughput exerciser.
 *
 * Prints integer checksums (not %f/%lf), so it works with minimal printf.
 * Timing is intentionally left to the user (stopwatch).
 */

static volatile float f_sink;
static volatile double d_sink;

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

static uint32_t fp_u32_from_float(float v)
{
	union {
		float f;
		uint32_t u;
	} x;

	x.f = v;
	return x.u;
}

static uint32_t fp_u32_from_double(double v)
{
	union {
		double d;
		uint8_t b[sizeof(double)];
	} x;
	uint32_t h = 0x811C9DC5u;
	uint8_t i;

	x.d = v;
	for (i = 0; i < (uint8_t)sizeof(double); i++) {
		h ^= x.b[i];
		h *= 0x01000193u;
	}
	return h;
}

/*
 * Simple floating point throughput test.
 *
 * - Avoids printing %f/%lf (kernel printf typically doesn't support it).
 * - Uses a volatile sink as a barrier so the loops are kept.
 * - Splits multiply/add into separate statements to discourage ns32k GCC
 *   from generating POLYF/POLYD instructions (not present on 32081).
 */
static uint32_t NOINLINE fp_perf_test_single(uint32_t iterations)
{
	float fa = 1.000123f;
	float fb = 0.999877f;
	uint32_t i;

	for (i = 0; i < iterations; i++) {
		fa = fa * fb;
		f_sink = fa;
		fa = f_sink + 1.414213f;

		fb = fb + 0.000001f;
		f_sink = fb;
		fb = f_sink * 0.999999f;
	}

	return fp_u32_from_float(fa + fb + f_sink);
}

static uint32_t NOINLINE fp_perf_test_double(uint32_t iterations)
{
	double da = 1.000000000123;
	double db = 0.999999999877;
	uint32_t i;

	for (i = 0; i < iterations; i++) {
		da = da * db;
		d_sink = da;
		da = d_sink + 1.414213562373;

		db = db + 0.000000000001;
		d_sink = db;
		db = d_sink * 0.999999999999;
	}

	return fp_u32_from_double(da + db + d_sink);
}

static int parse_u32(const char *s, uint32_t *out)
{
	uint32_t v = 0;
	char c;

	if (s == NULL || *s == 0)
		return 0;
	while ((c = *s++) != 0) {
		if (c < '0' || c > '9')
			return 0;
		v = (v * 10u) + (uint32_t)(c - '0');
	}
	*out = v;
	return 1;
}

int main(int argc, char *argv[])
{
	uint32_t iterations = 10000;   
	uint32_t s_sum, d_sum;

	if (argc > 2) {
		fprintf(stderr, "usage: %s [iterations]\n", argv[0]);
		return 1;
	}
	if (argc == 2) {
		if (!parse_u32(argv[1], &iterations)) {
			fprintf(stderr, "usage: %s [iterations]\n", argv[0]);
			return 1;
		}
	}

	printf("fpbench: %lu iterations\n", (unsigned long)iterations);
	printf("single: begin\n");
	s_sum = fp_perf_test_single(iterations);
	printf("single: 0x%08lx\n", (unsigned long)s_sum);

	printf("double: begin\n");
	d_sum = fp_perf_test_double(iterations);
	printf("double: 0x%08lx\n", (unsigned long)d_sum);

	return 0;
}
