/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _NPU_MGR_H
#define _NPU_MGR_H

/* -------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------
 */
#include "npu_hw_access.h"

/* -------------------------------------------------------------------------
 * Defines
 * -------------------------------------------------------------------------
 */
#define NW_SMALL_EXEC_TIMEOUT_MS (1000*300)	/* set for 5 min */
#define NW_SMALL_EXEC_TIMEOUT msecs_to_jiffies(NW_SMALL_EXEC_TIMEOUT_MS)
#define NW_LARGE_EXEC_TIMEOUT_MS (1000*300*12)	/* set for 60 min */
#define NW_LARGE_EXEC_TIMEOUT msecs_to_jiffies(NW_LARGE_EXEC_TIMEOUT_MS)
#define NW_LOAD_TIMEOUT_MS (1000*300) /* set for 5 min */
#define NW_LOAD_TIMEOUT msecs_to_jiffies(NW_LOAD_TIMEOUT_MS)
#define NW_UNLOAD_TIMEOUT_MS (1000*300) /* set for 5 min */
#define NW_UNLOAD_TIMEOUT msecs_to_jiffies(NW_UNLOAD_TIMEOUT_MS)
#define FIRMWARE_VERSION 0x00001000
#define MAX_LOADED_NETWORK 32
#define LARGE_NETWORK_SIZE_THRESHOLD (5*1024) /* 5 KB */
#define NPU_IPC_BUF_LENGTH 512

/* -------------------------------------------------------------------------
 * Data Structures
 * -------------------------------------------------------------------------
 */
struct npu_network {
	uint64_t id;
	int buf_hdl;
	uint64_t phy_add;
	uint32_t size;
	uint32_t first_block_size;
	uint32_t network_hdl;
	uint32_t ipc_trans_id;
};

struct npu_host_ctx {
	void *subsystem_handle;
	bool fw_enabled;
	bool power_enabled;
	int32_t power_vote_num;
	struct work_struct irq_work;
	struct workqueue_struct *wq;
	struct completion exec_done;
	struct completion load_done;
	struct completion unload_done;
	int32_t network_num;
	struct npu_network networks[MAX_LOADED_NETWORK];
};

struct npu_device;

/* -------------------------------------------------------------------------
 * Function Prototypes
 * -------------------------------------------------------------------------
 */
int npu_host_init(struct npu_device *npu_dev);
void npu_host_deinit(struct npu_device *npu_dev);

/* Host Driver IPC Interface */
int npu_host_ipc_pre_init(struct npu_device *npu_dev);
int npu_host_ipc_post_init(struct npu_device *npu_dev);
void npu_host_ipc_deinit(struct npu_device *npu_dev);
int npu_host_ipc_send_cmd(struct npu_device *npu_dev, uint32_t queueIndex,
	void *pCmd);
int npu_host_ipc_read_msg(struct npu_device *npu_dev, uint32_t queueIndex,
	uint32_t *pMsg);

int32_t npu_host_get_info(struct npu_device *npu_dev,
	struct msm_npu_get_info_ioctl *get_info_ioctl);
int32_t npu_host_map_buf(struct npu_device *npu_dev,
	struct msm_npu_map_buf_ioctl *map_ioctl);
int32_t npu_host_unmap_buf(struct npu_device *npu_dev,
	struct msm_npu_unmap_buf_ioctl *unmap_ioctl);
int32_t npu_host_load_network(struct npu_device *npu_dev,
	struct msm_npu_load_network_ioctl *load_ioctl);
int32_t npu_host_unload_network(struct npu_device *npu_dev,
	struct msm_npu_unload_network_ioctl *unload);
int32_t npu_host_exec_network(struct npu_device *npu_dev,
	struct msm_npu_exec_network_ioctl *exec_ioctl);

void npu_dump_debug_timeout_stats(struct npu_device *npu_dev);
void npu_dump_cal_state(struct npu_device *npu_dev);

#endif /* _NPU_MGR_H */