#define NAME 				"Voodoo Super AMOLED Plus driver"
#define VERSION								2
#define MAX_GAMMA_LEVEL						       25
#define GAMMA_POINTS							6
#define MAX_GAMMA_SETS							3
#define MAX_NAME_LENGTH						       64
#define GAMMA_NAMES 		    char *gamma_name[] = { "2.2", "1.9" }

typedef unsigned long (*gamma_set_pointer);
typedef unsigned short (*gamma_level_pointer);

struct gamma_set {
	struct device *dev22;
	long unsigned int addr22;
	long unsigned int addr19;
	char *name;
};

struct brightness_level {
	unsigned short r[6];
	unsigned short g[6];
	unsigned short b[6];
};

struct values_backup {
	struct brightness_level *gamma22[MAX_GAMMA_SETS][MAX_GAMMA_LEVEL];
	struct brightness_level *gamma19[MAX_GAMMA_SETS][MAX_GAMMA_LEVEL];
};

struct ld9040_voodoo {
	struct kobject kobj;
	struct class *ld9040_voodoo_class;

	struct device_attribute *dev_attrs[MAX_GAMMA_SETS * 2 *
					   MAX_GAMMA_LEVEL];

	struct gamma_set gamma_sets[MAX_GAMMA_SETS];
	struct values_backup *backup;

	bool is_19_copy_of_22;
};
