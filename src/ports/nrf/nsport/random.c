#include "ns/contiki.h"
#include "ns/lib/random.h"

#include "nrf_rng.h"

static inline uint32_t gen_hw_random(void)
{
    uint32_t retval = 0;
    uint8_t *p_retval = (uint8_t *)&retval;

    nrf_rng_event_clear(NRF_RNG_EVENT_VALRDY);
    nrf_rng_task_trigger(NRF_RNG_TASK_START);

    for (uint16_t i = 0; i < 4; i++) {
        while (!nrf_rng_event_get(NRF_RNG_EVENT_VALRDY)) {
            ; // wait for random generator value is ready
        }

        nrf_rng_event_clear(NRF_RNG_EVENT_VALRDY);
        p_retval[i] = nrf_rng_random_value_get();
    }

    nrf_rng_task_trigger(NRF_RNG_TASK_STOP);

    return retval;
}

static inline int rand_30bit(void)
{
    uint32_t val = gen_hw_random();
    return (val & 0x3fffffff);
}

static inline int rand_below(int n)
{
    return rand_30bit() % n;
}

void random_init(unsigned short seed)
{
    (void)seed; // use hardware random generator
}

unsigned short random_rand(void)
{
    return (unsigned short)rand_below(RANDOM_RAND_MAX);
}
