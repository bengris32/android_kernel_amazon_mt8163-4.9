/*
 * Dummy Battery power supply driver
 * based on drivers/power/supply/dummy-ac.c
 *
 * Copyright (C) 2016 Intel Corporation
 * Copyright (c) 2025 Ben Grisdale <bengris32@protonmail.ch>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 */

#include <linux/module.h>
#include <linux/param.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

struct dummy_battery_info {
	struct platform_device *pdev;
	struct power_supply *battery;
	struct power_supply_desc desc;
};

static enum power_supply_property dummy_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static int dummy_battery_remove(struct platform_device *pdev);

static int dummy_battery_get_property(struct power_supply *psy,
				    enum power_supply_property psp,
				    union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = POWER_SUPPLY_STATUS_FULL;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = 100;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		val->intval = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int dummy_battery_probe(struct platform_device *pdev)
{
	struct dummy_battery_info *info;
	struct power_supply_config psy_cfg = {};

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_err(&pdev->dev, "mem alloc failed\n");
		return -ENOMEM;
	}

	info->pdev = pdev;
	platform_set_drvdata(pdev, info);

	info->desc.name = "battery";
	info->desc.type = POWER_SUPPLY_TYPE_BATTERY;
	info->desc.properties = dummy_battery_props;
	info->desc.num_properties = ARRAY_SIZE(dummy_battery_props);
	info->desc.get_property = dummy_battery_get_property;
	psy_cfg.drv_data = info;
	info->battery = power_supply_register(&pdev->dev, &info->desc, &psy_cfg);

	if (IS_ERR(info->battery)){
		dev_err(&pdev->dev, "Failed: power supply register(%ld)\n", PTR_ERR(info->battery));
		dummy_battery_remove(info->pdev);
	}else
		dev_err(&pdev->dev, "dummy battery power supply registered\n");

	return 0;
}

static int dummy_battery_remove(struct platform_device *pdev)
{
	struct dummy_battery_info *info =  dev_get_drvdata(&pdev->dev);

	power_supply_unregister(info->battery);
	return 0;
}
static struct platform_driver dummy_battery_driver = {
	.driver = {
		.name = "dummy-battery",
		.owner	= THIS_MODULE,
	},
	.probe = dummy_battery_probe,
	.remove = dummy_battery_remove,
};

static struct platform_device *dummy_pdev;

static int __init dummy_battery_init(void)
{
	int ret;

	ret =  platform_driver_register(&dummy_battery_driver);
	if (ret < 0) {
		pr_err("dummy-battery driver reg failed\n");
		return ret;
	}

	dummy_pdev = platform_device_alloc("dummy-battery", -1);
	if (!dummy_pdev) {
		pr_err("dummy-battery device alloc failed\n");
		return -ENODEV;
	}

	ret = platform_device_add(dummy_pdev);
	if (ret) {
		pr_err("dummy-battery device add failed\n");
		return ret;
	}

	return 0;
}
device_initcall(dummy_battery_init);

static void __exit dummy_battery_exit(void)
{
	platform_driver_unregister(&dummy_battery_driver);
}
module_exit(dummy_battery_exit);

MODULE_AUTHOR("Ramakrishna Pallala <ramakrishna.pallala@intel.com>");
MODULE_AUTHOR("Ben Grisdale <bengris32@protonmail.ch>");
MODULE_DESCRIPTION("Dummy Battery power supply Driver");
MODULE_LICENSE("GPL");
