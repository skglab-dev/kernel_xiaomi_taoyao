/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (C) 2020 XiaoMi, Inc.
 */

#ifndef _MI_DSI_PANEL_H_
#define _MI_DSI_PANEL_H_

struct dsi_panel;

struct mi_dsi_panel_cfg {
	/* xiaomi panel id */
	u64 panel_id;

	int local_hbm_normal_alpha_87_index;
	int local_hbm_hlpm_alpha_87_index;
};

int mi_dsi_panel_write_cmd_set(struct dsi_panel *panel,
				struct dsi_panel_cmd_set *cmd_sets);

int mi_dsi_update_lhbm_cmd_87reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl);

#endif /* _MI_DSI_PANEL_H_ */
