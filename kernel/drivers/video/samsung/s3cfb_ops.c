/* linux/drivers/video/samsung/s3cfb-ops.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Middle layer file for Samsung Display Controller (FIMD) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/poll.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#if defined(CONFIG_S5P_MEM_CMA)
#include <linux/cma.h>
#elif defined(CONFIG_S5P_MEM_BOOTMEM)
#include <plat/media.h>
#include <mach/media.h>
#endif

#if MALI_USE_UNIFIED_MEMORY_PROVIDER
#include "ump_kernel_interface_ref_drv.h"
#define UMP_HANDLE_DD_INVALID ((void *)-1)
#endif

#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#include <linux/earlysuspend.h>
#include <linux/suspend.h>
#endif
#include <linux/serial_core.h>
#include <plat/regs-serial.h>
#include <plat/s5pv310.h>
#include <mach/sec_debug.h>
#include "s3cfb.h"
#include "logo_rgb24_homura.h"

struct s3c_platform_fb *to_fb_plat(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	return (struct s3c_platform_fb *)pdev->dev.platform_data;
}

#ifndef CONFIG_FRAMEBUFFER_CONSOLE
#define LPDDR0_BASE_ADDR		0x40000000
#define LPDDR0_SIZE				0x10000000	/* 256MB */
#define LPDDR1_BASE_ADDR		0x50000000
#define LPDDR1_SIZE				0x10000000	/* 256MB */
#define SDRAM_SIZE				(LPDDR0_SIZE + LPDDR1_SIZE)
#define BOOT_FB_BASE_ADDR		(LPDDR1_BASE_ADDR   + 0x0EC00000)	/* 0x5EC00000 from Bootloader */

static unsigned int bootloaderfb;
module_param_named(bootloaderfb, bootloaderfb, uint, 0444);
MODULE_PARM_DESC(bootloaderfb, "Address of booting logo image in Bootloader");

int s3cfb_draw_logo(struct fb_info *fb)
{
#ifdef CONFIG_FB_S3C_SPLASH_SCREEN
#if 0
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct fb_var_screeninfo *var = &fb->var;
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	memcpy(fbdev->fb[pdata->default_win]->screen_base,
	       LOGO_RGB24, fix->line_length * var->yres);
#else

#ifdef RGB_BOOTSCREEN
	u32 height = var->yres / 3;
	u32 line = fix->line_length;
	u32 i, j;

	for (i = 0; i < height; i++) {
		for (j = 0; j < var->xres; j++) {
			memset(fb->screen_base + i * line + j * 4 + 0, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 1, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 2, 0xff, 1);
			memset(fb->screen_base + i * line + j * 4 + 3, 0x00, 1);
		}
	}

	for (i = height; i < height * 2; i++) {
		for (j = 0; j < var->xres; j++) {
			memset(fb->screen_base + i * line + j * 4 + 0, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 1, 0xff, 1);
			memset(fb->screen_base + i * line + j * 4 + 2, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 3, 0x00, 1);
		}
	}

	for (i = height * 2; i < height * 3; i++) {
		for (j = 0; j < var->xres; j++) {
			memset(fb->screen_base + i * line + j * 4 + 0, 0xff, 1);
			memset(fb->screen_base + i * line + j * 4 + 1, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 2, 0x00, 1);
			memset(fb->screen_base + i * line + j * 4 + 3, 0x00, 1);
		}
	}

#else // #ifdef RGB_BOOTSCREEN

	if (bootloaderfb) {
		printk("Bootloader sent 'bootloaderfb' to Kernel Successfully : %d", bootloaderfb);
	}
	else {
		bootloaderfb = BOOT_FB_BASE_ADDR;
		printk("Fail to get 'bootloaderfb' from Bootloader. so we must set  this value as %d", bootloaderfb);
	}

	{
		u8 *logo_virt_buf;
		logo_virt_buf = ioremap_nocache(bootloaderfb, fb->var.yres * fb->fix.line_length);
		//memcpy(fb->screen_base, logo_virt_buf, fb->var.yres * fb->fix.line_length);
		memcpy(fb->screen_base, LOGO_RGB24, fb->var.yres * fb->fix.line_length);
		iounmap(logo_virt_buf);
	}

#endif // #ifdef RGB_BOOTSCREEN	
#endif // 0
#endif // #ifdef CONFIG_FB_S3C_SPLASH_SCREEN

	return 0;
}
#else
int fb_is_primary_device(struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);

	dev_dbg(fbdev->dev, "[fb%d] checking for primary device\n", win->id);

	if (win->id == pdata->default_win)
		return 1;
	else
		return 0;
}
#endif

int s3cfb_enable_window(struct s3cfb_global *fbdev, int id)
{
	struct s3cfb_window *win = fbdev->fb[id]->par;

	if (!win->enabled)
		atomic_inc(&fbdev->enabled_win);

	if (s3cfb_window_on(fbdev, id)) {
		win->enabled = 0;
		return -EFAULT;
	} else {
		win->enabled = 1;
		return 0;
	}
}

int s3cfb_disable_window(struct s3cfb_global *fbdev, int id)
{
	struct s3cfb_window *win = fbdev->fb[id]->par;

	if (win->enabled)
		atomic_dec(&fbdev->enabled_win);

	if (s3cfb_window_off(fbdev, id)) {
		win->enabled = 1;
		return -EFAULT;
	} else {
		win->enabled = 0;
		return 0;
	}
}

int s3cfb_update_power_state(struct s3cfb_global *fbdev, int id, int state)
{
	struct s3cfb_window *win = fbdev->fb[id]->par;
	win->power_state = state;

	return 0;
}

int s3cfb_init_global(struct s3cfb_global *fbdev)
{
	fbdev->output = OUTPUT_RGB;
	fbdev->rgb_mode = MODE_RGB_P;
#ifndef CONFIG_FB_S3C_MDNIE
	fbdev->wq_count = 0;
	init_waitqueue_head(&fbdev->wq);
#endif

	fbdev->wq_count = 0;
	init_waitqueue_head(&fbdev->wq);
	mutex_init(&fbdev->lock);

	s3cfb_set_output(fbdev);
	s3cfb_set_display_mode(fbdev);
	s3cfb_set_polarity(fbdev);
	s3cfb_set_timing(fbdev);
	s3cfb_set_lcd_size(fbdev);

	return 0;
}

int s3cfb_unmap_video_memory(struct s3cfb_global *fbdev, struct fb_info *fb)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;
#ifdef CONFIG_VCM
	struct fb_var_screeninfo *var = &fb->var;
	int frame_num = var->yres_virtual / var->yres;
	int i;
#endif

	if (fix->smem_start) {

#ifdef CONFIG_VCM
		vcm_unmap(win->s5p_vcm_res);
		cma_free(fix->smem_start);
		for (i = 0; i < frame_num; i++)
			kfree(win->s3cfb_vcm[i].dev_vcm_res);
#endif

		fix->smem_start = 0;
		fix->smem_len = 0;
		dev_info(fbdev->dev, "[fb%d] video memory released\n", win->id);
	}
	return 0;
}

#ifdef CONFIG_VCM
static const struct s5p_vcm_driver s3cfb_vcm_driver = {
	.tlb_invalidator = NULL,
	.pgd_base_specifier = NULL,
	.phys_alloc = NULL,
	.phys_free = NULL,
};

#if MALI_USE_UNIFIED_MEMORY_PROVIDER
int s3cfb_ump_wrapper(struct fb_fix_screeninfo *fix, int arg, int id,
			struct s3cfb_window *win)
{
	ump_dd_physical_block ump_memory_description;
	unsigned int buffer_size;
	buffer_size = fix->smem_len / CONFIG_FB_S3C_NR_BUFFERS;

	ump_memory_description.addr = fix->smem_start + (buffer_size * id);
	ump_memory_description.size = buffer_size;

	win->ump_wrapped_buffer[id] =
		ump_dd_handle_create_from_phys_blocks
		(&ump_memory_description, 1);

	if (ump_dd_vcm_attribute_set(win->ump_wrapped_buffer[id], arg))
		return -ENOMEM;

	return 0;
}
#endif
#endif

int s3cfb_map_video_memory(struct s3cfb_global *fbdev, struct fb_info *fb)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;

#ifdef CONFIG_VCM
	struct fb_var_screeninfo *var = &fb->var;
	struct cma_info mem_info;
	unsigned int reserved_size;
	int err;
	struct vcm_phys *phys = NULL;
	unsigned int device_virt_start = 0;
	int frame_num = var->yres_virtual / var->yres;
	int frame_size = fix->smem_len / frame_num;
	struct  vcm_res *fb_dev_vcm_res[frame_num];

	enum vcm_dev_id id;

	struct ump_vcm ump_vcm;
	unsigned int arg = 0;

	int i;
	ump_dd_physical_block ump_memory_description;

	if (win->owner == DMA_MEM_OTHER)
		return 0;

	phys = kmalloc(sizeof(*phys) + sizeof(*phys->parts), GFP_KERNEL);
	memset(phys, 0, sizeof(*phys) + sizeof(*phys->parts));

	if (win->id < 5)
		id = VCM_DEV_FIMD0;
	else
		id = VCM_DEV_FIMD1;

	err = cma_info(&mem_info, fbdev->dev, 0);
	if (ERR_PTR(err))
		return -ENOMEM;
	reserved_size = fix->smem_len;
	fix->smem_start = (dma_addr_t)cma_alloc
		(fbdev->dev, "fimd", (size_t)reserved_size, 0);
	fb->screen_base = cma_get_virt(fix->smem_start, reserved_size, 1);

	phys->count = 1;
	phys->size = fix->smem_len;
	phys->free = NULL;
	phys->parts[0].size = fix->smem_len;
	phys->parts[0].start = fix->smem_start;

	win->s5p_vcm_res = vcm_map(fbdev->s5p_vcm, phys, 0);
	device_virt_start = win->s5p_vcm_res->start;

	for (i = 0; i < frame_num; i++) {
		fb_dev_vcm_res[i] = kzalloc(sizeof(struct vcm_res), GFP_KERNEL);
		win->s3cfb_vcm[i].dev_vcm_res = fb_dev_vcm_res[i];

		win->s3cfb_vcm[i].dev_vcm_res->start = device_virt_start
							+ frame_size * i;
		win->s3cfb_vcm[i].dev_vcm_res->bound_size = frame_size;
		win->s3cfb_vcm[i].dev_vcm_res->res_size = frame_size;
		win->s3cfb_vcm[i].dev_vcm = fbdev->s5p_vcm;
		win->s3cfb_vcm[i].dev_vcm_res->vcm = fbdev->s5p_vcm;
		if (IS_ERR(win->s3cfb_vcm[i].dev_vcm_res))
			return -ENOMEM;
	}

	memset(fb->screen_base, 0, (fix->smem_len / frame_num));
	win->owner = DMA_MEM_FIMD;
#else
	if (win->owner == DMA_MEM_OTHER)
		return 0;

	fb->screen_base = dma_alloc_writecombine(fbdev->dev,
						 PAGE_ALIGN(fix->smem_len),
						 (unsigned int *)
						 &fix->smem_start, GFP_KERNEL);

	if (!fb->screen_base)
		return -ENOMEM;
	else
		dev_info(fbdev->dev, "[fb%d] dma: 0x%08x, cpu: 0x%08x, "
			 "size: 0x%08x\n", win->id,
			 (unsigned int)fix->smem_start,
			 (unsigned int)fb->screen_base, fix->smem_len);

	memset(fb->screen_base, 0, fix->smem_len);
	win->owner = DMA_MEM_FIMD;
#endif

#if MALI_USE_UNIFIED_MEMORY_PROVIDER
#ifdef CONFIG_VCM
#ifdef CONFIG_UMP_VCM_ALLOC
	for (i = 0; i < frame_num; i++) {
		ump_vcm.vcm = win->s3cfb_vcm[i].dev_vcm;
		ump_vcm.vcm_res = win->s3cfb_vcm[i].dev_vcm_res;
		ump_vcm.dev_id = id;
		arg = (unsigned int)&ump_vcm;

		ump_memory_description.addr = fix->smem_start + ((fix->smem_len / frame_num) * i);
		ump_memory_description.size = fix->smem_len / frame_num;

		win->ump_wrapped_buffer[i] =
			ump_dd_handle_create_from_phys_blocks
			(&ump_memory_description, 1);

		if (ump_dd_vcm_attribute_set(win->ump_wrapped_buffer[i], arg))
			return -ENOMEM;

	}
#else
	if (s3cfb_ump_wrapper(fix, arg, 0, win)) {
		dev_info(fbdev->dev, "[fb%d] : Wrapped UMP memory : %x\n"
				, win->id, (unsigned int)ump_wrapped_buffer);
		s3cfb_unmap_video_memory(fbdev, fb);
		return -ENOMEM;
	}
#endif
#endif
#endif

	return 0;
}

int s3cfb_map_default_video_memory(struct s3cfb_global *fbdev,
					struct fb_info *fb, int fimd_id)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;

#ifdef CONFIG_VCM
	struct cma_info mem_info;
	unsigned int reserved_size;
	int err;
	struct vcm_phys *phys = NULL;
	ump_dd_physical_block ump_memory_description;
	unsigned int device_virt_start = 0;
	int frame_size = fix->smem_len / CONFIG_FB_S3C_NR_BUFFERS;
	struct  vcm_res *fb_dev_vcm_res[CONFIG_FB_S3C_NR_BUFFERS];

	enum vcm_dev_id id;
#else
#ifdef CONFIG_S5P_MEM_CMA
	struct cma_info mem_info;
	unsigned int reserved_size;
	int err;
#endif
#endif

#ifdef MALI_USE_UNIFIED_MEMORY_PROVIDER
#ifdef CONFIG_VCM
	int i;
	unsigned int arg = 0;
#ifdef CONFIG_UMP_VCM_ALLOC
	struct ump_vcm ump_vcm;
#endif
	unsigned int arg = 0;
#endif
#endif

	if (win->owner == DMA_MEM_OTHER)
		return 0;

#ifdef CONFIG_VCM
	phys = kmalloc(sizeof(*phys) + sizeof(*phys->parts), GFP_KERNEL);
	memset(phys, 0, sizeof(*phys) + sizeof(*phys->parts));

	if (fimd_id == 0)
		id = VCM_DEV_FIMD0;
	else
		id = VCM_DEV_FIMD1;

	err = cma_info(&mem_info, fbdev->dev, 0);
	if (ERR_PTR(err))
		return -ENOMEM;
	reserved_size = fix->smem_len;
	fix->smem_start = (dma_addr_t)cma_alloc
		(fbdev->dev, "fimd", (size_t)reserved_size, 0);
	fb->screen_base = cma_get_virt(fix->smem_start, reserved_size, 1);

	fbdev->s5p_vcm = vcm_create_unified((SZ_64M), id, &s3cfb_vcm_driver);
	if (IS_ERR(fbdev->s5p_vcm))
		return PTR_ERR(fbdev->s5p_vcm);
	if (vcm_activate(fbdev->s5p_vcm))
		dev_info(fbdev->dev, "[fb%d] : VCM activated", win->id);

	phys->count = 1;
	phys->size = fix->smem_len;
	phys->free = NULL;
	phys->parts[0].size = fix->smem_len;
	phys->parts[0].start = fix->smem_start;

	win->s5p_vcm_res = vcm_map(fbdev->s5p_vcm, phys, 0);
	device_virt_start = win->s5p_vcm_res->start;

	for (i = 0; i < CONFIG_FB_S3C_NR_BUFFERS; i++) {
		fb_dev_vcm_res[i] = kzalloc(sizeof(struct vcm_res), GFP_KERNEL);
		win->s3cfb_vcm[i].dev_vcm_res = fb_dev_vcm_res[i];

		win->s3cfb_vcm[i].dev_vcm_res->start = device_virt_start
							+ frame_size * i;
		win->s3cfb_vcm[i].dev_vcm_res->bound_size = frame_size;
		win->s3cfb_vcm[i].dev_vcm_res->res_size = frame_size;
		win->s3cfb_vcm[i].dev_vcm = fbdev->s5p_vcm;
		win->s3cfb_vcm[i].dev_vcm_res->vcm = fbdev->s5p_vcm;
		if (IS_ERR(win->s3cfb_vcm[i].dev_vcm_res))
			return -ENOMEM;
	}
#else
#ifdef CONFIG_S5P_MEM_CMA
	err = cma_info(&mem_info, fbdev->dev, 0);
	if (ERR_PTR(err))
		return -ENOMEM;
	reserved_size = mem_info.total_size;
	fix->smem_start = (dma_addr_t)cma_alloc
		(fbdev->dev, "fimd", (size_t)reserved_size, 0);
	fb->screen_base = cma_get_virt(fix->smem_start, reserved_size, 1);
#elif defined(CONFIG_S5P_MEM_BOOTMEM)
	fix->smem_start = s5p_get_media_memory_bank(S5P_MDEV_FIMD, 1);
	fix->smem_len = s5p_get_media_memsize_bank(S5P_MDEV_FIMD, 1);
	fb->screen_base = ioremap_wc(fix->smem_start, fix->smem_len);
#endif
#endif

	memset(fb->screen_base, 0, fix->smem_len);
	win->owner = DMA_MEM_FIMD;

#if MALI_USE_UNIFIED_MEMORY_PROVIDER
#ifdef CONFIG_VCM
#ifdef CONFIG_UMP_VCM_ALLOC
	for (i = 0; i < CONFIG_FB_S3C_NR_BUFFERS; i++) {
		ump_vcm.vcm = win->s3cfb_vcm[i].dev_vcm;
		ump_vcm.vcm_res = win->s3cfb_vcm[i].dev_vcm_res;
		ump_vcm.dev_id = id;
		arg = (unsigned int)&ump_vcm;
		ump_memory_description.addr = fix->smem_start + ((fix->smem_len / CONFIG_FB_S3C_NR_BUFFERS) * i);
		ump_memory_description.size = fix->smem_len / CONFIG_FB_S3C_NR_BUFFERS;

		win->ump_wrapped_buffer[i] =
			ump_dd_handle_create_from_phys_blocks
			(&ump_memory_description, 1);

		if (ump_dd_vcm_attribute_set(win->ump_wrapped_buffer[i], arg))
			return -ENOMEM;
	}
#else
	if (s3cfb_ump_wrapper(fix, arg, 0, win)) {
		dev_info(fbdev->dev, "[fb%d] : Wrapped UMP memory : %x\n"
				, win->id, (unsigned int)ump_wrapped_buffer);
		s3cfb_unmap_video_memory(fbdev, fb);
		return -ENOMEM;
	}
#endif
#endif
#endif

	return 0;
}

int s3cfb_unmap_default_video_memory(struct s3cfb_global *fbdev,
					struct fb_info *fb)
{
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;
#ifdef CONFIG_VCM
	int i;
#endif

	if (fix->smem_start) {

#ifdef CONFIG_VCM
		vcm_unmap(win->s5p_vcm_res);
		cma_free(fix->smem_start);
		for (i = 0; i < CONFIG_FB_S3C_NR_BUFFERS; i++)
			kfree(win->s3cfb_vcm[i].dev_vcm_res);
#else
		iounmap(fb->screen_base);
#endif
		fix->smem_start = 0;
		fix->smem_len = 0;
		dev_info(fbdev->dev, "[fb%d] video memory released\n", win->id);
	}

	return 0;
}

int s3cfb_set_bitfield(struct fb_var_screeninfo *var)
{
	switch (var->bits_per_pixel) {
	case 16:
		if (var->transp.length == 1) {
			var->red.offset = 10;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 5;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 15;
		} else if (var->transp.length == 4) {
			var->red.offset = 8;
			var->red.length = 4;
			var->green.offset = 4;
			var->green.length = 4;
			var->blue.offset = 0;
			var->blue.length = 4;
			var->transp.offset = 12;
		} else {
			var->red.offset = 11;
			var->red.length = 5;
			var->green.offset = 5;
			var->green.length = 6;
			var->blue.offset = 0;
			var->blue.length = 5;
			var->transp.offset = 0;
		}
		break;

	case 24:
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;

	case 32:
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 24;
		var->transp.length = 8; //added for LCD RGB32
		break;
	}

	return 0;
}

int s3cfb_set_alpha_info(struct fb_var_screeninfo *var,
				struct s3cfb_window *win)
{
	if (var->transp.length > 0)
		win->alpha.mode = PIXEL_BLENDING;
	else {
		win->alpha.mode = PLANE_BLENDING;
		win->alpha.channel = 0;
		win->alpha.value = S3CFB_AVALUE(0xf, 0xf, 0xf);
	}

	return 0;
}

int s3cfb_check_var_window(struct s3cfb_global *fbdev,
			struct fb_var_screeninfo *var, struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	struct s3cfb_window *win = fb->par;
	struct s3cfb_lcd *lcd = fbdev->lcd;

	dev_dbg(fbdev->dev, "[fb%d] check_var\n", win->id);

	if (var->bits_per_pixel != 16 && var->bits_per_pixel != 24 &&
	    var->bits_per_pixel != 32) {
		dev_err(fbdev->dev, "invalid bits per pixel\n");
		return -EINVAL;
	}

	if (var->xres > lcd->width)
		var->xres = lcd->width;

	if (var->yres > lcd->height)
		var->yres = lcd->height;

	if (var->xres_virtual < var->xres)
		var->xres_virtual = var->xres;

#if defined(CONFIG_FB_S3C_VIRTUAL)
	if (var->yres_virtual < var->yres)
		var->yres_virtual = var->yres * CONFIG_FB_S3C_NR_BUFFERS;
#else
	if (var->yres_virtual > var->yres * CONFIG_FB_S3C_NR_BUFFERS)
		var->yres_virtual = var->yres * CONFIG_FB_S3C_NR_BUFFERS;
#endif

	if (var->xoffset > (var->xres_virtual - var->xres))
		var->xoffset = var->xres_virtual - var->xres;

	if (var->yoffset + var->yres > var->yres_virtual)
		var->yoffset = var->yres_virtual - var->yres;

	if (win->x + var->xres > lcd->width)
		win->x = lcd->width - var->xres;

	if (win->y + var->yres > lcd->height)
		win->y = lcd->height - var->yres;

	if (var->pixclock != fbdev->fb[pdata->default_win]->var.pixclock) {
		dev_info(fbdev->dev, "pixclk is changed from %d Hz to %d Hz\n",
			fbdev->fb[pdata->default_win]->var.pixclock, var->pixclock);
	}

	s3cfb_set_bitfield(var);
	s3cfb_set_alpha_info(var, win);

	return 0;
}

int s3cfb_check_var(struct fb_var_screeninfo *var, struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);

	s3cfb_check_var_window(fbdev, var, fb);

	return 0;
}

void s3cfb_set_win_params(struct s3cfb_global *fbdev, int id)
{
	s3cfb_set_window_control(fbdev, id);
	s3cfb_set_window_position(fbdev, id);
	s3cfb_set_window_size(fbdev, id);
	s3cfb_set_buffer_address(fbdev, id);
	s3cfb_set_buffer_size(fbdev, id);

	if (id > 0) {
		s3cfb_set_alpha_blending(fbdev, id);
		s3cfb_set_chroma_key(fbdev, id);
	}
}

int s3cfb_set_par_window(struct s3cfb_global *fbdev, struct fb_info *fb)
{
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	struct s3cfb_window *win = fb->par;

	dev_dbg(fbdev->dev, "[fb%d] set_par\n", win->id);

#if 0	/* In android scenario, this routine is not required */
	if ((win->id != pdata->default_win) && fb->fix.smem_start)
		s3cfb_unmap_video_memory(fbdev, fb);
#endif

	/* modify the fix info */
	if (win->id != pdata->default_win) {
		fb->fix.line_length = fb->var.xres_virtual *
						fb->var.bits_per_pixel / 8;
		fb->fix.smem_len = fb->fix.line_length * fb->var.yres_virtual;
	}

#if 0	/* In android scenario, this routine is not required */
	if (win->id != pdata->default_win)
		s3cfb_map_video_memory(fbdev, fb);
#endif

	s3cfb_set_win_params(fbdev, win->id);

	return 0;
}

int s3cfb_set_par(struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);

#ifdef CONFIG_S5PV310_DEV_PD
	if (fbdev->system_state == POWER_OFF) {
		dev_err(fbdev->dev, "system_state is POWER_OFF\n");
		return 0;
	}
#endif

	s3cfb_set_par_window(fbdev, fb);

	return 0;
}

int s3cfb_init_fbinfo(struct s3cfb_global *fbdev, int id)
{
	struct fb_info *fb = fbdev->fb[id];
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct fb_var_screeninfo *var = &fb->var;
	struct s3cfb_window *win = fb->par;
	struct s3cfb_alpha *alpha = &win->alpha;
	struct s3cfb_lcd *lcd = fbdev->lcd;
	struct s3cfb_lcd_timing *timing = &lcd->timing;

	memset(win, 0, sizeof(struct s3cfb_window));
	platform_set_drvdata(to_platform_device(fbdev->dev), fb);
	strcpy(fix->id, S3CFB_NAME);

	/* fimd specific */
	win->id = id;
	win->path = DATA_PATH_DMA;
	win->dma_burst = 16;
	s3cfb_update_power_state(fbdev, win->id, FB_BLANK_POWERDOWN);
	alpha->mode = PLANE_BLENDING;

	/* fbinfo */
	fb->fbops = &s3cfb_ops;
	fb->flags = FBINFO_FLAG_DEFAULT;
	fb->pseudo_palette = &win->pseudo_pal;
#if (CONFIG_FB_S3C_NR_BUFFERS != 1)
	fix->xpanstep = 2;
	fix->ypanstep = 1;
#else
	fix->xpanstep = 0;
	fix->ypanstep = 0;
#endif
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->accel = FB_ACCEL_NONE;
	fix->visual = FB_VISUAL_TRUECOLOR;
	var->xres = lcd->width;
	var->yres = lcd->height;

#if defined(CONFIG_FB_S3C_VIRTUAL)
	var->xres_virtual = CONFIG_FB_S3C_X_VRES;
	var->yres_virtual = CONFIG_FB_S3C_Y_VRES * CONFIG_FB_S3C_NR_BUFFERS;
#else
	var->xres_virtual = var->xres;
	var->yres_virtual = var->yres * CONFIG_FB_S3C_NR_BUFFERS;
#endif
	var->bits_per_pixel = 32;
	var->xoffset = 0;
	var->yoffset = 0;
	var->width = lcd->p_width;
	var->height = lcd->p_height;
	var->transp.length = 0;

	fix->line_length = var->xres_virtual * var->bits_per_pixel / 8;
	fix->smem_len = fix->line_length * var->yres_virtual;

	var->nonstd = 0;
	var->activate = FB_ACTIVATE_NOW;
	var->vmode = FB_VMODE_NONINTERLACED;
	var->hsync_len = timing->h_sw;
	var->vsync_len = timing->v_sw;
	var->left_margin = timing->h_bp;
	var->right_margin = timing->h_fp;
	var->upper_margin = timing->v_bp;
	var->lower_margin = timing->v_fp;
	var->pixclock = (lcd->freq *
		(var->left_margin + var->right_margin
		+ var->hsync_len + var->xres) *
		(var->upper_margin + var->lower_margin
		+ var->vsync_len + var->yres));
	var->pixclock = KHZ2PICOS(var->pixclock/1000);

	s3cfb_set_bitfield(var);
	s3cfb_set_alpha_info(var, win);

	return 0;
}

int s3cfb_alloc_framebuffer(struct s3cfb_global *fbdev, int fimd_id)
{
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	int ret = 0;
	int i;

	fbdev->fb = kmalloc(pdata->nr_wins *
				sizeof(struct fb_info *), GFP_KERNEL);
	if (!fbdev->fb) {
		dev_err(fbdev->dev, "not enough memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	for (i = 0; i < pdata->nr_wins; i++) {
		fbdev->fb[i] = framebuffer_alloc(sizeof(struct s3cfb_window),
						fbdev->dev);
		if (!fbdev->fb[i]) {
			dev_err(fbdev->dev, "not enough memory\n");
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		ret = s3cfb_init_fbinfo(fbdev, i);
		if (ret) {
			dev_err(fbdev->dev,
				"failed to allocate memory for fb%d\n", i);
			ret = -ENOMEM;
			goto err_alloc_fb;
		}

		if (i == pdata->default_win)
			if (s3cfb_map_default_video_memory(fbdev,
						fbdev->fb[i], fimd_id)) {
				dev_err(fbdev->dev,
				"failed to map video memory "
				"for default window (%d)\n", i);
			ret = -ENOMEM;
			goto err_alloc_fb;
		}
		sec_getlog_supply_fbinfo((void *)fbdev->fb[i]->fix.smem_start,
					 fbdev->fb[i]->var.xres,
					 fbdev->fb[i]->var.yres,
					 fbdev->fb[i]->var.bits_per_pixel, 2);
	}

	return 0;

err_alloc_fb:
	for (i = 0; i < pdata->nr_wins; i++) {
		if (fbdev->fb[i])
			framebuffer_release(fbdev->fb[i]);
	}
	kfree(fbdev->fb);

err_alloc:
	return ret;
}

int s3cfb_open(struct fb_info *fb, int user)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	int ret = 0;

	mutex_lock(&fbdev->lock);

	if (atomic_read(&win->in_use)) {
		if (win->id == pdata->default_win) {
			dev_dbg(fbdev->dev,
				"mutiple open fir default window\n");
			ret = 0;
		} else {
			dev_dbg(fbdev->dev,
				"do not allow multiple open	\
				for non-default window\n");
			ret = -EBUSY;
		}
	} else {
		atomic_inc(&win->in_use);
	}

	mutex_unlock(&fbdev->lock);

	return ret;
}

int s3cfb_release_window(struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);

	if (win->id != pdata->default_win) {
		s3cfb_disable_window(fbdev, win->id);
		s3cfb_unmap_video_memory(fbdev, fb);
		s3cfb_set_buffer_address(fbdev, win->id);
	}

	win->x = 0;
	win->y = 0;

	return 0;
}

int s3cfb_release(struct fb_info *fb, int user)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);

	s3cfb_release_window(fb);

	mutex_lock(&fbdev->lock);
	atomic_dec(&win->in_use);
	mutex_unlock(&fbdev->lock);

	return 0;
}

inline unsigned int __chan_to_field(unsigned int chan, struct fb_bitfield bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf.length;

	return chan << bf.offset;
}

int s3cfb_setcolreg(unsigned int regno, unsigned int red,
			unsigned int green, unsigned int blue,
			unsigned int transp, struct fb_info *fb)
{
	unsigned int *pal = (unsigned int *)fb->pseudo_palette;
	unsigned int val = 0;

	if (regno < 16) {
		/* fake palette of 16 colors */
		val |= __chan_to_field(red, fb->var.red);
		val |= __chan_to_field(green, fb->var.green);
		val |= __chan_to_field(blue, fb->var.blue);
		val |= __chan_to_field(transp, fb->var.transp);
		pal[regno] = val;
	}

	return 0;
}

int s3cfb_blank(int blank_mode, struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_window *tmp_win;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);
	struct platform_device *pdev = to_platform_device(fbdev->dev);
	struct s3c_platform_fb *pdata = to_fb_plat(fbdev->dev);
	int enabled_win = 0;
	int i;

	dev_dbg(fbdev->dev, "change blank mode\n");

#ifdef CONFIG_S5PV310_DEV_PD
	if (fbdev->system_state == POWER_OFF) {
		dev_err(fbdev->dev, "system_state is POWER_OFF\n");
		return 0;
	}
#endif

	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
		if (!fb->fix.smem_start) {
			dev_info(fbdev->dev, "[fb%d] no allocated memory \
				for unblank\n", win->id);
			break;
		}

		if (win->power_state == FB_BLANK_UNBLANK) {
			dev_info(fbdev->dev, "[fb%d] already in \
				FB_BLANK_UNBLANK\n", win->id);
			break;
		} else {
			s3cfb_update_power_state(fbdev, win->id,
						FB_BLANK_UNBLANK);
		}

		enabled_win = atomic_read(&fbdev->enabled_win);
		if (enabled_win == 0) {
			pdata->clk_on(pdev, &fbdev->clock);
			s3cfb_init_global(fbdev);
			s3cfb_set_clock(fbdev);
				for (i = 0; i < pdata->nr_wins; i++) {
					tmp_win = fbdev->fb[i]->par;
					if (tmp_win->owner == DMA_MEM_FIMD)
						s3cfb_set_win_params(fbdev,
								tmp_win->id);
				}
		}

		if (win->enabled)	/* from FB_BLANK_NORMAL */
			s3cfb_win_map_off(fbdev, win->id);
		else			/* from FB_BLANK_POWERDOWN */
			s3cfb_enable_window(fbdev, win->id);

		if (enabled_win == 0) {
			s3cfb_display_on(fbdev);

			if (pdata->backlight_on)
				pdata->backlight_on(pdev);
			if (pdata->lcd_on)
				pdata->lcd_on(pdev);
			if (fbdev->lcd->init_ldi)
				fbdev->lcd->init_ldi();
		}

		break;

	case FB_BLANK_NORMAL:
		if (win->power_state == FB_BLANK_NORMAL) {
			dev_info(fbdev->dev, "[fb%d] already in FB_BLANK_NORMAL\n", win->id);
			break;
		} else {
			s3cfb_update_power_state(fbdev, win->id,
						FB_BLANK_NORMAL);
		}

		enabled_win = atomic_read(&fbdev->enabled_win);
		if (enabled_win == 0) {
			pdata->clk_on(pdev, &fbdev->clock);
			s3cfb_init_global(fbdev);
			s3cfb_set_clock(fbdev);

			for (i = 0; i < pdata->nr_wins; i++) {
				tmp_win = fbdev->fb[i]->par;
				if (tmp_win->owner == DMA_MEM_FIMD)
					s3cfb_set_win_params(fbdev,
							tmp_win->id);
			}
		}

		s3cfb_win_map_on(fbdev, win->id, 0x0);

		if (!win->enabled)	/* from FB_BLANK_POWERDOWN */
			s3cfb_enable_window(fbdev, win->id);

		if (enabled_win == 0) {
			s3cfb_display_on(fbdev);

			if (pdata->backlight_on)
				pdata->backlight_on(pdev);
			if (pdata->lcd_on)
				pdata->lcd_on(pdev);
			if (fbdev->lcd->init_ldi)
				fbdev->lcd->init_ldi();
		}

		break;

	case FB_BLANK_POWERDOWN:
		if (win->power_state == FB_BLANK_POWERDOWN) {
			dev_info(fbdev->dev, "[fb%d] already in FB_BLANK_POWERDOWN\n", win->id);
			break;
		} else {
			s3cfb_update_power_state(fbdev, win->id,
						FB_BLANK_POWERDOWN);
		}

		s3cfb_disable_window(fbdev, win->id);
		s3cfb_win_map_off(fbdev, win->id);

		if (atomic_read(&fbdev->enabled_win) == 0) {
			if (pdata->backlight_off)
				pdata->backlight_off(pdev);
			if (fbdev->lcd->deinit_ldi)
				fbdev->lcd->deinit_ldi();
			if (pdata->lcd_off)
				pdata->lcd_off(pdev);
			s3cfb_display_off(fbdev);
			pdata->clk_off(pdev, &fbdev->clock);
		}

		break;

	case FB_BLANK_VSYNC_SUSPEND:	/* fall through */
	case FB_BLANK_HSYNC_SUSPEND:	/* fall through */
	default:
		dev_dbg(fbdev->dev, "unsupported blank mode\n");
		return -EINVAL;
	}

	return 0;
}

int s3cfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *fb)
{
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);

#ifdef CONFIG_S5PV310_DEV_PD
	if (fbdev->system_state == POWER_OFF) {
		dev_err(fbdev->dev, "system_state is POWER_OFF\n");
		return 0;
	}
#endif

	if (var->yoffset + var->yres > var->yres_virtual) {
		dev_err(fbdev->dev, "invalid yoffset value\n");
		return -EINVAL;
	}

	fb->var.yoffset = var->yoffset;

	dev_dbg(fbdev->dev, "[fb%d] yoffset for pan display: %d\n", win->id,
		var->yoffset);

	s3cfb_set_buffer_address(fbdev, win->id);

	return 0;
}

int s3cfb_cursor(struct fb_info *fb, struct fb_cursor *cursor)
{
	/* nothing to do for removing cursor */
	return 0;
}

int s3cfb_wait_for_vsync(struct s3cfb_global *fbdev)
{
	dev_dbg(fbdev->dev, "waiting for VSYNC interrupt\n");

	sleep_on_timeout(&fbdev->wq, HZ / 10);

	dev_dbg(fbdev->dev, "got a VSYNC interrupt\n");

	return 0;
}

int s3cfb_ioctl(struct fb_info *fb, unsigned int cmd, unsigned long arg)
{
	struct fb_var_screeninfo *var = &fb->var;
	struct s3cfb_window *win = fb->par;
	struct s3cfb_global *fbdev = get_fimd_global(win->id);
	struct s3cfb_lcd *lcd = fbdev->lcd;
	int ret = 0;

	dma_addr_t start_addr = 0;
	struct fb_fix_screeninfo *fix = &fb->fix;

	union {
		struct s3cfb_user_window user_window;
		struct s3cfb_user_plane_alpha user_alpha;
		struct s3cfb_user_chroma user_chroma;
		int vsync;
	} p;

#ifdef CONFIG_S5PV310_DEV_PD
	if (fbdev->system_state == POWER_OFF) {
		dev_err(fbdev->dev, "system_state is POWER_OFF cmd is 0x%08x\n", cmd);
		return -EFAULT;
	}
#endif

	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		s3cfb_wait_for_vsync(fbdev);
		break;

	case S3CFB_WIN_POSITION:
		if (copy_from_user(&p.user_window,
				   (struct s3cfb_user_window __user *)arg,
				   sizeof(p.user_window)))
			ret = -EFAULT;
		else {
			if (p.user_window.x < 0)
				p.user_window.x = 0;

			if (p.user_window.y < 0)
				p.user_window.y = 0;

			if (p.user_window.x + var->xres > lcd->width)
				win->x = lcd->width - var->xres;
			else
				win->x = p.user_window.x;

			if (p.user_window.y + var->yres > lcd->height)
				win->y = lcd->height - var->yres;
			else
				win->y = p.user_window.y;

			s3cfb_set_window_position(fbdev, win->id);
		}
		break;

	case S3CFB_WIN_SET_PLANE_ALPHA:
		if (copy_from_user(&p.user_alpha,
				   (struct s3cfb_user_plane_alpha __user *)arg,
				   sizeof(p.user_alpha)))
			ret = -EFAULT;
		else {
			win->alpha.mode = PLANE_BLENDING;
			win->alpha.channel = p.user_alpha.channel;
			win->alpha.value =
			    S3CFB_AVALUE(p.user_alpha.red,
					 p.user_alpha.green, p.user_alpha.blue);

			s3cfb_set_alpha_blending(fbdev, win->id);
		}
		break;

	case S3CFB_WIN_SET_CHROMA:
		if (copy_from_user(&p.user_chroma,
				   (struct s3cfb_user_chroma __user *)arg,
				   sizeof(p.user_chroma)))
			ret = -EFAULT;
		else {
			win->chroma.enabled = p.user_chroma.enabled;
			win->chroma.key = S3CFB_CHROMA(p.user_chroma.red,
						       p.user_chroma.green,
						       p.user_chroma.blue);

			s3cfb_set_chroma_key(fbdev, win->id);
		}
		break;

	case S3CFB_SET_VSYNC_INT:
		if (get_user(p.vsync, (int __user *)arg))
			ret = -EFAULT;
		else {
			s3cfb_set_global_interrupt(fbdev, p.vsync);
			s3cfb_set_vsync_interrupt(fbdev, p.vsync);
		}
		break;

#if MALI_USE_UNIFIED_MEMORY_PROVIDER
#ifdef CONFIG_VCM
	case S3CFB_GET_FB_UMP_SECURE_ID_0:
		{
			u32 __user *psecureid = (u32 __user *) arg;
			ump_secure_id secure_id;

			dev_info(fbdev->dev, "ump_dd_secure_id_get\n");
			secure_id = ump_dd_secure_id_get(win->ump_wrapped_buffer[0]);
			dev_info(fbdev->dev,
				"Saving secure id 0x%x in userptr %p\n"
				, (unsigned int)secure_id, psecureid);
			dev_dbg(fbdev->dev,
				"Saving secure id 0x%x in userptr %p\n"
				, (unsigned int)secure_id, psecureid);
			return put_user((unsigned int)secure_id, psecureid);
		}
		break;

	case S3CFB_GET_FB_UMP_SECURE_ID_1:
		{
			u32 __user *psecureid = (u32 __user *) arg;
			ump_secure_id secure_id;

			dev_info(fbdev->dev, "ump_dd_secure_id_get\n");
			secure_id = ump_dd_secure_id_get(win->ump_wrapped_buffer[1]);
			dev_info(fbdev->dev,
					"Saving secure id 0x%x in userptr %p\n"
					, (unsigned int)secure_id, psecureid);
			dev_dbg(fbdev->dev,
					"Saving secure id 0x%x in userptr %p\n"
					, (unsigned int)secure_id, psecureid);
			return put_user((unsigned int)secure_id, psecureid);
		}
		break;

	case S3CFB_GET_FB_UMP_SECURE_ID_2:
		{
			u32 __user *psecureid = (u32 __user *) arg;
			ump_secure_id secure_id;

			dev_info(fbdev->dev, "ump_dd_secure_id_get\n");
			secure_id = ump_dd_secure_id_get(win->ump_wrapped_buffer[2]);
			dev_info(fbdev->dev,
					"Saving secure id 0x%x in userptr %p\n"
					, (unsigned int)secure_id, psecureid);
			dev_dbg(fbdev->dev,
					"Saving secure id 0x%x in userptr %p\n"
					, (unsigned int)secure_id, psecureid);
			return put_user((unsigned int)secure_id, psecureid);
		}
		break;
#endif
#endif
	case S3CFB_GET_FB_PHY_ADDR:
		start_addr = fix->smem_start + ((var->xres_virtual *
				var->yoffset + var->xoffset) *
				(var->bits_per_pixel / 8));

		dev_dbg(fbdev->dev, "framebuffer_addr: 0x%08x\n", start_addr);

		if (copy_to_user((void *)arg, &start_addr, sizeof(unsigned int)))
		{
			dev_err(fbdev->dev, "copy_to_user error\n");
			return -EFAULT;
		}

		break;
	}

	return ret;
}

int s3cfb_enable_localpath(struct s3cfb_global *fbdev, int id)
{
	struct s3cfb_window *win = fbdev->fb[id]->par;

	if (s3cfb_channel_localpath_on(fbdev, id)) {
		win->enabled = 0;
		return -EFAULT;
	} else {
		win->enabled = 1;
		return 0;
	}
}

int s3cfb_disable_localpath(struct s3cfb_global *fbdev, int id)
{
	struct s3cfb_window *win = fbdev->fb[id]->par;

	if (s3cfb_channel_localpath_off(fbdev, id)) {
		win->enabled = 1;
		return -EFAULT;
	} else {
		win->enabled = 0;
		return 0;
	}
}

int s3cfb_open_fifo(int id, int ch, int (*do_priv) (void *), void *param)
{
	struct s3cfb_global *fbdev = get_fimd_global(id);
	struct s3cfb_window *win = fbdev->fb[id]->par;

	dev_dbg(fbdev->dev, "[fb%d] open fifo\n", win->id);

	if (win->path == DATA_PATH_DMA) {
		dev_err(fbdev->dev, "WIN%d is busy.\n", id);
		return -EFAULT;
	}

	win->local_channel = ch;

	if (do_priv) {
		if (do_priv(param)) {
			dev_err(fbdev->dev, "failed to run for "
				"private fifo open\n");
			s3cfb_enable_window(fbdev, id);
			return -EFAULT;
		}
	}

	s3cfb_set_window_control(fbdev, id);
	s3cfb_enable_window(fbdev, id);
	s3cfb_enable_localpath(fbdev, id);

	return 0;
}
EXPORT_SYMBOL(s3cfb_open_fifo);

int s3cfb_close_fifo(int id, int (*do_priv) (void *), void *param)
{
	struct s3cfb_global *fbdev = get_fimd_global(id);
	struct s3cfb_window *win = fbdev->fb[id]->par;
	win->path = DATA_PATH_DMA;

	dev_dbg(fbdev->dev, "[fb%d] close fifo\n", win->id);

	if (do_priv) {
		s3cfb_display_off(fbdev);

		if (do_priv(param)) {
			dev_err(fbdev->dev, "failed to run for"
				"private fifo close\n");
			s3cfb_enable_window(fbdev, id);
			s3cfb_display_on(fbdev);
			return -EFAULT;
		}

		s3cfb_disable_window(fbdev, id);
		s3cfb_disable_localpath(fbdev, id);
		s3cfb_display_on(fbdev);
	} else {
		s3cfb_disable_window(fbdev, id);
		s3cfb_disable_localpath(fbdev, id);
	}

	return 0;
}
EXPORT_SYMBOL(s3cfb_close_fifo);

int s3cfb_direct_ioctl(int id, unsigned int cmd, unsigned long arg)
{
	struct s3cfb_global *fbdev = get_fimd_global(id);
	struct fb_info *fb = fbdev->fb[id];
	struct fb_var_screeninfo *var = &fb->var;
	struct fb_fix_screeninfo *fix = &fb->fix;
	struct s3cfb_window *win = fb->par;
	struct s3cfb_lcd *lcd = fbdev->lcd;
	struct s3cfb_user_window user_win;
	struct platform_device *pdev = to_platform_device(fbdev->dev);
	void *argp = (void *)arg;
	int ret = 0;

#ifdef CONFIG_S5PV310_DEV_PD
	/* enable the power domain */
	if (fbdev->system_state == POWER_OFF) {
		/* This IOCTLs are came from fimc irq.
		 * To avoid scheduling problem in irq mode,
		 * runtime get/put sync shold be not called.
		 */
		switch (cmd) {
		case S3CFB_SET_WIN_ADDR:
			fix->smem_start = (unsigned long)argp;
			return ret;

		case S3CFB_SET_WIN_ON:
			if (!win->enabled)
				atomic_inc(&fbdev->enabled_win);
			win->enabled = 1;

			return ret;
		}

		pm_runtime_get_sync(&pdev->dev);
	}
#endif

	switch (cmd) {
	case FBIOGET_FSCREENINFO:
		ret = memcpy(argp, &fb->fix, sizeof(fb->fix)) ? 0 : -EFAULT;
		break;

	case FBIOGET_VSCREENINFO:
		ret = memcpy(argp, &fb->var, sizeof(fb->var)) ? 0 : -EFAULT;
		break;

	case FBIOPUT_VSCREENINFO:
		ret = s3cfb_check_var((struct fb_var_screeninfo *)argp, fb);
		if (ret) {
			dev_err(fbdev->dev, "invalid vscreeninfo\n");
			break;
		}

		ret = memcpy(&fb->var, (struct fb_var_screeninfo *)argp,
			     sizeof(fb->var)) ? 0 : -EFAULT;
		if (ret) {
			dev_err(fbdev->dev, "failed to put new vscreeninfo\n");
			break;
		}

		fix->line_length = var->xres_virtual * var->bits_per_pixel / 8;
		fix->smem_len = fix->line_length * var->yres_virtual;

		s3cfb_set_win_params(fbdev, id);
		break;

	case S3CFB_WIN_POSITION:
		ret = memcpy(&user_win, (struct s3cfb_user_window __user *)arg,
			     sizeof(user_win)) ? 0 : -EFAULT;
		if (ret) {
			dev_err(fbdev->dev, "failed to S3CFB_WIN_POSITION\n");
			break;
		}

		if (user_win.x < 0)
			user_win.x = 0;

		if (user_win.y < 0)
			user_win.y = 0;

		if (user_win.x + var->xres > lcd->width)
			win->x = lcd->width - var->xres;
		else
			win->x = user_win.x;

		if (user_win.y + var->yres > lcd->height)
			win->y = lcd->height - var->yres;
		else
			win->y = user_win.y;

		s3cfb_set_window_position(fbdev, win->id);
		break;

	case S3CFB_GET_LCD_WIDTH:
		ret = memcpy(argp, &lcd->width, sizeof(int)) ? 0 : -EFAULT;
		if (ret) {
			dev_err(fbdev->dev, "failed to S3CFB_GET_LCD_WIDTH\n");
			break;
		}

		break;

	case S3CFB_GET_LCD_HEIGHT:
		ret = memcpy(argp, &lcd->height, sizeof(int)) ? 0 : -EFAULT;
		if (ret) {
			dev_err(fbdev->dev, "failed to S3CFB_GET_LCD_HEIGHT\n");
			break;
		}

		break;

	case S3CFB_SET_WRITEBACK:
		if ((u32)argp == 1)
			fbdev->output = OUTPUT_WB_RGB;
		else
			fbdev->output = OUTPUT_RGB;

		s3cfb_set_output(fbdev);

		break;

	case S3CFB_SET_WIN_ON:
		s3cfb_enable_window(fbdev, id);
		break;

	case S3CFB_SET_WIN_OFF:
		s3cfb_disable_window(fbdev, id);
		break;

	case S3CFB_SET_WIN_PATH:
		win->path = (enum s3cfb_data_path_t)argp;
		break;

	case S3CFB_SET_WIN_ADDR:
		fix->smem_start = (unsigned long)argp;
		s3cfb_set_buffer_address(fbdev, id);
		break;

	case S3CFB_SET_WIN_MEM:
		win->owner = (enum s3cfb_mem_owner_t)argp;
		break;

	case S3CFB_SET_VSYNC_INT:
		if (argp)
			s3cfb_set_global_interrupt(fbdev, 1);

		s3cfb_set_vsync_interrupt(fbdev, (int)argp);
		break;

	case S3CFB_GET_VSYNC_INT_STATUS:
		ret = s3cfb_get_vsync_interrupt(fbdev);
		break;

	default:
#ifdef CONFIG_S5PV310_DEV_PD
		fbdev->system_state = POWER_ON;
#endif
		ret = s3cfb_ioctl(fb, cmd, arg);
#ifdef CONFIG_S5PV310_DEV_PD
		fbdev->system_state = POWER_OFF;
#endif
		break;
	}

#ifdef CONFIG_S5PV310_DEV_PD
	/* disable the power domain */
	if (fbdev->system_state == POWER_OFF)
		pm_runtime_put(&pdev->dev);
#endif

	return ret;
}
EXPORT_SYMBOL(s3cfb_direct_ioctl);

MODULE_AUTHOR("Jingoo Han <jg1.han@samsung.com>");
MODULE_AUTHOR("Jonghun, Han <jonghun.han@samsung.com>");
MODULE_AUTHOR("Jinsung, Yang <jsgood.yang@samsung.com>");
MODULE_DESCRIPTION("Samsung Display Controller (FIMD) driver");
MODULE_LICENSE("GPL");
