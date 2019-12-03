
#ifndef _HUMAN_H_
#define _HUMAN_H_

extern void Human_Init(void);
extern void Humanapp(void);
extern void Human_Task(void *arg);

#define GPIO_HUMAN 27

#define NOHUMAN 0x00
#define HAVEHUMAN 0x01

int havehuman_count;
int nohuman_count;

extern uint8_t human_status;
extern int human_gpio_value;

//int need_send;

TaskHandle_t Human_Handle;

#endif
