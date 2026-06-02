/*
 * pwm_z42.h — por Prof. Gustavo Rehder (gprehder/pwm)
 * Cópia local com aliases de compatibilidade para MKL25Z4.h estilo Freescale SDK.
 */
#ifndef SOURCES_PWM_H_
#define SOURCES_PWM_H_

#include "stdbool.h"
#include <stdint.h>
#include "MKL25Z4.h"

/* Aliases: mapeiam nomes CMSIS-2 para os ponteiros base do MKL25Z4.h */
#ifndef SIM
#define SIM    SIM_BASE_PTR
#endif
#ifndef TPM0
#define TPM0   TPM0_BASE_PTR
#endif
#ifndef TPM1
#define TPM1   TPM1_BASE_PTR
#endif
#ifndef TPM2
#define TPM2   TPM2_BASE_PTR
#endif
#ifndef GPIOA
#define GPIOA  PTA_BASE_PTR
#endif
#ifndef GPIOB
#define GPIOB  PTB_BASE_PTR
#endif
#ifndef GPIOC
#define GPIOC  PTC_BASE_PTR
#endif
#ifndef GPIOD
#define GPIOD  PTD_BASE_PTR
#endif
#ifndef GPIOE
#define GPIOE  PTE_BASE_PTR
#endif
#ifndef PORTA
#define PORTA  PORTA_BASE_PTR
#endif
#ifndef PORTB
#define PORTB  PORTB_BASE_PTR
#endif
#ifndef PORTC
#define PORTC  PORTC_BASE_PTR
#endif
#ifndef PORTD
#define PORTD  PORTD_BASE_PTR
#endif
#ifndef PORTE
#define PORTE  PORTE_BASE_PTR
#endif

/* TPM clock source select */
#define TPM_CLK_DIS   0
#define TPM_PLLFLL    1
#define TPM_OSCERCLK  2
#define TPM_MCGIRCLK  3

#define TPM_CNT_DIS   0
#define TPM_CLK       1
#define TPM_EXT_CLK   2

/* Prescaler */
#define PS_1    0
#define PS_2    1
#define PS_4    2
#define PS_8    3
#define PS_16   4
#define PS_32   5
#define PS_64   6
#define PS_128  7

/* Channel modes */
#define TPM_OC_TOGGLE  TPM_CnSC_MSA_MASK|TPM_CnSC_ELSA_MASK
#define TPM_OC_CLR     TPM_CnSC_MSA_MASK|TPM_CnSC_ELSB_MASK
#define TPM_OC_SET     TPM_CnSC_MSA_MASK|TPM_CnSC_ELSA_MASK|TPM_CnSC_ELSB_MASK
#define TPM_OC_OUTL    TPM_CnSC_MSB_MASK|TPM_CnSC_MSA_MASK|TPM_CnSC_ELSB_MASK
#define TPM_OC_OUTH    TPM_CnSC_MSB_MASK|TPM_CnSC_MSA_MASK|TPM_CnSC_ELSA_MASK

#define TPM_PWM_H   TPM_CnSC_MSB_MASK|TPM_CnSC_ELSB_MASK
#define TPM_PWM_L   TPM_CnSC_MSB_MASK|TPM_CnSC_ELSA_MASK

#define EDGE_PWM    0
#define CENTER_PWM  1

/* TPM_MemMapPtr e GPIO_MemMapPtr ja sao tipos definidos em MKL25Z4.h */

bool pwm_tpm_Init(TPM_MemMapPtr tpm, uint16_t clk, uint16_t module,
                  uint8_t clock_mode, uint8_t ps, bool counting_mode);

bool pwm_tpm_Ch_Init(TPM_MemMapPtr tpm, uint16_t channel, uint8_t mode,
                     GPIO_MemMapPtr gpio, uint8_t pin);

void pwm_tpm_CnV(TPM_MemMapPtr TPMx, uint16_t channel, uint16_t value);

#endif /* SOURCES_PWM_H_ */
