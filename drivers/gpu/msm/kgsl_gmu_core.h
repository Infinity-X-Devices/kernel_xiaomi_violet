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
 *
 */
#ifndef __KGSL_GMU_CORE_H
#define __KGSL_GMU_CORE_H

#define GMU_INT_WDOG_BITE		BIT(0)
#define GMU_INT_RSCC_COMP		BIT(1)
#define GMU_INT_FENCE_ERR		BIT(3)
#define GMU_INT_DBD_WAKEUP		BIT(4)
#define GMU_INT_HOST_AHB_BUS_ERR	BIT(5)
#define GMU_AO_INT_MASK		\
		(GMU_INT_WDOG_BITE |	\
		GMU_INT_HOST_AHB_BUS_ERR |	\
		GMU_INT_FENCE_ERR)

/* GMU_DEVICE - Given an KGSL device return the GMU specific struct */
#define GMU_DEVICE_OPS(_a) ((_a)->gmu_core.dev_ops)
#define GMU_CORE_OPS(_a) ((_a)->gmu_core.core_ops)

#define NUM_BW_LEVELS		100
#define MAX_GX_LEVELS		16
#define MAX_CX_LEVELS		4
#define MAX_CNOC_LEVELS		2
#define MAX_CNOC_CMDS		6
#define MAX_BW_CMDS		8
#define INVALID_DCVS_IDX	0xFF

#if MAX_CNOC_LEVELS > MAX_GX_LEVELS
#error "CNOC levels cannot exceed GX levels"
#endif

#define MAX_GMU_CLKS 6
#define DEFAULT_GMU_FREQ_IDX 1

/*
 * These are the different ways the GMU can boot. GMU_WARM_BOOT is waking up
 * from slumber. GMU_COLD_BOOT is booting for the first time. GMU_RESET
 * is a soft reset of the GMU.
 */
enum gmu_core_boot {
	GMU_WARM_BOOT = 0,
	GMU_COLD_BOOT = 1,
	GMU_RESET = 2
};

/* Bits for the flags field in the gmu structure */
enum gmu_core_flags {
	GMU_BOOT_INIT_DONE = 0,
	GMU_CLK_ON,
	GMU_HFI_ON,
	GMU_FAULT,
	GMU_DCVS_REPLAY,
	GMU_GPMU,
	GMU_ENABLED,
	GMU_RSCC_SLEEP_SEQ_DONE,
};

/*
 * OOB requests values. These range from 0 to 7 and then
 * the BIT() offset into the actual value is calculated
 * later based on the request. This keeps the math clean
 * and easy to ensure not reaching over/under the range
 * of 8 bits.
 */
enum oob_request {
	oob_gpu = 0,
	oob_perfcntr = 1,
	oob_preempt = 2,
	oob_boot_slumber = 6, /* reserved special case */
	oob_dcvs = 7, /* reserved special case */
};

/*
 * Wait time before trying to write the register again.
 * Hopefully the GMU has finished waking up during this delay.
 * This delay must be less than the IFPC main hysteresis or
 * the GMU will start shutting down before we try again.
 */
#define GMU_CORE_WAKEUP_DELAY_US 10
/* Max amount of tries to wake up the GMU. */
#define GMU_CORE_WAKEUP_RETRY_MAX 60

#define FENCE_STATUS_WRITEDROPPED0_MASK 0x1
#define FENCE_STATUS_WRITEDROPPED1_MASK 0x2

struct kgsl_device;
struct adreno_device;
struct kgsl_snapshot;

struct gmu_core_ops {
	int (*probe)(struct kgsl_device *device, struct device_node *node,
			unsigned long flags);
	void (*remove)(struct kgsl_device *device);
	void (*regread)(struct kgsl_device *device,
		unsigned int offsetwords, unsigned int *value);
	void (*regwrite)(struct kgsl_device *device,
		unsigned int offsetwords, unsigned int value);
	bool (*isenabled)(struct kgsl_device *device);
	bool (*gpmu_isenabled)(struct kgsl_device *device);
	int (*dcvs_set)(struct kgsl_device *device,
			unsigned int gpu_pwrlevel, unsigned int bus_level);
	void (*set_bit)(struct kgsl_device *device, enum gmu_core_flags flag);
	void (*clear_bit)(struct kgsl_device *device, enum gmu_core_flags flag);
	int (*test_bit)(struct kgsl_device *device, enum gmu_core_flags flag);
	int (*start)(struct kgsl_device *device);
	void (*stop)(struct kgsl_device *device);
	void (*snapshot)(struct kgsl_device *device);
	int (*get_idle_level)(struct kgsl_device *device);
	void (*set_idle_level)(struct kgsl_device *device, unsigned int val);
	bool (*regulator_isenabled)(struct kgsl_device *device);
};

struct gmu_dev_ops {
	int (*load_firmware)(struct kgsl_device *device);
	int (*oob_set)(struct adreno_device *adreno_dev,
			enum oob_request req);
	void (*oob_clear)(struct adreno_device *adreno_dev,
			enum oob_request req);
	int (*hfi_start_msg)(struct adreno_device *adreno_dev);
	void (*enable_lm)(struct kgsl_device *device);
	int (*rpmh_gpu_pwrctrl)(struct adreno_device *, unsigned int ops,
			unsigned int arg1, unsigned int arg2);
	int (*wait_for_lowest_idle)(struct adreno_device *);
	int (*wait_for_gmu_idle)(struct adreno_device *);
	int (*sptprac_enable)(struct adreno_device *adreno_dev);
	void (*sptprac_disable)(struct adreno_device *adreno_dev);
	int (*ifpc_store)(struct adreno_device *adreno_dev,
			unsigned int val);
	unsigned int (*ifpc_show)(struct adreno_device *adreno_dev);
	void (*snapshot)(struct adreno_device *, struct kgsl_snapshot *);
};

/**
 * struct gmu_core_device - GMU Core device structure
 * @ptr: Pointer to GMU device structure
 * @gmu2gpu_offset: address difference between GMU register set
 *	and GPU register set, the offset will be used when accessing
 *	gmu registers using offset defined in GPU register space.
 * @reg_len: GMU registers length
 * @core_ops: Pointer to gmu core operations
 * @dev_ops: Pointer to gmu device operations
 */
struct gmu_core_device {
	void *ptr;
	unsigned int gmu2gpu_offset;
	unsigned int reg_len;
	struct gmu_core_ops *core_ops;
	struct gmu_dev_ops *dev_ops;
};

/* GMU core functions */
extern struct gmu_core_ops gmu_ops;

int gmu_core_probe(struct kgsl_device *device);
void gmu_core_remove(struct kgsl_device *device);
int gmu_core_start(struct kgsl_device *device);
void gmu_core_stop(struct kgsl_device *device);
void gmu_core_snapshot(struct kgsl_device *device);
bool gmu_core_gpmu_isenabled(struct kgsl_device *device);
bool gmu_core_isenabled(struct kgsl_device *device);
int gmu_core_dcvs_set(struct kgsl_device *device, unsigned int gpu_pwrlevel,
		unsigned int bus_level);
void gmu_core_setbit(struct kgsl_device *device, enum gmu_core_flags flag);
void gmu_core_clearbit(struct kgsl_device *device, enum gmu_core_flags flag);
int gmu_core_testbit(struct kgsl_device *device, enum gmu_core_flags flag);
bool gmu_core_regulator_isenabled(struct kgsl_device *device);
bool gmu_core_is_register_offset(struct kgsl_device *device,
				unsigned int offsetwords);
void gmu_core_regread(struct kgsl_device *device, unsigned int offsetwords,
		unsigned int *value);
void gmu_core_regwrite(struct kgsl_device *device, unsigned int offsetwords,
		unsigned int value);
void gmu_core_regrmw(struct kgsl_device *device, unsigned int offsetwords,
		unsigned int mask, unsigned int bits);
#endif /* __KGSL_GMU_CORE_H */