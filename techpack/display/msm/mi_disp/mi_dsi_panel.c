/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (C) 2020 XiaoMi, Inc.
 */

#include "dsi_display.h"
#include "dsi_panel.h"

#include "mi_dsi_panel.h"
#include "mi_disp_nvt_alpha_data.h"

extern const char *cmd_set_prop_map[DSI_CMD_SET_MAX];

int mi_dsi_panel_write_cmd_set(struct dsi_panel *panel,
				struct dsi_panel_cmd_set *cmd_sets)
{
	int rc = 0, i = 0;
	ssize_t len;
	struct dsi_cmd_desc *cmds;
	u32 count;
	enum dsi_cmd_set_state state;
	struct dsi_display_mode *mode;
	const struct mipi_dsi_host_ops *ops = panel->host->ops;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mode = panel->cur_mode;

	cmds = cmd_sets->cmds;
	count = cmd_sets->count;
	state = cmd_sets->state;

	if (count == 0) {
		DSI_DEBUG("[%s] No commands to be sent for state\n", panel->type);
		goto error;
	}

	for (i = 0; i < count; i++) {
		if (state == DSI_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		if (cmds->last_command)
			cmds->msg.flags |= MIPI_DSI_MSG_LASTCOMMAND;

		len = ops->transfer(panel->host, &cmds->msg);
		if (len < 0) {
			rc = len;
			DSI_ERR("failed to set cmds, rc=%d\n", rc);
			goto error;
		}
		if (cmds->post_wait_ms)
			usleep_range(cmds->post_wait_ms * 1000,
					((cmds->post_wait_ms * 1000) + 10));
		cmds++;
	}
error:
	return rc;
}

int mi_dsi_update_lhbm_cmd_87reg(struct dsi_panel *panel,
			enum dsi_cmd_set_type type, int bl_lvl)
{
	struct dsi_display_mode_priv_info *priv_info;
	struct dsi_cmd_desc *cmds = NULL;
	struct mi_dsi_panel_cfg *mi_cfg  = NULL;
	u32 count;
	int index;
	u8 *tx_buf;
	int rc = 0;

	if (!panel || !panel->cur_mode || !panel->cur_mode->priv_info) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mi_cfg = &panel->mi_cfg;
	priv_info = panel->cur_mode->priv_info;

	switch (type) {
		case DSI_CMD_SET_MI_LOCAL_HBM_NORMAL_WHITE_1000NIT:
			index = mi_cfg->local_hbm_normal_alpha_87_index;
			break;
		case DSI_CMD_SET_MI_LOCAL_HBM_HLPM_WHITE_1000NIT:
			index = mi_cfg->local_hbm_hlpm_alpha_87_index;
			break;
		default:
			DSI_ERR("%s panel wrong cmd type!\n", panel->type);
			return -EINVAL;
	}

	if (index < 0) {
		DSI_DEBUG("%s panel cmd[%s] update not supported\n",
			panel->type, cmd_set_prop_map[type]);
			return 0;
	}

	DSI_INFO("cmd[%s], bl_lvl=%d\n", cmd_set_prop_map[type], bl_lvl);

	/* update lhbm aa area alpha */
	cmds = priv_info->cmd_sets[type].cmds;
	count = priv_info->cmd_sets[type].count;
	if (cmds && count >= index) {
		tx_buf = (u8 *)cmds[index].msg.tx_buf;
		if (tx_buf && tx_buf[0] == 0x87) {
			if(mi_cfg->panel_id == 0x4C3900420200) {
				tx_buf[1] = (aa_alpha_set_4c3942[bl_lvl] >> 8) & 0xff;
				tx_buf[2] = aa_alpha_set_4c3942[bl_lvl] & 0xff;
				DSI_INFO("panel lhbm alpha cmd[0x%02x] = 0x%02x 0x%02x\n",tx_buf[0], tx_buf[1], tx_buf[2]);
			}
			if(mi_cfg->panel_id == 0x4C3900360200) {
				tx_buf[1] = (aa_alpha_set_4c3936[bl_lvl] >> 8) & 0xff;
				tx_buf[2] = aa_alpha_set_4c3936[bl_lvl] & 0xff;
				DSI_INFO("panel lhbm alpha cmd[0x%02x] = 0x%02x 0x%02x\n",tx_buf[0], tx_buf[1], tx_buf[2]);
			}
		} else {
			if (tx_buf) {
				DSI_ERR("%s panel aa index = %d, tx_buf[0] = 0x%02X, check cmd[%s] index\n",
					panel->type, index, tx_buf[0], cmd_set_prop_map[type]);
			} else {
				DSI_ERR("%s panel tx_buf is NULL pointer\n", panel->type);
			}
			rc = -EINVAL;
		}
	} else {
		DSI_ERR("%s panel aa cmd[%s] 0x87 index(%d) error\n",
			panel->type, cmd_set_prop_map[type], index);
		rc = -EINVAL;
	}
	return rc;
}
