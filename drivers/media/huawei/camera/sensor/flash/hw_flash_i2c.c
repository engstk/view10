/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#include <linux/i2c.h>
#include "hw_flash_i2c.h"

extern int memset_s(void *dest, size_t destMax, int c, size_t count);
int hw_flash_i2c_read(struct hw_flash_i2c_client *client,
	u8 reg, u8 *data)
{
	int rc = 0;
	struct i2c_msg msgs[2];

	cam_debug("%s enter.\n", __func__);

	msgs[0].addr = client->client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = client->client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = data;

	rc = i2c_transfer(client->client->adapter, msgs, 2);
	if (rc < 0) {
		cam_err("%s transfer error, reg=0x%x, data=0x%x.",
			__func__, reg, *data);
	} else {
		cam_debug("%s reg=0x%x, data=0x%x.\n", __func__, reg, *data);
	}

	return rc;
}

int hw_flash_i2c_write(struct hw_flash_i2c_client *client,
	u8 reg, u8 data)
{
	int rc = 0;
	u8 buf[2] = {0};
	struct i2c_msg msg = {0};

	cam_debug("%s reg=0x%x, data=0x%x.\n", __func__, reg, data);

	buf[0] = reg;
	buf[1] = data;
	msg.addr = client->client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = buf;

	rc = i2c_transfer(client->client->adapter, &msg, 1);
	if (rc < 0) {
		cam_err("%s transfer error, reg=0x%x, data=0x%x.",
			__func__, reg, data);
	}

	return rc;
}

#define MAX_BUF_LEN  16
/*lint -e679 */
int hw_flash_i2c_writex(struct hw_flash_i2c_client *client,
                        struct hw_flash_i2c_msgs_t *msgs,
                        int len)
{
    int rc = 0;
    int i  = 0;
    u8 buf[MAX_BUF_LEN*2] = {0};
    struct i2c_msg msg[MAX_BUF_LEN];

    if ((NULL == client) || (NULL == msgs)) {
        cam_err("%s invalid params, client or msgs is null", __func__);
        return -EINVAL;
    }

    if (len > MAX_BUF_LEN) {
        cam_err("%s invalid params, len(%d) > MAX_BUF_LEN(%d)", __func__,
            len, MAX_BUF_LEN);
        return -EINVAL;
    }

    memset_s(msg, MAX_BUF_LEN*sizeof(struct i2c_msg),
        0, MAX_BUF_LEN*sizeof(struct i2c_msg));

    for(i=0; i<len; i++)
    {
        buf[i*2]   = msgs[i].reg;
        buf[i*2+1] = msgs[i].data;
        msg[i].addr  = client->client->addr;
        msg[i].flags = 0;
        msg[i].len = 2;
        msg[i].buf = &buf[i*2];
        cam_info("%s reg:0x%x, data:0x%x", __func__, msgs[i].reg, msgs[i].data);
    }

    rc = i2c_transfer(client->client->adapter, msg, len);
    if (rc < 0) {
        cam_err("%s transfer error", __func__);
    }

    return rc;
}
/*lint +e679 */
struct hw_flash_i2c_fn_t hw_flash_i2c_func_tbl = {
	.i2c_read = hw_flash_i2c_read,
	.i2c_write = hw_flash_i2c_write,
	.i2c_writex = hw_flash_i2c_writex,
};
