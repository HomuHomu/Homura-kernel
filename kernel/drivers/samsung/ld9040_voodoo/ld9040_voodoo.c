/*
 * Author: François SIMOND aka supercurio
 * License: GPLv2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "ld9040_voodoo.h"

static char *config;
module_param(config, charp, 0600);

static struct ld9040_voodoo ldv;

static void parse_config(void)
{
	int i;
	char *name;
	unsigned long addr22;
	unsigned long addr19;
	char *config_original;
	unsigned int bytes_read = 0;

	printk(NAME ": parsing config: %s\n", config);

	name = kzalloc(MAX_NAME_LENGTH, GFP_KERNEL);

	if (config) {
		config_original = config;

		i = 0;
		while (sscanf(config,
			      "%63s %lx:%lx%n",
			      name, &addr22, &addr19, &bytes_read) == 3) {
			config += bytes_read;

			printk(NAME ": config found: gamma set %s "
			       "(2.2 addr: 0x%lx, 1.9 addr: 0x%lx)\n",
			       name, addr22, addr19);

			ldv.gamma_sets[i].addr22 = addr22;
			ldv.gamma_sets[i].addr19 = addr19;
			ldv.gamma_sets[i].name = name;

			name = kzalloc(MAX_NAME_LENGTH, GFP_KERNEL);

			i++;
		}

		kfree(name);
		config = config_original;
	}

}

static gamma_level_pointer get_gamma_level(const char *name, unsigned int level,
					   bool is_gamma19)
{
	gamma_set_pointer gamma_set = NULL;
	gamma_level_pointer gamma_level = NULL;
	int i;

	if (!ldv.gamma_sets) {
		printk(NAME ": driver configuration error\n");
		return NULL;
	}

	for (i = 0; i < MAX_GAMMA_SETS; i++) {
		if (strcmp(name, ldv.gamma_sets[i].name) == 0) {
			if (ldv.gamma_sets[i].addr19 && !is_gamma19)
				gamma_set = (gamma_set_pointer)
				    ldv.gamma_sets[i].addr22;
			else if (ldv.gamma_sets[i].addr22)
				gamma_set = (gamma_set_pointer)
				    ldv.gamma_sets[i].addr19;
		}
	}

	gamma_level = (gamma_level_pointer) gamma_set[level];

	return gamma_level;

}

static struct brightness_level *get_bl(const char *name, unsigned int level,
				       bool is_gamma19)
{
	int i;
	int j;
	struct brightness_level *bl;
	gamma_level_pointer gamma_level;
	GAMMA_NAMES;

	printk(NAME ": read brightness level %d for %s (%s)\n", level, name,
	       gamma_name[(int)is_gamma19]);

	bl = kzalloc(sizeof(*bl), GFP_KERNEL);
	gamma_level = get_gamma_level(name, level, is_gamma19);

	for (i = j = 0; i < GAMMA_POINTS; i++) {
		if (i == 5)
			j = j + 2;
		bl->r[i] = gamma_level[j + 1];
		bl->g[i] = gamma_level[j + 15];
		bl->b[i] = gamma_level[j + 29];
		j = j + 2;
	}

	return bl;
}

static void set_bl(const char *name, unsigned int level,
		   struct brightness_level *bl, bool is_gamma19)
{
	int i;
	int j;
	gamma_level_pointer gamma_level;
	GAMMA_NAMES;

	printk(NAME ": write brightness level %d for %s (%s)\n", level, name,
	       gamma_name[(int)is_gamma19]);

	gamma_level = get_gamma_level(name, level, is_gamma19);

	for (i = j = 0; i < GAMMA_POINTS; i++) {
		if (i == 5)
			j = j + 2;
		gamma_level[j + 1] = bl->r[i];
		gamma_level[j + 15] = bl->g[i];
		gamma_level[j + 29] = bl->b[i];
		j = j + 2;
	}

}

static void make_gamma_sets_backup(void)
{
	int bl_level;
	int i;

	printk(NAME ": make gamma sets backup\n");

	ldv.backup = kzalloc(sizeof(*ldv.backup), GFP_KERNEL);

	for (i = 0; i < MAX_GAMMA_SETS; i++)

		for (bl_level = 0; bl_level < MAX_GAMMA_LEVEL; bl_level++) {

			if (ldv.gamma_sets[i].addr22)
				ldv.backup->gamma22[i][bl_level] =
				    get_bl(ldv.gamma_sets[i].name, bl_level,
					   false);

			if (ldv.gamma_sets[i].addr19)
				ldv.backup->gamma19[i][bl_level] =
				    get_bl(ldv.gamma_sets[i].name, bl_level,
					   true);
		}

}

static void restore_gamma_sets_backup(const char *name, bool is_gamma19)
{
	int i;
	int bl_level;
	struct brightness_level *bl;

	for (i = 0; i < MAX_GAMMA_SETS; i++) {

		if (((is_gamma19 && ldv.gamma_sets[i].addr19)
		     || ldv.gamma_sets[i].addr22)
		    && strcmp(name, ldv.gamma_sets[i].name) == 0) {

			for (bl_level = 0; bl_level < MAX_GAMMA_LEVEL;
			     bl_level++) {
				if (is_gamma19)
					bl = ldv.backup->gamma19[i][bl_level];
				else
					bl = ldv.backup->gamma22[i][bl_level];

				if (bl)
					set_bl(name, bl_level, bl, is_gamma19);
			}
		}
	}
}

static void copy_gamma_22_to_19(void)
{
	int i;
	int bl_level;
	struct brightness_level *bl;

	for (i = 0; i < MAX_GAMMA_SETS; i++) {

		if (ldv.gamma_sets[i].addr19 && ldv.gamma_sets[i].addr22) {

			for (bl_level = 0; bl_level < MAX_GAMMA_LEVEL;
			     bl_level++) {

				bl = get_bl(ldv.gamma_sets[i].name, bl_level,
					    false);
				set_bl(ldv.gamma_sets[i].name, bl_level, bl,
				       true);
			}
		}
	}
}

static ssize_t copy_gamma_22_to_19_show(struct class *class,
					struct class_attribute *attr, char *buf)
{
	if (ldv.is_19_copy_of_22)
		return sprintf(buf, "gamma set 1.9 is a copy of 2.2\n");
	else
		return sprintf(buf, "gamma set 1.9 is not a copy of 2.2\n");
}

static ssize_t copy_gamma_22_to_19_store(struct class *class,
					 struct class_attribute *attr,
					 const char *buf, size_t count)
{
	int state;
	int i;

	if ((sscanf(buf, "%d", &state) == 1) && state) {
		copy_gamma_22_to_19();
	} else {
		for (i = 0; i < MAX_GAMMA_SETS; i++)
			if (ldv.gamma_sets[i].addr22
			    && ldv.gamma_sets[i].addr19)
				restore_gamma_sets_backup(ldv.gamma_sets[i].
							  name, true);

	}

	ldv.is_19_copy_of_22 = (bool) state;

	return count;
}

static CLASS_ATTR(copy_gamma_22_to_19, 0666, copy_gamma_22_to_19_show,
		  copy_gamma_22_to_19_store);

static ssize_t version_show(struct class *class, struct class_attribute *attr,
			    char *buf)
{
	return sprintf(buf, "%d\n", VERSION);
}

static CLASS_ATTR(version, 0444, version_show, NULL);

static ssize_t bl_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	unsigned int bl_level;
	struct brightness_level *bl;
	ssize_t size;

	sscanf(attr->attr.name, "bl_%d", &bl_level);
	bl = get_bl(dev_name(dev), bl_level, false);

	size = sprintf(buf,
		       "%u,%u,%u,%u,%u,%u\n"
		       "%u,%u,%u,%u,%u,%u\n"
		       "%u,%u,%u,%u,%u,%u\n",
		       bl->r[0], bl->r[1], bl->r[2], bl->r[3], bl->r[4],
		       bl->r[5], bl->g[0], bl->g[1], bl->g[2], bl->g[3],
		       bl->g[4], bl->g[5], bl->b[0], bl->b[1], bl->b[2],
		       bl->b[3], bl->r[4], bl->b[5]);

	kfree(bl);

	return (size);
}

static ssize_t bl_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	int bl_level;
	struct brightness_level *bl;

	bl = kzalloc(sizeof(*bl), GFP_KERNEL);

	sscanf(attr->attr.name, "bl_%d", &bl_level);

	// strict parsing only
	if (sscanf(buf,
		   "%hu,%hu,%hu,%hu,%hu,%hu\n"
		   "%hu,%hu,%hu,%hu,%hu,%hu\n"
		   "%hu,%hu,%hu,%hu,%hu,%hu\n",
		   &bl->r[0], &bl->r[1], &bl->r[2], &bl->r[3], &bl->r[4],
		   &bl->r[5], &bl->g[0], &bl->g[1], &bl->g[2], &bl->g[3],
		   &bl->g[4], &bl->g[5], &bl->b[0], &bl->b[1], &bl->b[2],
		   &bl->b[3], &bl->b[4], &bl->b[5]) == GAMMA_POINTS * 3) {

		printk(NAME ": store brightness level %d for %s\n", bl_level,
		       dev_name(dev));

		set_bl(dev_name(dev), bl_level, bl, false);
	} else
		printk(NAME ": ignoring malformed custom brightness level"
		       " %d for %s\n", bl_level, dev_name(dev));

	kfree(bl);

	return count;
}

static ssize_t restore_gamma_sets_backup_store(struct class *dev,
					       struct class_attribute *attr,
					       const char *buf, size_t count)
{
	int i;
	for (i = 0; i < MAX_GAMMA_SETS; i++)
		if (ldv.gamma_sets[i].dev22)
			restore_gamma_sets_backup(dev_name
						  (ldv.gamma_sets[i].dev22),
						  false);

	return count;
}

static CLASS_ATTR(restore_gamma_sets_backup, 0222, NULL,
		  restore_gamma_sets_backup_store);

static struct device_attribute *create_dev_attr(char name[])
{
	struct device_attribute *dev_attr;
	struct attribute *attr;

	dev_attr = kzalloc(sizeof(*dev_attr), GFP_KERNEL);
	if (!dev_attr)
		return NULL;

	attr = kzalloc(sizeof(*attr), GFP_KERNEL);
	if (!attr)
		return NULL;

	attr->name = name;
	attr->mode = 0666;

	dev_attr->attr = *attr;
	dev_attr->show = bl_show;
	dev_attr->store = bl_store;

	return dev_attr;
}

static void create_sysfs_devices(void)
{
	int i;
	int j;
	int dev_attrs_number = 0;
	int ret;

	printk(NAME ": create sysfs interface\n");

	for (i = 0; i < MAX_GAMMA_SETS; i++) {

		if (ldv.gamma_sets[i].addr22) {
			printk(NAME ": create %s\n", ldv.gamma_sets[i].name);
			ldv.gamma_sets[i].dev22 =
			    device_create(ldv.ld9040_voodoo_class, NULL, 0,
					  NULL, ldv.gamma_sets[i].name);

		}
	}

	for (i = 0; i < MAX_GAMMA_LEVEL; i++) {
		char *level_name = kzalloc(8, GFP_KERNEL);
		sprintf(level_name, "bl_%d", i);

		for (j = 0; j < MAX_GAMMA_SETS; j++) {
			if (ldv.gamma_sets[j].dev22) {
				ldv.dev_attrs[dev_attrs_number] =
				    create_dev_attr(level_name);
				ret =
				    device_create_file(ldv.gamma_sets[j].dev22,
						       ldv.dev_attrs
						       [dev_attrs_number]);
				dev_attrs_number++;
			}
		}
	}

}

static void delete_sysfs_devices(void)
{
	int i;
	int j;
	int dev_attrs_number = 0;

	printk(NAME ": delete sysfs interface\n");

	for (i = 0; i < MAX_GAMMA_LEVEL; i++) {

		for (j = 0; j < MAX_GAMMA_SETS; j++) {
			if (ldv.gamma_sets[j].dev22) {

				// delete device attributes
				device_remove_file(ldv.gamma_sets[j].dev22,
						   ldv.dev_attrs
						   [dev_attrs_number]);
				kfree(ldv.dev_attrs[dev_attrs_number]);
				dev_attrs_number++;

				// delete device (gamma set)
				device_unregister(ldv.gamma_sets[j].dev22);
				ldv.gamma_sets[j].dev22 = NULL;
			}
		}

	}

}

static ssize_t reload_sysfs_interface(struct class *class,
				      struct class_attribute *attr,
				      const char *buf, size_t count)
{
	int i;

	printk(NAME ": re-creating sysfs interface...\n");

	for (i = 0; i < MAX_GAMMA_SETS; i++)
		if (ldv.gamma_sets[i].dev22)
			restore_gamma_sets_backup(dev_name
						  (ldv.gamma_sets[i].dev22),
						  false);

	delete_sysfs_devices();

	parse_config();
	create_sysfs_devices();
	make_gamma_sets_backup();

	return count;
}

static CLASS_ATTR(reload_sysfs_interface, 0200, NULL, reload_sysfs_interface);

static int __init ld9040_voodoo_init(void)
{
	int ret;

	printk(NAME ": init\n");

	ldv.ld9040_voodoo_class = class_create(THIS_MODULE, "ld9040_voodoo");
	if (!ldv.ld9040_voodoo_class)
		return -ENOMEM;

	ret = class_create_file(ldv.ld9040_voodoo_class, &class_attr_version);
	ret = class_create_file(ldv.ld9040_voodoo_class,
				&class_attr_reload_sysfs_interface);
	ret = class_create_file(ldv.ld9040_voodoo_class,
				&class_attr_restore_gamma_sets_backup);
	ret = class_create_file(ldv.ld9040_voodoo_class,
				&class_attr_copy_gamma_22_to_19);

	parse_config();
	create_sysfs_devices();

	make_gamma_sets_backup();

	return 0;

//error:
//      class_destroy(ldv.ld9040_voodoo_class);
//      return ret;
}

void __exit ld9040_voodoo_exit(void)
{
	int i;
	printk(NAME ": init\n");

	for (i = 0; i < MAX_GAMMA_SETS; i++)
		if (ldv.gamma_sets[i].dev22)
			restore_gamma_sets_backup(dev_name
						  (ldv.gamma_sets[i].dev22),
						  false);

	kfree(ldv.backup);

	delete_sysfs_devices();

	class_remove_file(ldv.ld9040_voodoo_class, &class_attr_version);
	class_remove_file(ldv.ld9040_voodoo_class,
			  &class_attr_reload_sysfs_interface);
	class_remove_file(ldv.ld9040_voodoo_class,
			  &class_attr_restore_gamma_sets_backup);
	class_destroy(ldv.ld9040_voodoo_class);
}

MODULE_DESCRIPTION(NAME);
MODULE_AUTHOR("supercurio");
MODULE_LICENSE("GPL");

module_init(ld9040_voodoo_init);
module_exit(ld9040_voodoo_exit);
