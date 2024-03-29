#ifndef _GTA02_H
#define _GTA02_H

#include <mach/regs-gpio.h>

/* Different hardware revisions, passed in ATAG_REVISION by u-boot */
#define GTA02v1_SYSTEM_REV	0x00000310
#define GTA02v2_SYSTEM_REV	0x00000320
#define GTA02v3_SYSTEM_REV	0x00000330
#define GTA02v4_SYSTEM_REV	0x00000340
#define GTA02v5_SYSTEM_REV	0x00000350
/* since A7 is basically same as A6, we use A6 PCB ID */
#define GTA02v6_SYSTEM_REV	0x00000360

#define GTA02_GPIO_n3DL_GSM	S3C2410_GPA(13)	/* v1 + v2 + v3 only */

#define GTA02_GPIO_PWR_LED1	S3C2410_GPB(0)
#define GTA02_GPIO_PWR_LED2	S3C2410_GPB(1)
#define GTA02_GPIO_AUX_LED	S3C2410_GPB(2)
#define GTA02_GPIO_VIBRATOR_ON	S3C2410_GPB(3)
#define GTA02_GPIO_MODEM_RST	S3C2410_GPB(5)
#define GTA02_GPIO_BT_EN	S3C2410_GPB(6)
#define GTA02_GPIO_MODEM_ON	S3C2410_GPB(7)
#define GTA02_GPIO_EXTINT8	S3C2410_GPB(8)
#define GTA02_GPIO_USB_PULLUP	S3C2410_GPB(9)

#define GTA02_GPIO_PIO5		S3C2410_GPC(5)	/* v3 + v4 only */

#define GTA02v3_GPIO_nG1_CS	S3C2410_GPD(12)	/* v3 + v4 only */
#define GTA02v3_GPIO_nG2_CS	S3C2410_GPD(13)	/* v3 + v4 only */
#define GTA02v5_GPIO_HDQ	S3C2410_GPD(14)   /* v5 + */

#define GTA02_GPIO_nG1_INT	S3C2410_GPF(0)
#define GTA02_GPIO_IO1		S3C2410_GPF(1)
#define GTA02_GPIO_PIO_2	S3C2410_GPF(2)	/* v2 + v3 + v4 only */
#define GTA02_GPIO_JACK_INSERT	S3C2410_GPF(4)
#define GTA02_GPIO_WLAN_GPIO1	S3C2410_GPF(5)	/* v2 + v3 + v4 only */
#define GTA02_GPIO_AUX_KEY	S3C2410_GPF(6)
#define GTA02_GPIO_HOLD_KEY	S3C2410_GPF(7)

#define GTA02_GPIO_3D_IRQ	S3C2410_GPG(4)
#define GTA02v2_GPIO_nG2_INT	S3C2410_GPG(8)	/* v2 + v3 + v4 only */
#define GTA02v3_GPIO_nUSB_OC	S3C2410_GPG(9)	/* v3 + v4 only */
#define GTA02v3_GPIO_nUSB_FLT	S3C2410_GPG(10)	/* v3 + v4 only */
#define GTA02v3_GPIO_nGSM_OC	S3C2410_GPG(11)	/* v3 + v4 only */

#define GTA02_GPIO_AMP_SHUT	S3C2410_GPJ(1)	/* v2 + v3 + v4 only */
#define GTA02v1_GPIO_WLAN_GPIO10	S3C2410_GPJ(2)
#define GTA02_GPIO_HP_IN	S3C2410_GPJ(2)	/* v2 + v3 + v4 only */
#define GTA02_GPIO_INT0		S3C2410_GPJ(3)	/* v2 + v3 + v4 only */
#define GTA02_GPIO_nGSM_EN	S3C2410_GPJ(4)
#define GTA02_GPIO_3D_RESET	S3C2410_GPJ(5)
#define GTA02_GPIO_nDL_GSM	S3C2410_GPJ(6)	/* v4 + v5 only */
#define GTA02_GPIO_WLAN_GPIO0	S3C2410_GPJ(7)
#define GTA02v1_GPIO_BAT_ID	S3C2410_GPJ(8)
#define GTA02_GPIO_KEEPACT	S3C2410_GPJ(8)
#define GTA02v1_GPIO_HP_IN	S3C2410_GPJ(10)
#define GTA02_CHIP_PWD		S3C2410_GPJ(11)	/* v2 + v3 + v4 only */
#define GTA02_GPIO_nWLAN_RESET	S3C2410_GPJ(12)	/* v2 + v3 + v4 only */

#define GTA02_IRQ_GSENSOR_1	IRQ_EINT0
#define GTA02_IRQ_MODEM		IRQ_EINT1
#define GTA02_IRQ_PIO_2		IRQ_EINT2	/* v2 + v3 + v4 only */
#define GTA02_IRQ_nJACK_INSERT	IRQ_EINT4
#define GTA02_IRQ_WLAN_GPIO1	IRQ_EINT5
#define GTA02_IRQ_AUX		IRQ_EINT6
#define GTA02_IRQ_nHOLD		IRQ_EINT7
#define GTA02_IRQ_PCF50633	IRQ_EINT9
#define GTA02_IRQ_3D		IRQ_EINT12
#define GTA02_IRQ_GSENSOR_2	IRQ_EINT16	/* v2 + v3 + v4 only */
#define GTA02v3_IRQ_nUSB_OC	IRQ_EINT17	/* v3 + v4 only */
#define GTA02v3_IRQ_nUSB_FLT	IRQ_EINT18	/* v3 + v4 only */
#define GTA02v3_IRQ_nGSM_OC	IRQ_EINT19	/* v3 + v4 only */

/* returns 00 000 on GTA02 A5 and earlier, A6 returns 01 001 */
#define GTA02_PCB_ID1_0		S3C2410_GPC(13)
#define GTA02_PCB_ID1_1		S3C2410_GPC(15)
#define GTA02_PCB_ID1_2		S3C2410_GPD(0)
#define GTA02_PCB_ID2_0		S3C2410_GPD(3)
#define GTA02_PCB_ID2_1		S3C2410_GPD(4)

#define GTA02_GPIO_GLAMO_BASE S3C_GPIO_END
#define GTA02_GPIO_GLAMO(x) (GTA02_GPIO_GLAMO_BASE + (x))
#define GTA02_GPIO_PCF_BASE (GTA02_GPIO_GLAMO_BASE + 32)
#define GTA02_GPIO_PCF(x) (GTA02_GPIO_PCF_BASE + (x))

int gta02_get_pcb_revision(void);

#endif /* _GTA02_H */
