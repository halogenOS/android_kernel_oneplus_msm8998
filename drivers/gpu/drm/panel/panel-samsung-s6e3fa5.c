// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2020 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct samsung_s6e3fa5 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline
struct samsung_s6e3fa5 *to_samsung_s6e3fa5(struct drm_panel *panel)
{
	return container_of(panel, struct samsung_s6e3fa5, panel);
}

#define dsi_dcs_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void samsung_s6e3fa5_reset(struct samsung_s6e3fa5 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(2000, 3000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(2000, 3000);
}

static int samsung_s6e3fa5_on(struct samsung_s6e3fa5 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(20);

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret < 0) {
		dev_err(dev, "Failed to set tear on: %d\n", ret);
		return ret;
	}

	dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	dsi_dcs_write_seq(dsi, 0xb0, 0x04);
	dsi_dcs_write_seq(dsi, 0xb4, 0x06, 0x0c, 0x12);
	dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x20);
	dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	dsi_dcs_write_seq(dsi, 0xc3, 0x01);
	dsi_dcs_write_seq(dsi, 0xb0, 0x18);
	dsi_dcs_write_seq(dsi, 0xc3, 0x00);
	dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}

	return 0;
}

static int samsung_s6e3fa5_off(struct samsung_s6e3fa5 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(40);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(160);

	return 0;
}

static int samsung_s6e3fa5_prepare(struct drm_panel *panel)
{
	struct samsung_s6e3fa5 *ctx = to_samsung_s6e3fa5(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	samsung_s6e3fa5_reset(ctx);

	ret = samsung_s6e3fa5_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 0);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int samsung_s6e3fa5_unprepare(struct drm_panel *panel)
{
	struct samsung_s6e3fa5 *ctx = to_samsung_s6e3fa5(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = samsung_s6e3fa5_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 0);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode samsung_s6e3fa5_mode = {
	.clock = (1080 + 120 + 19 + 70) * (1920 + 18 + 2 + 4) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 120,
	.hsync_end = 1080 + 120 + 19,
	.htotal = 1080 + 120 + 19 + 70,
	.vdisplay = 1920,
	.vsync_start = 1920 + 18,
	.vsync_end = 1920 + 18 + 2,
	.vtotal = 1920 + 18 + 2 + 4,
	.vrefresh = 60,
	.width_mm = 68,
	.height_mm = 122,
};

static int samsung_s6e3fa5_get_modes(struct drm_panel *panel,
				     struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &samsung_s6e3fa5_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs samsung_s6e3fa5_panel_funcs = {
	.prepare = samsung_s6e3fa5_prepare,
	.unprepare = samsung_s6e3fa5_unprepare,
	.get_modes = samsung_s6e3fa5_get_modes,
};

static int samsung_s6e3fa5_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = bl->props.brightness;
	int ret;

	if (bl->props.power != FB_BLANK_UNBLANK ||
	    bl->props.fb_blank != FB_BLANK_UNBLANK ||
	    bl->props.state & (BL_CORE_SUSPENDED | BL_CORE_FBBLANK))
		brightness = 0;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

// TODO: Check if /sys/class/backlight/.../actual_brightness actually returns
// correct values. If not, remove this function.
static int samsung_s6e3fa5_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = bl->props.brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness & 0xff;
}

static const struct backlight_ops samsung_s6e3fa5_bl_ops = {
	.update_status = samsung_s6e3fa5_bl_update_status,
	.get_brightness = samsung_s6e3fa5_bl_get_brightness,
};

static struct backlight_device *
samsung_s6e3fa5_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &samsung_s6e3fa5_bl_ops, &props);
}

static int samsung_s6e3fa5_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung_s6e3fa5 *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->reset_gpio)) {
		ret = PTR_ERR(ctx->reset_gpio);
		dev_err(dev, "Failed to get reset-gpios: %d\n", ret);
		return ret;
	}

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_VIDEO_HSE |
			  MIPI_DSI_MODE_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &samsung_s6e3fa5_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ctx->panel.backlight = samsung_s6e3fa5_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight)) {
		ret = PTR_ERR(ctx->panel.backlight);
		dev_err(dev, "Failed to create backlight: %d\n", ret);
		return ret;
	}

	ret = drm_panel_add(&ctx->panel);
	if (ret < 0) {
		dev_err(dev, "Failed to add panel: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		return ret;
	}

	return 0;
}

static int samsung_s6e3fa5_remove(struct mipi_dsi_device *dsi)
{
	struct samsung_s6e3fa5 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id samsung_s6e3fa5_of_match[] = {
	{ .compatible = "samsung,s6e3fa5" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, samsung_s6e3fa5_of_match);

static struct mipi_dsi_driver samsung_s6e3fa5_driver = {
	.probe = samsung_s6e3fa5_probe,
	.remove = samsung_s6e3fa5_remove,
	.driver = {
		.name = "panel-samsung-s6e3fa5",
		.of_match_table = samsung_s6e3fa5_of_match,
	},
};
module_mipi_dsi_driver(samsung_s6e3fa5_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for samsung s6e3fa5 1080p cmd mode dsi panel");
MODULE_LICENSE("GPL v2");
