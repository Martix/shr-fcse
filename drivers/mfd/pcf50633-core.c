/* NXP PCF50633 Power Management Unit (PMU) driver
 *
 * (C) 2006-2008 by Openmoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 * 	   Balaji Rao <balajirrao@openmoko.org>
 * All rights reserved.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/pm.h>
#include <linux/slab.h>

#include <linux/mfd/core.h>

#include <linux/mfd/pcf50633/core.h>

static int __pcf50633_read(struct pcf50633 *pcf, u8 reg, int num, u8 *data)
{
	int ret;

	ret = i2c_smbus_read_i2c_block_data(pcf->i2c_client, reg,
				num, data);
	if (ret < 0)
		dev_err(pcf->dev, "Error reading %d regs at %d\n", num, reg);

	return ret;
}

static int __pcf50633_write(struct pcf50633 *pcf, u8 reg, int num, u8 *data)
{
	int ret;

	ret = i2c_smbus_write_i2c_block_data(pcf->i2c_client, reg,
				num, data);
	if (ret < 0)
		dev_err(pcf->dev, "Error writing %d regs at %d\n", num, reg);

	return ret;

}

/* Read a block of upto 32 regs  */
int pcf50633_read_block(struct pcf50633 *pcf, u8 reg,
					int nr_regs, u8 *data)
{
	int ret;

	mutex_lock(&pcf->lock);
	ret = __pcf50633_read(pcf, reg, nr_regs, data);
	mutex_unlock(&pcf->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pcf50633_read_block);

/* Write a block of upto 32 regs  */
int pcf50633_write_block(struct pcf50633 *pcf , u8 reg,
					int nr_regs, u8 *data)
{
	int ret;

	mutex_lock(&pcf->lock);
	ret = __pcf50633_write(pcf, reg, nr_regs, data);
	mutex_unlock(&pcf->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pcf50633_write_block);

u8 pcf50633_reg_read(struct pcf50633 *pcf, u8 reg)
{
	u8 val;

	mutex_lock(&pcf->lock);
	__pcf50633_read(pcf, reg, 1, &val);
	mutex_unlock(&pcf->lock);

	return val;
}
EXPORT_SYMBOL_GPL(pcf50633_reg_read);

int pcf50633_reg_write(struct pcf50633 *pcf, u8 reg, u8 val)
{
	int ret;

	mutex_lock(&pcf->lock);
	ret = __pcf50633_write(pcf, reg, 1, &val);
	mutex_unlock(&pcf->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pcf50633_reg_write);

int pcf50633_reg_set_bit_mask(struct pcf50633 *pcf, u8 reg, u8 mask, u8 val)
{
	int ret;
	u8 tmp;

	val &= mask;

	mutex_lock(&pcf->lock);
	ret = __pcf50633_read(pcf, reg, 1, &tmp);
	if (ret < 0)
		goto out;

	tmp &= ~mask;
	tmp |= val;
	ret = __pcf50633_write(pcf, reg, 1, &tmp);

out:
	mutex_unlock(&pcf->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pcf50633_reg_set_bit_mask);

int pcf50633_reg_clear_bits(struct pcf50633 *pcf, u8 reg, u8 val)
{
	int ret;
	u8 tmp;

	mutex_lock(&pcf->lock);
	ret = __pcf50633_read(pcf, reg, 1, &tmp);
	if (ret < 0)
		goto out;

	tmp &= ~val;
	ret = __pcf50633_write(pcf, reg, 1, &tmp);

out:
	mutex_unlock(&pcf->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(pcf50633_reg_clear_bits);

/* sysfs attributes */
static ssize_t show_dump_regs(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct pcf50633 *pcf = dev_get_drvdata(dev);
	u8 dump[16];
	int n, n1, idx = 0;
	char *buf1 = buf;
	static u8 address_no_read[] = { /* must be ascending */
		PCF50633_REG_INT1,
		PCF50633_REG_INT2,
		PCF50633_REG_INT3,
		PCF50633_REG_INT4,
		PCF50633_REG_INT5,
		0 /* terminator */
	};

	for (n = 0; n < 256; n += sizeof(dump)) {
		for (n1 = 0; n1 < sizeof(dump); n1++)
			if (n == address_no_read[idx]) {
				idx++;
				dump[n1] = 0x00;
			} else
				dump[n1] = pcf50633_reg_read(pcf, n + n1);

		hex_dump_to_buffer(dump, sizeof(dump), 16, 1, buf1, 128, 0);
		buf1 += strlen(buf1);
		*buf1++ = '\n';
		*buf1 = '\0';
	}

	return buf1 - buf;
}
static DEVICE_ATTR(dump_regs, 0400, show_dump_regs, NULL);

static ssize_t show_resume_reason(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pcf50633 *pcf = dev_get_drvdata(dev);
	int n;

	n = sprintf(buf, "%02x%02x%02x%02x%02x\n",
				pcf->resume_reason[0],
				pcf->resume_reason[1],
				pcf->resume_reason[2],
				pcf->resume_reason[3],
				pcf->resume_reason[4]);

	return n;
}
static DEVICE_ATTR(resume_reason, 0400, show_resume_reason, NULL);

static struct attribute *pcf_sysfs_entries[] = {
	&dev_attr_dump_regs.attr,
	&dev_attr_resume_reason.attr,
	NULL,
};

static struct attribute_group pcf_attr_group = {
	.name	= NULL,			/* put in device directory */
	.attrs	= pcf_sysfs_entries,
};


#ifdef CONFIG_PM_SLEEP
static int pcf50633_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct pcf50633 *pcf = i2c_get_clientdata(client);

	return pcf50633_irq_suspend(pcf);
}

static int pcf50633_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct pcf50633 *pcf = i2c_get_clientdata(client);

	return pcf50633_irq_resume(pcf);
}
#endif

static SIMPLE_DEV_PM_OPS(pcf50633_pm, pcf50633_suspend, pcf50633_resume);

#define PCF50633_CELL(_name) \
	{ \
		.name = _name, \
		.id = -1, \
	} \

#define PCF50633_CELL_RESOURCES(_name, _resources) \
	{ \
		.name = _name, \
		.num_resources = ARRAY_SIZE(_resources), \
		.resources = _resources, \
		.id = -1, \
	} \

#define PCF50633_CELL_ID(_name, _id) \
	{ \
		.name = _name, \
		.id = _id, \
	} \

static struct resource pcf50633_adc_resources[] = {
	{
		.start = PCF50633_IRQ_ADCRDY,
		.end = PCF50633_IRQ_ADCRDY,
		.flags = IORESOURCE_IRQ,
		.name = "ADCRDY",
	}
};

static struct resource pcf50633_input_resources[] = {
	{
		.start = PCF50633_IRQ_ONKEYR,
		.end = PCF50633_IRQ_ONKEYR,
		.flags = IORESOURCE_IRQ,
		.name = "ONKEYR",
	},
	{
		.start = PCF50633_IRQ_ONKEYF,
		.end = PCF50633_IRQ_ONKEYF,
		.flags = IORESOURCE_IRQ,
		.name = "ONKEYF",
	}
};

static struct resource pcf50633_rtc_resources[] = {
	{
		.start = PCF50633_IRQ_ALARM,
		.end = PCF50633_IRQ_ALARM,
		.flags = IORESOURCE_IRQ,
		.name = "ALARM",
	},
	{
		.start = PCF50633_IRQ_SECOND,
		.end = PCF50633_IRQ_SECOND,
		.flags = IORESOURCE_IRQ,
		.name = "SECOND",
	}
};

static struct resource pcf50633_mbc_resources[] = {
	{
		.start = PCF50633_IRQ_ADPINS,
		.end = PCF50633_IRQ_ADPINS,
		.flags = IORESOURCE_IRQ,
		.name = "ADPINS",
	},
	{
		.start = PCF50633_IRQ_ADPREM,
		.end = PCF50633_IRQ_ADPREM,
		.flags = IORESOURCE_IRQ,
		.name = "ADPREM",
	},
	{
		.start = PCF50633_IRQ_USBINS,
		.end = PCF50633_IRQ_USBINS,
		.flags = IORESOURCE_IRQ,
		.name = "USBINS",
	},
	{
		.start = PCF50633_IRQ_USBREM,
		.end = PCF50633_IRQ_USBREM,
		.flags = IORESOURCE_IRQ,
		.name = "USBREM",
	},
	{
		.start = PCF50633_IRQ_BATFULL,
		.end = PCF50633_IRQ_BATFULL,
		.flags = IORESOURCE_IRQ,
		.name = "BATFULL",
	},
	{
		.start = PCF50633_IRQ_CHGHALT,
		.end = PCF50633_IRQ_CHGHALT,
		.flags = IORESOURCE_IRQ,
		.name = "CHGHALT",
	},
	{
		.start = PCF50633_IRQ_THLIMON,
		.end = PCF50633_IRQ_THLIMON,
		.flags = IORESOURCE_IRQ,
		.name = "THLIMON",
	},
	{
		.start = PCF50633_IRQ_THLIMOFF,
		.end = PCF50633_IRQ_THLIMOFF,
		.flags = IORESOURCE_IRQ,
		.name = "THLIMOFF",
	},
	{
		.start = PCF50633_IRQ_USBLIMON,
		.end = PCF50633_IRQ_USBLIMON,
		.flags = IORESOURCE_IRQ,
		.name = "USBLIMON",
	},
	{
		.start = PCF50633_IRQ_USBLIMOFF,
		.end = PCF50633_IRQ_USBLIMOFF,
		.flags = IORESOURCE_IRQ,
		.name = "USBLIMOFF",
	},
	{
		.start = PCF50633_IRQ_LOWSYS,
		.end = PCF50633_IRQ_LOWSYS,
		.flags = IORESOURCE_IRQ,
		.name = "LOWSYS",
	},
	{
		.start = PCF50633_IRQ_LOWBAT,
		.end = PCF50633_IRQ_LOWBAT,
		.flags = IORESOURCE_IRQ,
		.name = "LOWBAT",
	},
};

static struct mfd_cell pcf50633_cells[] = {
	PCF50633_CELL_RESOURCES("pcf50633-input", pcf50633_input_resources),
	PCF50633_CELL_RESOURCES("pcf50633-rtc", pcf50633_rtc_resources),
	PCF50633_CELL_RESOURCES("pcf50633-mbc", pcf50633_mbc_resources),
	PCF50633_CELL_RESOURCES("pcf50633-adc", pcf50633_adc_resources),
	PCF50633_CELL("pcf50633-backlight"),
	PCF50633_CELL("pcf50633-gpio"),
	PCF50633_CELL_ID("pcf50633-regltr", 0),
	PCF50633_CELL_ID("pcf50633-regltr", 1),
	PCF50633_CELL_ID("pcf50633-regltr", 2),
	PCF50633_CELL_ID("pcf50633-regltr", 3),
	PCF50633_CELL_ID("pcf50633-regltr", 4),
	PCF50633_CELL_ID("pcf50633-regltr", 5),
	PCF50633_CELL_ID("pcf50633-regltr", 6),
	PCF50633_CELL_ID("pcf50633-regltr", 7),
	PCF50633_CELL_ID("pcf50633-regltr", 8),
	PCF50633_CELL_ID("pcf50633-regltr", 9),
	PCF50633_CELL_ID("pcf50633-regltr", 10),
};

static int __devinit pcf50633_probe(struct i2c_client *client,
				const struct i2c_device_id *ids)
{
	struct pcf50633 *pcf;
	struct pcf50633_platform_data *pdata = client->dev.platform_data;
	int ret;
	int version, variant;

	if (!client->irq) {
		dev_err(&client->dev, "Missing IRQ\n");
		return -ENOENT;
	}

	pcf = kzalloc(sizeof(*pcf), GFP_KERNEL);
	if (!pcf)
		return -ENOMEM;

	pcf->pdata = pdata;

	mutex_init(&pcf->lock);

	i2c_set_clientdata(client, pcf);
	pcf->dev = &client->dev;
	pcf->i2c_client = client;

	version = pcf50633_reg_read(pcf, 0);
	variant = pcf50633_reg_read(pcf, 1);
	if (version < 0 || variant < 0) {
		dev_err(pcf->dev, "Unable to probe pcf50633\n");
		ret = -ENODEV;
		goto err_free;
	}

	dev_info(pcf->dev, "Probed device version %d variant %d\n",
							version, variant);

	ret = pcf50633_irq_init(pcf, client->irq);
	if (ret)
		goto err_free;

	ret = mfd_add_devices(pcf->dev, 0, pcf50633_cells,
			ARRAY_SIZE(pcf50633_cells), NULL, pcf->irq_base);
	if (ret) {
		dev_err(pcf->dev, "Failed to add mfd cells.\n");
		goto err_irq_free;
	}

	ret = sysfs_create_group(&client->dev.kobj, &pcf_attr_group);
	if (ret)
		dev_err(pcf->dev, "error creating sysfs entries\n");

	if (pdata->probe_done)
		pdata->probe_done(pcf);

	return 0;

err_irq_free:
	pcf50633_irq_free(pcf);
err_free:
	kfree(pcf);

	return ret;
}

static int __devexit pcf50633_remove(struct i2c_client *client)
{
	struct pcf50633 *pcf = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &pcf_attr_group);

	mfd_remove_devices(pcf->dev);

	pcf50633_irq_free(pcf);

	kfree(pcf);

	return 0;
}

static const struct i2c_device_id pcf50633_id_table[] = {
	{"pcf50633", 0x73},
	{/* end of list */}
};
MODULE_DEVICE_TABLE(i2c, pcf50633_id_table);

static struct i2c_driver pcf50633_driver = {
	.driver = {
		.name	= "pcf50633",
		.pm	= &pcf50633_pm,
	},
	.id_table = pcf50633_id_table,
	.probe = pcf50633_probe,
	.remove = __devexit_p(pcf50633_remove),
};

static int __init pcf50633_init(void)
{
	return i2c_add_driver(&pcf50633_driver);
}

static void __exit pcf50633_exit(void)
{
	i2c_del_driver(&pcf50633_driver);
}

MODULE_DESCRIPTION("I2C chip driver for NXP PCF50633 PMU");
MODULE_AUTHOR("Harald Welte <laforge@openmoko.org>");
MODULE_LICENSE("GPL");

subsys_initcall(pcf50633_init);
module_exit(pcf50633_exit);
