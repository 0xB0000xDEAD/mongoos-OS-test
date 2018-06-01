#include <mgos.h>
#include "mgos_timers.h"
#include "mgos_gpio.h"

static int count = 0; // the encoder position

int vect[3];
void *param = vect;

void encInit(int a, int b, int btn)
{
  vect[0] = a;
  vect[1] = b;
  vect[2] = btn;
  // debug
  for (int i = 0; i < 3; i++)
  {
    LOG(LL_INFO, ("vect[%d] = %d", i, vect[i]));
  }

  mgos_gpio_set_mode(a, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_pull(a, MGOS_GPIO_PULL_UP);

  mgos_gpio_set_mode(b, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_pull(b, MGOS_GPIO_PULL_UP);

  mgos_gpio_set_mode(btn, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_pull(btn, MGOS_GPIO_PULL_UP);
}
int getCount()
{
  return count;
}

// int to bit string helper
char *int2bin(unsigned n, char *buf)
{
#define BITS (sizeof(n) * CHAR_BIT)

  static char static_buf[BITS + 1];
  int i;

  if (buf == NULL)
    buf = static_buf;

  for (i = BITS - 1; i >= 0; --i)
  {
    buf[i] = (n & 1) ? '1' : '0';
    n >>= 1;
  }

  buf[BITS] = '\0';
  return buf;

#undef BITS
}
char printBuf[8]; // 8 bit buffer

void my_timer_cb(void *arg)
{
  // LOG(LL_INFO, ("uptime: %.2lf", mgos_uptime()));
  // // bool val = mgos_gpio_toggle(mgos_sys_config_get_pins_led());
  // (void) arg; // nessuna idea

  // printf("\nvalue is: %d", *(int*)arg); // https://stackoverflow.com/questions/15468441/dereference-void-pointer#15468474

  // LOG(LL_INFO, ("address of arg is: %p", arg));
  // LOG(LL_INFO, ("casting arg to int results to: %p", (int*)arg));
  // LOG(LL_INFO, ("...now deferencing it results to: %d", *(int*)arg));

  // orginal from Oleg
  static int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; //encoder lookup table

  // from 0XPIT flaky version
  // static const int8_t enc_states[] = {0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0};

  static uint8_t state = 0; //inti lookup table index to 0B0000

  int *cast = (int *)arg; //casting void pointer to int pointer

  uint8_t input = NULL;

  input = (mgos_gpio_read(*cast));
  input <<= 1;
  cast++;
  input |= (mgos_gpio_read(*cast));

  // if (mgos_gpio_read(*cast))
  //   input |= 2;
  // cast++;
  // if (!mgos_gpio_read(*cast))
  //   input |= 1;

  state <<= 2;      //remember previous state 0B1100
  state |= (input); // add input to state

  // LOG(LL_INFO, ("Input  is:\n%s", int2bin(input, NULL)));
  // LOG(LL_INFO, ("Lut index is:\n%d <==(0B%s)", (state & 0x0f), int2bin((state & 0x0f), NULL)));
  volatile int8_t inc = enc_states[(state & 0x0f)];
  count += inc;
  // if(inc) {
  //   updateRing(count);
  // }

  // LOG(LL_INFO, ("LUT locked on:\n%d <==(0B%s)\n\n\n", enc_states[(state & 0x0f)], int2bin(enc_states[(state & 0x0f)], NULL)));
}

enum mgos_app_init_result mgos_app_init(void)
{
  mgos_set_hw_timer(1000, MGOS_TIMER_REPEAT, my_timer_cb, param);
  return MGOS_APP_INIT_SUCCESS;
}
