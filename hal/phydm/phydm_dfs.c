/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/*@
 * ============================================================
 * include files
 * ============================================================
 */

#include "mp_precomp.h"
#include "phydm_precomp.h"

#if defined(CONFIG_PHYDM_DFS_MASTER)

boolean phydm_dfs_is_meteorology_channel(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u8 ch = *dm->channel;
	u8 bw = *dm->band_width;

	return ((bw  == CHANNEL_WIDTH_80 && (ch) >= 116 && (ch) <= 128) ||
		(bw  == CHANNEL_WIDTH_40 && (ch) >= 116 && (ch) <= 128) ||
		(bw  == CHANNEL_WIDTH_20 && (ch) >= 120 && (ch) <= 128));
}

void phydm_dfs_segment_distinguish(void *dm_void, enum rf_syn syn_path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (!(dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C)))
		return;
	if (syn_path == RF_SYN1)
		dm->seg1_dfs_flag = 1;
	else
		dm->seg1_dfs_flag = 0;
}

void phydm_dfs_segment_flag_reset(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (!(dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C)))
		return;
	if (dm->seg1_dfs_flag)
		dm->seg1_dfs_flag = 0;
}

void phydm_radar_detect_reset(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type & (ODM_RTL8198F | ODM_RTL8822C | ODM_RTL8812F |\
				   ODM_RTL8197G | ODM_RTL8733B | ODM_RTL8735B |\
				   ODM_RTL8730A | ODM_RTL8822E)) {
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 1);
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & (ODM_RTL8721D)) {
		odm_set_bb_reg(dm, R_0xf58, BIT(29), 0);
		odm_set_bb_reg(dm, R_0xf58, BIT(29), 1);
	#endif
	} else if (dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C)) {
		if (dm->seg1_dfs_flag == 1) {
			odm_set_bb_reg(dm, R_0xa6c, BIT(0), 0);
			odm_set_bb_reg(dm, R_0xa6c, BIT(0), 1);
			return;
		}
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 1);
	} else {
		odm_set_bb_reg(dm, R_0x924, BIT(15), 0);
		odm_set_bb_reg(dm, R_0x924, BIT(15), 1);
	}
}

void phydm_radar_detect_disable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type & (ODM_RTL8198F | ODM_RTL8822C | ODM_RTL8812F |\
				   ODM_RTL8197G | ODM_RTL8733B | ODM_RTL8735B |\
				   ODM_RTL8730A | ODM_RTL8822E))
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
	else if (dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C)) {
		if (dm->seg1_dfs_flag == 1) {
			odm_set_bb_reg(dm, R_0xa6c, BIT(0), 0);
			dm->seg1_dfs_flag = 0;
			return;
		}
		odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
	}
	#if (RTL8721D_SUPPORT)
	else if (dm->support_ic_type & (ODM_RTL8721D))
		odm_set_bb_reg(dm, R_0xf58, BIT(29), 0);
	#endif
	else
		odm_set_bb_reg(dm, R_0x924, BIT(15), 0);

	PHYDM_DBG(dm, DBG_DFS, "\n");
}

static void phydm_radar_detect_with_dbg_parm(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type & ODM_RTL8721D) {
		odm_set_bb_reg(dm, R_0xf54, MASKDWORD,
			       dm->radar_detect_reg_f54);
		odm_set_bb_reg(dm, R_0xf58, MASKDWORD,
			       dm->radar_detect_reg_f58);
		odm_set_bb_reg(dm, R_0xf5c, MASKDWORD,
			       dm->radar_detect_reg_f5c);
		odm_set_bb_reg(dm, R_0xf70, MASKDWORD,
			       dm->radar_detect_reg_f70);
		odm_set_bb_reg(dm, R_0xf74, MASKDWORD,
			       dm->radar_detect_reg_f74);
	} else if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		odm_set_bb_reg(dm, R_0xa40, MASKDWORD,
			       dm->radar_detect_reg_a40);
		odm_set_bb_reg(dm, R_0xa44, MASKDWORD,
			       dm->radar_detect_reg_a44);
		odm_set_bb_reg(dm, R_0xa48, MASKDWORD,
			       dm->radar_detect_reg_a48);
		odm_set_bb_reg(dm, R_0xa4c, MASKDWORD,
			       dm->radar_detect_reg_a4c);
		odm_set_bb_reg(dm, R_0xa50, MASKDWORD,
			       dm->radar_detect_reg_a50);
		odm_set_bb_reg(dm, R_0xa54, MASKDWORD,
			       dm->radar_detect_reg_a54);
	} else {
		odm_set_bb_reg(dm, R_0x918, MASKDWORD,
			       dm->radar_detect_reg_918);
		odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
			       dm->radar_detect_reg_91c);
		odm_set_bb_reg(dm, R_0x920, MASKDWORD,
			       dm->radar_detect_reg_920);
		odm_set_bb_reg(dm, R_0x924, MASKDWORD,
			       dm->radar_detect_reg_924);
	}
}

/* @Init radar detection parameters, called after ch, bw is set */

void phydm_radar_detect_enable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;
	u8 band_width = *dm->band_width;
	u8 enable = 0, i;
	u8 short_pw_upperbound = 0;

	PHYDM_DBG(dm, DBG_DFS, "test, region_domain = %d\n", region_domain);
	if (region_domain == PHYDM_DFS_DOMAIN_UNKNOWN) {
		PHYDM_DBG(dm, DBG_DFS, "PHYDM_DFS_DOMAIN_UNKNOWN\n");
		goto exit;
	}

	if (dm->support_ic_type & (ODM_RTL8821 | ODM_RTL8812 | ODM_RTL8881A)) {
		odm_set_bb_reg(dm, R_0x814, 0x3fffffff, 0x04cc4d10);
		odm_set_bb_reg(dm, R_0x834, MASKBYTE0, 0x06);

		if (dm->radar_detect_dbg_parm_en) {
			phydm_radar_detect_with_dbg_parm(dm);
			enable = 1;
			goto exit;
		}

		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			odm_set_bb_reg(dm, R_0x918, MASKDWORD, 0x1c17ecdf);
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x01528500);
			odm_set_bb_reg(dm, R_0x91c, MASKDWORD, 0x0fa21a20);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe0f69204);

		} else if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x01528500);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe0d67234);

			if (c_channel >= 52 && c_channel <= 64) {
				odm_set_bb_reg(dm, R_0x918, MASKDWORD,
					       0x1c16ecdf);
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x0f141a20);
			} else {
				odm_set_bb_reg(dm, R_0x918, MASKDWORD,
					       0x1c16acdf);
				if (band_width == CHANNEL_WIDTH_20)
					odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
						       0x64721a20);
				else
					odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
						       0x68721a20);
			}

		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			odm_set_bb_reg(dm, R_0x918, MASKDWORD, 0x1c16acdf);
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x01528500);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe0d67231);
			if (band_width == CHANNEL_WIDTH_20)
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x64741a20);
			else
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x68741a20);

		} else {
			/* not supported */
			PHYDM_DBG(dm, DBG_DFS,
				  "Unsupported dfs_region_domain:%d\n",
				  region_domain);
			goto exit;
		}

	} else if (dm->support_ic_type &
		   (ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C)) {

		odm_set_bb_reg(dm, R_0x814, 0x3fffffff, 0x04cc4d10);
		odm_set_bb_reg(dm, R_0x834, MASKBYTE0, 0x06);

		/* @8822B only, when BW = 20M, DFIR output is 40Mhz,
		 * but DFS input is 80MMHz, so it need to upgrade to 80MHz
		 */
		if (dm->support_ic_type & (ODM_RTL8822B | ODM_RTL8821C)) {
			if (band_width == CHANNEL_WIDTH_20)
				odm_set_bb_reg(dm, R_0x1984, BIT(26), 1);
			else
				odm_set_bb_reg(dm, R_0x1984, BIT(26), 0);
		}

		if (dm->radar_detect_dbg_parm_en) {
			phydm_radar_detect_with_dbg_parm(dm);
			enable = 1;
			goto exit;
		}

		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			odm_set_bb_reg(dm, R_0x918, MASKDWORD, 0x1c16acdf);
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x095a8562);
			odm_set_bb_reg(dm, R_0x91c, MASKDWORD, 0x0fc01a24);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe0f57204);

		} else if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x095a8562);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe0d67234);

			if (c_channel >= 52 && c_channel <= 64) {
				odm_set_bb_reg(dm, R_0x918, MASKDWORD,
					       0x1c16ecdf);
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x0f141a24);
			} else {
				odm_set_bb_reg(dm, R_0x918, MASKDWORD,
					       0x1c166cdf);
				if (band_width == CHANNEL_WIDTH_20)
					odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
						       0x64721a24);
				else
					odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
						       0x68721a24);
			}

		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			odm_set_bb_reg(dm, R_0x918, MASKDWORD, 0x1c176cdf);
			odm_set_bb_reg(dm, R_0x924, MASKDWORD, 0x095a8462);
			odm_set_bb_reg(dm, R_0x920, MASKDWORD, 0xe076d231);
			if (band_width == CHANNEL_WIDTH_20)
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x64901a24);
			else
				odm_set_bb_reg(dm, R_0x91c, MASKDWORD,
					       0x62901a24);

		} else {
			/* not supported */
			PHYDM_DBG(dm, DBG_DFS,
				  "Unsupported dfs_region_domain:%d\n",
				  region_domain);
			goto exit;
		}
		/*RXHP low corner will extend the pulse width,
		 *so we need to increase the upper bound.
		 */
		if (dm->support_ic_type & (ODM_RTL8822B | ODM_RTL8821C)) {
			if (odm_get_bb_reg(dm, 0x8d8,
					   BIT28 | BIT27 | BIT26) == 0) {
				short_pw_upperbound =
					(u8)odm_get_bb_reg(dm, 0x91c,
						       BIT23 | BIT22 |
						       BIT21 | BIT20);
				if ((short_pw_upperbound + 4) > 15)
					odm_set_bb_reg(dm, 0x91c,
						       BIT23 | BIT22 |
						       BIT21 | BIT20, 15);
				else
					odm_set_bb_reg(dm, 0x91c,
						       BIT23 | BIT22 |
						       BIT21 | BIT20,
						       short_pw_upperbound + 4);
			}
			/*@if peak index -1~+1, use original NB method*/
			odm_set_bb_reg(dm, 0x19e4, 0x003C0000, 13);
			odm_set_bb_reg(dm, 0x924, 0x70000, 0);
		}

		if (dm->support_ic_type & (ODM_RTL8881A))
			odm_set_bb_reg(dm, 0xb00, 0xc0000000, 3);

		/*@for 8814 new dfs mechanism setting*/
		if (dm->support_ic_type &
		    (ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C)) {
			/*Turn off dfs scaling factor*/
			odm_set_bb_reg(dm, 0x19e4, 0x1fff, 0x0c00);
			/*NonDC peak_th = 2times DC peak_th*/
			odm_set_bb_reg(dm, 0x19e4, 0x30000, 1);
			/*power for debug and auto test flow latch after ST*/
			odm_set_bb_reg(dm, 0x9f8, 0xc0000000, 3);

			/*@low pulse width radar pattern will cause wrong drop*/
			/*@disable peak index should the same
			 *during the same short pulse (new mechan)
			 */
			odm_set_bb_reg(dm, 0x9f4, 0x80000000, 0);

			/*@disable peak index should the same
			 *during the same short pulse (old mechan)
			 */
			odm_set_bb_reg(dm, 0x924, 0x20000000, 0);

			/*@if peak index diff >=2, then drop the result*/
			odm_set_bb_reg(dm, 0x19e4, 0xe000, 2);
			if (region_domain == 2) {
				if ((c_channel >= 52) && (c_channel <= 64)) {
					/*pulse width hist th setting*/
					/*th1=2*04us*/
					odm_set_bb_reg(dm, 0x19e4,
						       0xff000000, 2);
					/*th2 = 3*0.4us, th3 = 4*0.4us
					 *th4 = 7*0.4, th5 = 34*0.4
					 */
					odm_set_bb_reg(dm, 0x19e8,
						       MASKDWORD, 0x22070403);

					/*PRI hist th setting*/
					/*th1=42*32us*/
					odm_set_bb_reg(dm, 0x19b8,
						       0x00007f80, 42);
					/*th2=47*32us, th3=115*32us,
					 *th4=123*32us, th5=130*32us
					 */
					odm_set_bb_reg(dm, 0x19ec,
						       MASKDWORD, 0x827b732f);
				} else {
					/*pulse width hist th setting*/
					/*th1=2*04us*/
					odm_set_bb_reg(dm, 0x19e4,
						       0xff000000, 1);
					/*th2 = 13*0.4us, th3 = 26*0.4us
					 *th4 = 75*0.4us, th5 = 255*0.4us
					 */
					odm_set_bb_reg(dm, 0x19e8,
						       MASKDWORD, 0xff4b1a0d);
					/*PRI hist th setting*/
					/*th1=4*32us*/

					odm_set_bb_reg(dm, 0x19b8,
						       0x00007f80, 4);
					/*th2=8*32us, th3=16*32us,
					 *th4=32*32us, th5=128*32=4096us
					 */
					odm_set_bb_reg(dm, 0x19ec,
						       MASKDWORD, 0x80201008);
				}
			}
			/*@ETSI*/
			else if (region_domain == 3) {
				/*pulse width hist th setting*/
				/*th1=2*04us*/
				odm_set_bb_reg(dm, 0x19e4, 0xff000000, 1);
				odm_set_bb_reg(dm, 0x19e8,
					       MASKDWORD, 0x68260d06);
				/*PRI hist th setting*/
				/*th1=7*32us*/
				odm_set_bb_reg(dm, 0x19b8, 0x00007f80, 7);
				/*th2=40*32us, th3=80*32us,
				 *th4=110*32us, th5=157*32=5024
				 */
				odm_set_bb_reg(dm, 0x19ec,
					       MASKDWORD, 0xc06e2010);
			}
			/*@FCC*/
			else if (region_domain == 1) {
				/*pulse width hist th setting*/
				/*th1=2*04us*/
				odm_set_bb_reg(dm, 0x19e4, 0xff000000, 2);
				/*th2 = 13*0.4us, th3 = 26*0.4us,
				 *th4 = 75*0.4us, th5 = 255*0.4us
				 */
				odm_set_bb_reg(dm, 0x19e8,
					       MASKDWORD, 0xff4b1a0d);

				/*PRI hist th setting*/
				/*th1=4*32us*/
				odm_set_bb_reg(dm, 0x19b8, 0x00007f80, 4);
				/*th2=8*32us, th3=21*32us,
				 *th4=32*32us, th5=96*32=3072
				 */
				if (band_width == CHANNEL_WIDTH_20)
					odm_set_bb_reg(dm, 0x19ec,
						       MASKDWORD, 0x60282010);
				else
					odm_set_bb_reg(dm, 0x19ec,
						       MASKDWORD, 0x60282420);
			} else {
			}
		}
	} else if (dm->support_ic_type &
		   ODM_IC_JGR3_SERIES) {
		if (dm->radar_detect_dbg_parm_en) {
			phydm_radar_detect_with_dbg_parm(dm);
			enable = 1;
			goto exit;
		}
		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb25dc0bd);
			if (dm->support_ic_type &
			    (ODM_RTL8814B | ODM_RTL8814C)) {
				if (dm->seg1_dfs_flag == 1)
					odm_set_bb_reg(dm, R_0xa6c, BIT(0), 1);
			}
			odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3030bea0);
			odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
			odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0x42533345);
			odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be0c3);
			odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500009e8);
			odm_set_bb_reg(dm, R_0x180c, 0xe0000, 0x0);
		} else if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb25dc0bd);
			if (dm->support_ic_type &
			    (ODM_RTL8814B | ODM_RTL8814C)) {
				if (dm->seg1_dfs_flag == 1)
					odm_set_bb_reg(dm, R_0xa6c, BIT(0), 1);
			}
			odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3030bea0);
			odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
			odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0x42533345);
			odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be0c3);
			odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500009e8);
			odm_set_bb_reg(dm, R_0x180c, 0xe0000, 0x0);
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			odm_set_bb_reg(dm, R_0xa40, MASKDWORD, 0xb25dc0bd);
			if (dm->support_ic_type &
			    (ODM_RTL8814B | ODM_RTL8814C)) {
				if (dm->seg1_dfs_flag == 1)
					odm_set_bb_reg(dm, R_0xa6c, BIT(0), 1);
			}
			odm_set_bb_reg(dm, R_0xa44, MASKDWORD, 0x3030bea0);
			odm_set_bb_reg(dm, R_0xa48, MASKDWORD, 0x2a521254);
			odm_set_bb_reg(dm, R_0xa4c, MASKDWORD, 0x42533345);
			odm_set_bb_reg(dm, R_0xa50, MASKDWORD, 0x605be0c3);
			odm_set_bb_reg(dm, R_0xa54, MASKDWORD, 0x500009e8);
			odm_set_bb_reg(dm, R_0x180c, 0xe0000, 0x0);
		} else {
			/* not supported */
			PHYDM_DBG(dm, DBG_DFS,
				  "Unsupported dfs_region_domain:%d\n",
				  region_domain);
			goto exit;
		}
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & ODM_RTL8721D) {
		odm_set_bb_reg(dm, R_0x814, 0x3fffffff, 0x04cc4d10);
		/*CCA MASK*/
		odm_set_bb_reg(dm, R_0xc38, 0x07c00000, 0x06);
		/*CCA Threshold*/
		odm_set_bb_reg(dm, R_0xc3c, 0x00000007, 0x0);

		if (dm->radar_detect_dbg_parm_en) {
			phydm_radar_detect_with_dbg_parm(dm);
			enable = 1;
			goto exit;
		}

		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			odm_set_bb_reg(dm, R_0xf54, MASKDWORD, 0x230006a8);
			odm_set_bb_reg(dm, R_0xf58, MASKDWORD, 0x354cd7dd);
			odm_set_bb_reg(dm, R_0xf5c, MASKDWORD, 0x9984ab25);
			odm_set_bb_reg(dm, R_0xf70, MASKDWORD, 0xbd9fab98);
			odm_set_bb_reg(dm, R_0xf74, MASKDWORD, 0xcc45029f);

		} else if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			odm_set_bb_reg(dm, R_0xf54, MASKDWORD, 0x230006a8);
			odm_set_bb_reg(dm, R_0xf5c, MASKDWORD, 0x9984ab25);
			odm_set_bb_reg(dm, R_0xf70, MASKDWORD, 0xbd9fb398);
			odm_set_bb_reg(dm, R_0xf74, MASKDWORD, 0xcc450e9d);

			if (c_channel >= 52 && c_channel <= 64) {
				odm_set_bb_reg(dm, R_0xf58, MASKDWORD,
					       0x354cd7fd);
			} else {
				odm_set_bb_reg(dm, R_0xf58, MASKDWORD,
					       0x354cd7bd);
			}
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			odm_set_bb_reg(dm, R_0xf54, MASKDWORD, 0x230006a8);
			odm_set_bb_reg(dm, R_0xf58, MASKDWORD, 0x3558d7bd);
			odm_set_bb_reg(dm, R_0xf5c, MASKDWORD, 0x9984ab35);
			odm_set_bb_reg(dm, R_0xf70, MASKDWORD, 0xbd9fb398);
			odm_set_bb_reg(dm, R_0xf74, MASKDWORD, 0xcc444e9d);
		} else {
			/* not supported */
			PHYDM_DBG(dm, DBG_DFS,
				  "Unsupported dfs_region_domain:%d\n",
				  region_domain);
			goto exit;
		}

		/*if peak index -1~+1, use original NB method*/
		odm_set_bb_reg(dm, R_0xf70, 0x00070000, 0x7);
		odm_set_bb_reg(dm, R_0xf74, 0x000c0000, 0);

		/*Turn off dfs scaling factor*/
		odm_set_bb_reg(dm, R_0xf70, 0x00080000, 0x0);
		/*NonDC peak_th = 2times DC peak_th*/
		odm_set_bb_reg(dm, R_0xf58, 0x00007800, 1);

		/*low pulse width radar pattern will cause wrong drop*/
		/*disable peak index should the same*/
		/*during the same short pulse (new mechan)*/
		odm_set_bb_reg(dm, R_0xf70, 0x00100000, 0x0);
		/*if peak index diff >=2, then drop the result*/
		odm_set_bb_reg(dm, R_0xf70, 0x30000000, 0x2);
	#endif
	} else {
		/*not supported IC type*/
		PHYDM_DBG(dm, DBG_DFS, "Unsupported IC type:%d\n",
			  dm->support_ic_type);
		goto exit;
	}

	enable = 1;

	if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00007f00);
		dfs->pwdb_th_cur = (u8)odm_get_bb_reg(dm, R_0xa50, 0x000000f0);
		dfs->peak_th = (u8)odm_get_bb_reg(dm, R_0xa48, 0x00c00000);
		dfs->short_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xa50,
							     0x00f00000);
		dfs->long_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xa4c,
							    0xf0000000);
		dfs->peak_window = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00030000);
		dfs->three_peak_opt = (u8)odm_get_bb_reg(dm, R_0xa40,
							 0x30000000);
		dfs->three_peak_th2 = (u8)odm_get_bb_reg(dm, R_0xa44,
							 0x00000007);
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & (ODM_RTL8721D)) {
		dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0xf54,
						     0x0000001f) << 2);
		dfs->st_l2h_cur += (u8)odm_get_bb_reg(dm, R_0xf58, 0xc0000000);
		dfs->pwdb_th_cur = (u8)odm_get_bb_reg(dm, R_0xf70, 0x03c00000);
		dfs->peak_th = (u8)odm_get_bb_reg(dm, R_0xf5c, 0x00000030);
		dfs->short_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xf70,
							     0x00007800);
		dfs->long_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0xf74,
							    0x0000000f);
		dfs->peak_window = (u8)odm_get_bb_reg(dm, R_0xf58, 0x18000000);
		dfs->three_peak_opt = (u8)odm_get_bb_reg(dm, R_0xf58,
							 0x00030000);
		dfs->three_peak_th2 = (u8)odm_get_bb_reg(dm,
							 R_0xf58, 0x00007c00);
	#endif
	} else {
		dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0x91c, 0x000000ff);
		dfs->pwdb_th_cur = (u8)odm_get_bb_reg(dm, R_0x918, 0x00001f00);
		dfs->peak_th = (u8)odm_get_bb_reg(dm, R_0x918, 0x00030000);
		dfs->short_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0x920,
							     0x000f0000);
		dfs->long_pulse_cnt_th = (u8)odm_get_bb_reg(dm, R_0x920,
							    0x00f00000);
		dfs->peak_window = (u8)odm_get_bb_reg(dm, R_0x920, 0x00000300);
		dfs->three_peak_opt = (u8)odm_get_bb_reg(dm, 0x924, 0x00000180);
		dfs->three_peak_th2 = (u8)odm_get_bb_reg(dm, 0x924, 0x00007000);
	}

		phydm_dfs_parameter_init(dm);

exit:
	if (enable) {
		phydm_radar_detect_reset(dm);
		PHYDM_DBG(dm, DBG_DFS, "on cch:%u, bw:%u\n", c_channel,
			  band_width);
	} else
		phydm_radar_detect_disable(dm);
}

void phydm_dfs_parameter_init(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	u8 i;
	for (i = 0; i < 5; i++) {
		dfs->pulse_flag_hist[i] = 0;
		dfs->pulse_type_hist[i] = 0;
		dfs->radar_det_mask_hist[i] = 0;
		dfs->fa_inc_hist[i] = 0;
	}

	/*@for dfs mode*/
	dfs->dbg_mode = 0;
	dfs->force_TP_mode = 0;
	dfs->sw_trigger_mode = 0;
	dfs->det_print = 0;
	dfs->det_print2 = 0;   // HW detect print in n-series
	dfs->det_print_sw = 0; // SW detect print
	dfs->print_hist_rpt = 0;
	if (dm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C))
		dfs->hist_cond_on = 1;
	else
		dfs->hist_cond_on = 0;
	if (dm->support_ic_type & (ODM_RTL8733B | ODM_RTL8822C | ODM_RTL8822E)) {
		dfs->det_print_jar3 = 1;
		dfs->nhm_dfs_en = true;
		dfs->det_jar3_en = true;
	} else {
		dfs->det_print_jar3 = 0;
		dfs->nhm_dfs_en = false;
		dfs->det_jar3_en = false;
	}

	/*@for flag en*/
	dfs->pw_flag_en = true;
	dfs->pri_flag_en = true;
	dfs->loct_flag_en = true;
	dfs->nhm_cnt_th = 4;
	dfs->nhm_dty_th = 40;
	dfs->fa_cnt_th = 3000;
	dfs->fa_cnt_th_backup = dfs->fa_cnt_th;

	/*@for jgar3*/
	dfs->pri_var_offset = 2;
	dfs->pri_var_tp_offset = 10;
	dfs->pri_cnt_th = 2;
	dfs->loct_diff_th = 15;
	dfs->dc_cnt_th = 3;

	/*@for dynamic dfs*/
	dfs->pwdb_th = 8;
	dfs->fa_mask_th = 120 * dfs->dfs_polling_time / 100;
	dfs->st_l2h_min = 0x20;
	dfs->st_l2h_max = 0x4e;
	dfs->pwdb_scalar_factor = 12;

	/*@for dfs histogram*/
	dfs->pri_hist_idle_th = 4;
	dfs->pri_hist_th = 2;   // due to ESTI type1 in TP (may set to 3 to prevent FRD)
	dfs->pri_sum_g1_th = 0;
	dfs->pri_sum_g0_idle_th = 0;
	dfs->pri_sum_g0_th = 0;
	dfs->pri_sum_g5_idle_th = 0;
	dfs->pri_sum_g5_th = 2;
	dfs->pw_sum_g0_idle_th = 10;
	dfs->pw_sum_g0_th = 20;
	dfs->pw_sum_g5_idle_th = 0;
	dfs->pw_sum_g5_th = 0;
	dfs->pw_pri_valid_set_idle_th = 3;
	dfs->pw_pri_valid_set_th = 5;
	dfs->pri_sum_g1_fcc_th = 4;		/*@FCC Type6*/
	dfs->pri_sum_g3_fcc_th = 6;
	dfs->pri_sum_safe_th = 50;
	dfs->pri_sum_safe_fcc_th = 110;		/*@30 for AP*/
	dfs->pri_sum_type4_th = 16;
	dfs->pri_sum_type6_th = 12;
	dfs->pri_sum_g5_under_g1_th = 4;
	dfs->pri_pw_diff_idle_th = 6;
	dfs->pri_pw_diff_th = 10;
	dfs->pri_pw_diff_fcc_th = 8;
	dfs->pri_pw_diff_fcc_idle_th = 2;
	dfs->pri_pw_diff_w53_th = 10;
	dfs->pw_std_th = 7;			/*@FCC Type4*/
	dfs->pw_std_idle_th = 10;
	dfs->pri_std_th = 6;			/*@FCC Type3,4,6*/
	dfs->pri_std_idle_th = 10;
	dfs->pri_type1_upp_fcc_th = 110;
	dfs->pri_type1_low_fcc_th = 50;
	dfs->pri_type1_cen_fcc_th = 70;
	dfs->pw_g0_th = 8;
	dfs->pw_long_lower_th = 6;		/*@7->6*/
	dfs->pri_long_upper_th = 30;
	dfs->pw_long_lower_20m_th = 7;		/*@7 for AP*/
	dfs->pw_long_sum_upper_th = 60;
	dfs->type4_pw_max_cnt = 7;
	dfs->type4_safe_pri_sum_th = 5;
}

void phydm_dfs_dynamic_setting(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	u8 peak_th_cur = 0, short_pulse_cnt_th_cur = 0;
	u8 long_pulse_cnt_th_cur = 0, three_peak_opt_cur = 0;
	u8 three_peak_th2_cur = 0;
	u8 peak_window_cur = 0;
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;
	u8 band_width = *dm->band_width;
	u32 fa_cnt_th = 0;

	if (dm->rx_tp + dm->tx_tp <= 2) {
		dfs->idle_mode = 1;
		if (dfs->force_TP_mode)
			dfs->idle_mode = 0;
	} else {
		dfs->idle_mode = 0;
	}

	if (dfs->idle_mode == 1) { /*@idle (no traffic)*/
		fa_cnt_th = 110;
		peak_th_cur = 3;
		short_pulse_cnt_th_cur = 6;
		long_pulse_cnt_th_cur = 9;
		peak_window_cur = 2;
		three_peak_opt_cur = 0;
		three_peak_th2_cur = 2;
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				short_pulse_cnt_th_cur = 14;
				long_pulse_cnt_th_cur = 15;
				three_peak_th2_cur = 0;
			} else {
				short_pulse_cnt_th_cur = 6;
				three_peak_th2_cur = 0;
				long_pulse_cnt_th_cur = 10;
			}
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			three_peak_th2_cur = 0;
		} else if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			long_pulse_cnt_th_cur = 15;
			if (phydm_dfs_is_meteorology_channel(dm)) {
			/*need to add check cac end condition*/
				peak_th_cur = 2;
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				short_pulse_cnt_th_cur = 7;
			} else {
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				short_pulse_cnt_th_cur = 7;
			}
		} else /*@default: FCC*/
			three_peak_th2_cur = 0;

	} else { /*@in service (with TP)*/
		if (band_width == CHANNEL_WIDTH_40)
			fa_cnt_th = 500;
		else if (band_width == CHANNEL_WIDTH_80)
			fa_cnt_th = 1500;
		else
			fa_cnt_th = 110;
		peak_th_cur = 2;
		short_pulse_cnt_th_cur = 6;
		long_pulse_cnt_th_cur = 7;
		peak_window_cur = 2;
		three_peak_opt_cur = 0;
		three_peak_th2_cur = 2;
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				long_pulse_cnt_th_cur = 15;
				/*@for high duty cycle*/
				short_pulse_cnt_th_cur = 5;
				three_peak_th2_cur = 0;
			} else {
				three_peak_opt_cur = 0;
				three_peak_th2_cur = 0;
				long_pulse_cnt_th_cur = 8;
			}
		} else if (region_domain == PHYDM_DFS_DOMAIN_FCC) {
			long_pulse_cnt_th_cur = 5;	/*for 80M FCC*/
			short_pulse_cnt_th_cur = 5;	/*for 80M FCC*/
		} else if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			long_pulse_cnt_th_cur = 15;
			short_pulse_cnt_th_cur = 3;
			three_peak_opt_cur = 0;
		}
	}

	if (dm->phydm_sys_up_time <= 62)  // in CAC
		fa_cnt_th = 3000;

	if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		/*
		if (dfs->peak_th != peak_th_cur)
			odm_set_bb_reg(dm, R_0xa48, 0x00c00000, peak_th_cur);
		if (dfs->short_pulse_cnt_th != short_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0xa50, 0x00f00000,
				       short_pulse_cnt_th_cur);
		if (dfs->long_pulse_cnt_th != long_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0xa4c, 0xf0000000,
				       long_pulse_cnt_th_cur);
		if (dfs->peak_window != peak_window_cur)
			odm_set_bb_reg(dm, R_0xa40, 0x00030000,
				       peak_window_cur);
		if (dfs->three_peak_opt != three_peak_opt_cur)
			odm_set_bb_reg(dm, R_0xa40, 0x30000000,
				       three_peak_opt_cur);
		if (dfs->three_peak_th2 != three_peak_th2_cur)
			odm_set_bb_reg(dm, R_0xa44, 0x00000007,
				       three_peak_th2_cur);
		*/
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & (ODM_RTL8721D)) {
		if (dfs->peak_th != peak_th_cur)
			odm_set_bb_reg(dm, R_0xf5c, 0x00000030, peak_th_cur);
		if (dfs->short_pulse_cnt_th != short_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0xf70, 0x00007800,
				       short_pulse_cnt_th_cur);
		if (dfs->long_pulse_cnt_th != long_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0xf74, 0x0000000f,
				       long_pulse_cnt_th_cur);
		if (dfs->peak_window != peak_window_cur)
			odm_set_bb_reg(dm, R_0xf58, 0x18000000,
				       peak_window_cur);
		if (dfs->three_peak_opt != three_peak_opt_cur)
			odm_set_bb_reg(dm, R_0xf58, 0x00030000,
				       three_peak_opt_cur);
		if (dfs->three_peak_th2 != three_peak_th2_cur)
			odm_set_bb_reg(dm, R_0xf58, 0x00007c00,
				       three_peak_th2_cur);
	#endif
	} else {
		if (dfs->peak_th != peak_th_cur)
			odm_set_bb_reg(dm, R_0x918, 0x00030000, peak_th_cur);
		if (dfs->short_pulse_cnt_th != short_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0x920, 0x000f0000,
				       short_pulse_cnt_th_cur);
		if (dfs->long_pulse_cnt_th != long_pulse_cnt_th_cur)
			odm_set_bb_reg(dm, R_0x920, 0x00f00000,
				       long_pulse_cnt_th_cur);
		if (dfs->peak_window != peak_window_cur)
			odm_set_bb_reg(dm, R_0x920, 0x00000300,
				       peak_window_cur);
		if (dfs->three_peak_opt != three_peak_opt_cur)
			odm_set_bb_reg(dm, R_0x924, 0x00000180,
				       three_peak_opt_cur);
		if (dfs->three_peak_th2 != three_peak_th2_cur)
			odm_set_bb_reg(dm, R_0x924, 0x00007000,
				       three_peak_th2_cur);
	}

	if (dfs->fa_cnt_th_backup != fa_cnt_th)
		dfs->fa_cnt_th = fa_cnt_th;

	dfs->fa_cnt_th_backup = fa_cnt_th;
	dfs->peak_th = peak_th_cur;
	dfs->short_pulse_cnt_th = short_pulse_cnt_th_cur;
	dfs->long_pulse_cnt_th = long_pulse_cnt_th_cur;
	dfs->peak_window = peak_window_cur;
	dfs->three_peak_opt = three_peak_opt_cur;
	dfs->three_peak_th2 = three_peak_th2_cur;
}

boolean
phydm_radar_detect_dm_check(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	struct phydm_fa_struct *fa_t = &dm->false_alm_cnt;
	u8 region_domain = dm->dfs_region_domain, index = 0;

	u16 i = 0, j = 0, fa_count_cur = 0, fa_count_inc = 0;
	u16 total_fa_in_hist = 0, total_pulse_count_inc = 0;
	u16 short_pulse_cnt_inc = 0, short_pulse_cnt_cur = 0;
	u16 long_pulse_cnt_inc = 0, long_pulse_cnt_cur = 0;
	u32 regf98_value = 0, reg918_value = 0, reg91c_value = 0;
	u32 reg920_value = 0, reg924_value = 0, radar_rpt_reg_value = 0;
	u32 regf54_value = 0, regf58_value = 0, regf5c_value = 0;
	u32 regdf4_value = 0, regf70_value = 0, regf74_value = 0;
	#if (RTL8812F_SUPPORT || RTL8822C_SUPPORT || RTL8814B_SUPPORT || RTL8733B_SUPPORT ||RTL8822E_SUPPORT)
	u32 rega40_value = 0, rega44_value = 0, rega48_value = 0;
	u32 rega4c_value = 0, rega50_value = 0, rega54_value = 0;
	u32 reg_2e08 = 0, reg_2e24 = 0, reg_2e28 = 0;
	u32 reg_2e2c = 0, reg_2e30 = 0, reg_2e34 = 0;
	u32 reg_2e0c = 0, reg_2e10 = 0, reg_2e20 = 0;
	#endif
	#if (RTL8721D_SUPPORT)
	u32 reg908_value = 0, regdf4_value = 0;
	u32 regf54_value = 0, regf58_value = 0, regf5c_value = 0;
	u32 regf70_value = 0, regf74_value = 0;
	#endif
	boolean tri_short_pulse = 0, tri_long_pulse = 0, radar_type = 0;
	boolean fault_flag_det = 0, fault_flag_psd = 0, fa_flag = 0;
	boolean radar_detected = 0;
	u8 st_l2h_new = 0, fa_mask_th = 0, k = 0, sum = 0;
	u8 c_channel = *dm->channel;

	/*@Get FA count during past 100ms, R_0xf48 for AC series*/
	if (dm->support_ic_type & ODM_IC_JGR3_SERIES)
		/*fa_count_cur = (u16)odm_get_bb_reg(dm, R_0x2d00, MASKLWORD);*/
		/*1-sec period*/
		fa_count_cur = (u16)(fa_t->cnt_all * dfs->dfs_polling_time / 1000);
	#if (RTL8721D_SUPPORT)
	else if (dm->support_ic_type & (ODM_RTL8721D)) {
		fa_count_cur = (u16)odm_get_bb_reg(dm,
						   ODM_REG_OFDM_FA_TYPE2_11N,
						   MASKHWORD);
		fa_count_cur += (u16)odm_get_bb_reg(dm,
						    ODM_REG_OFDM_FA_TYPE3_11N,
						    MASKLWORD);
		fa_count_cur += (u16)odm_get_bb_reg(dm,
						    ODM_REG_OFDM_FA_TYPE3_11N,
						    MASKHWORD);
		fa_count_cur += (u16)odm_get_bb_reg(dm,
						    ODM_REG_OFDM_FA_TYPE4_11N,
						    MASKLWORD);
		fa_count_cur += (u16)odm_get_bb_reg(dm,
						    ODM_REG_OFDM_FA_TYPE1_11N,
						    MASKLWORD);
		fa_count_cur += (u16)odm_get_bb_reg(dm,
						    ODM_REG_OFDM_FA_TYPE1_11N,
						    MASKHWORD);
	}
	#endif
	else
		fa_count_cur = (u16)odm_get_bb_reg(dm, R_0xf48, 0x0000ffff);

	if (!(dm->support_ic_type & ODM_IC_JGR3_SERIES)) {
		if (dfs->fa_count_pre == 0)
			fa_count_inc = 0;
		else if (fa_count_cur >= dfs->fa_count_pre)
			fa_count_inc = fa_count_cur - dfs->fa_count_pre;
		else
			fa_count_inc = fa_count_cur;
		dfs->fa_count_pre = fa_count_cur;
	} else {
		fa_count_inc = fa_count_cur;
	}

	for (i = 0; i < 5; i++)
		total_fa_in_hist = total_fa_in_hist + dfs->fa_inc_hist[i];

	if (dfs->mask_idx >= 2)
		index = dfs->mask_idx - 2;
	else
		index = 5 + dfs->mask_idx - 2;

	if (dm->support_ic_type & (ODM_RTL8198F | ODM_RTL8822C | ODM_RTL8812F |\
				   ODM_RTL8197G| ODM_RTL8733B | ODM_RTL8735B |\
				   ODM_RTL8730A | ODM_RTL8822E)) {
		radar_rpt_reg_value = odm_get_bb_reg(dm, R_0x2e00, 0xffffffff);
		short_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x000ff800)
					    >> 11);
		long_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x0fc00000)
					    >> 22);
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & (ODM_RTL8721D)) {
		reg908_value = (u32)odm_get_bb_reg(dm, R_0x908, MASKDWORD);
		odm_set_bb_reg(dm, R_0x908, MASKDWORD, 0x254);
		regdf4_value = odm_get_bb_reg(dm, R_0xdf4, MASKDWORD);
		short_pulse_cnt_cur = (u16)((regdf4_value & 0x000ff000) >> 12);
		long_pulse_cnt_cur = (u16)((regdf4_value & 0x0fc00000) >> 22);

		tri_short_pulse = (regdf4_value & BIT(20)) ? 1 : 0;
		tri_long_pulse = (regdf4_value & BIT(28)) ? 1 : 0;
		if (tri_short_pulse || tri_long_pulse) {
			odm_set_bb_reg(dm, R_0xf58, BIT(29), 0);
			odm_set_bb_reg(dm, R_0xf58, BIT(29), 1);
		}
	#endif
	} else if (dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C)) {
		if (dm->seg1_dfs_flag == 1)
			radar_rpt_reg_value = odm_get_bb_reg(dm, R_0x2e20,
							     0xffffffff);
		else
			radar_rpt_reg_value = odm_get_bb_reg(dm, R_0x2e00,
							     0xffffffff);
		short_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x000ff800)
					    >> 11);
		long_pulse_cnt_cur = (u16)((radar_rpt_reg_value & 0x0fc00000)
					    >> 22);
	} else {
		regf98_value = odm_get_bb_reg(dm, R_0xf98, 0xffffffff);
		short_pulse_cnt_cur = (u16)(regf98_value & 0x000000ff);
		long_pulse_cnt_cur = (u16)((regf98_value & 0x0000ff00) >> 8);
	}

	/*@Get short pulse count, need carefully handle the counter overflow*/
	if (short_pulse_cnt_cur >= dfs->short_pulse_cnt_pre) {
		short_pulse_cnt_inc = short_pulse_cnt_cur -
				      dfs->short_pulse_cnt_pre;
	} else {
		short_pulse_cnt_inc = short_pulse_cnt_cur;
	}
	dfs->short_pulse_cnt_pre = short_pulse_cnt_cur;

	/*@Get long pulse count, need carefully handle the counter overflow*/
	if (long_pulse_cnt_cur >= dfs->long_pulse_cnt_pre) {
		long_pulse_cnt_inc = long_pulse_cnt_cur -
				     dfs->long_pulse_cnt_pre;
	} else {
		long_pulse_cnt_inc = long_pulse_cnt_cur;
	}
	dfs->long_pulse_cnt_pre = long_pulse_cnt_cur;

	total_pulse_count_inc = short_pulse_cnt_inc + long_pulse_cnt_inc;
	//PHYDM_DBG(dm, DBG_DFS, "total_pulse_count_inc=%d\n", total_pulse_count_inc);

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS,
			  "===============================================\n");
		PHYDM_DBG(dm, DBG_DFS, "FA_count_inc[%d]\n", fa_count_inc);
		if (dm->support_ic_type & (ODM_RTL8721D)) {
			PHYDM_DBG(dm, DBG_DFS,
				  "Init_Gain[%x] st_l2h_cur[%x] 0xdf4[%08x] short_pulse_cnt_inc[%d] long_pulse_cnt_inc[%d]\n",
				  dfs->igi_cur, dfs->st_l2h_cur, regdf4_value,
				  short_pulse_cnt_inc, long_pulse_cnt_inc);
			regf54_value = odm_get_bb_reg(dm, R_0xf54, MASKDWORD);
			regf58_value = odm_get_bb_reg(dm, R_0xf58, MASKDWORD);
			regf5c_value = odm_get_bb_reg(dm, R_0xf5c, MASKDWORD);
			regf70_value = odm_get_bb_reg(dm, R_0xf70, MASKDWORD);
			regf74_value = odm_get_bb_reg(dm, R_0xf74, MASKDWORD);
			PHYDM_DBG(dm, DBG_DFS,
				  "0xf54[%08x] 0xf58[%08x] 0xf5c[%08x] 0xf70[%08x] 0xf74[%08x]\n",
				  regf54_value, regf58_value, regf5c_value,
				  regf70_value, regf74_value);
		} else if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
			PHYDM_DBG(dm, DBG_DFS,
				  "Init_Gain[%x] st_l2h_cur[%x] 0x2dbc[%08x] short_pulse_cnt_inc[%d] long_pulse_cnt_inc[%d]\n",
				  dfs->igi_cur, dfs->st_l2h_cur,
				  radar_rpt_reg_value, short_pulse_cnt_inc,
				  long_pulse_cnt_inc);
		#if (RTL8812F_SUPPORT || RTL8822C_SUPPORT || RTL8814B_SUPPORT ||\
			RTL8822E_SUPPORT)
			rega40_value = odm_get_bb_reg(dm, R_0xa40, MASKDWORD);
			rega44_value = odm_get_bb_reg(dm, R_0xa44, MASKDWORD);
			rega48_value = odm_get_bb_reg(dm, R_0xa48, MASKDWORD);
			rega4c_value = odm_get_bb_reg(dm, R_0xa4c, MASKDWORD);
			rega50_value = odm_get_bb_reg(dm, R_0xa50, MASKDWORD);
			rega54_value = odm_get_bb_reg(dm, R_0xa54, MASKDWORD);
			PHYDM_DBG(dm, DBG_DFS,
				  "0xa40[%08x] 0xa44[%08x] 0xa48[%08x] 0xa4c[%08x] 0xa50[%08x] 0xa54[%08x]\n",
				  rega40_value, rega44_value, rega48_value,
				  rega4c_value, rega50_value, rega54_value);
		#endif
		} else {
			PHYDM_DBG(dm, DBG_DFS,
				  "Init_Gain[%x] 0x91c[%x] 0xf98[%08x] short_pulse_cnt_inc[%d] long_pulse_cnt_inc[%d]\n",
				  dfs->igi_cur, dfs->st_l2h_cur, regf98_value,
				  short_pulse_cnt_inc, long_pulse_cnt_inc);
			reg918_value = odm_get_bb_reg(dm, R_0x918,
						      0xffffffff);
			reg91c_value = odm_get_bb_reg(dm, R_0x91c,
						      0xffffffff);
			reg920_value = odm_get_bb_reg(dm, R_0x920,
						      0xffffffff);
			reg924_value = odm_get_bb_reg(dm, R_0x924,
						      0xffffffff);
			PHYDM_DBG(dm, DBG_DFS,
				  "0x918[%08x] 0x91c[%08x] 0x920[%08x] 0x924[%08x]\n",
				  reg918_value, reg91c_value,
				  reg920_value, reg924_value);
		}
		PHYDM_DBG(dm, DBG_DFS, "Throughput: %dMbps\n",
			  (dm->rx_tp + dm->tx_tp));

		PHYDM_DBG(dm, DBG_DFS,
			  "dfs_regdomain = %d, dbg_mode = %d, idle_mode = %d, print_hist_rpt = %d, hist_cond_on = %d\n",
			  region_domain, dfs->dbg_mode,
			  dfs->idle_mode, dfs->print_hist_rpt,
			  dfs->hist_cond_on);
	}
	if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		tri_short_pulse = (radar_rpt_reg_value & BIT(20)) ? 1 : 0;
		tri_long_pulse = (radar_rpt_reg_value & BIT(28)) ? 1 : 0;
	} else {
		tri_short_pulse = (regf98_value & BIT(17)) ? 1 : 0;
		tri_long_pulse = (regf98_value & BIT(19)) ? 1 : 0;
	}

	if (tri_long_pulse){
		if (dfs->det_print2 ||dfs->det_print_jar3) {
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "tri_long_pulse = %d\n", tri_long_pulse);
		}
		dfs->pulse_flag_hist[dfs->mask_idx] = 1;
		dfs->pulse_type_hist[dfs->mask_idx] = 1;
		}
	
	else if (tri_short_pulse){
		if (dfs->det_print2 ||dfs->det_print_jar3) {
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "tri_short_pulse = %d\n", tri_short_pulse);
		}
		dfs->pulse_flag_hist[dfs->mask_idx] = 1;
		dfs->pulse_type_hist[dfs->mask_idx] = 0;
		}
	
	else{
		dfs->pulse_flag_hist[dfs->mask_idx] = 0;
		dfs->pulse_type_hist[dfs->mask_idx] = 0;
		}

	
	
	if (tri_short_pulse) {
		if (!(dm->support_ic_type & ODM_IC_JGR3_SERIES))
			phydm_radar_detect_reset(dm);
	}
	if (tri_long_pulse) {
		/*
		if (!(dm->support_ic_type & ODM_IC_JGR3_SERIES))
			phydm_radar_detect_reset(dm);
		*/
		if (region_domain == PHYDM_DFS_DOMAIN_MKK) {
			if (c_channel >= 52 && c_channel <= 64) {
				tri_long_pulse = 0;
			}
		}
		if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
			tri_long_pulse = 0; 
		}
	}

	st_l2h_new = dfs->st_l2h_cur;
#if (RTL8733B_SUPPORT||RTL8822C_SUPPORT || RTL8822E_SUPPORT)
	if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		if (dfs->pulse_type_hist[dfs->mask_idx])
			dfs->radar_type = 1;
		else
			dfs->radar_type = 0;

		if (dfs->pulse_flag_hist[dfs->mask_idx]) {
			if (dfs->det_jar3_en)
				phydm_dfs_rpt_distinguish(dm);
			phydm_radar_detect_reset(dm);
		}
	}
#endif

	/* PSD(not ready) */

	fault_flag_det = 0;
	fault_flag_psd = 0;
	fa_flag = 0;
	if (region_domain == PHYDM_DFS_DOMAIN_ETSI) {
		fa_mask_th = dfs->fa_mask_th + 20;
	} else {
		fa_mask_th = dfs->fa_mask_th;
	}
	if (total_fa_in_hist >= fa_mask_th || dfs->igi_cur >= 0x30) {
		/* st_l2h_new = dfs->st_l2h_max; */
		dfs->radar_det_mask_hist[index] = 1;
		if (dfs->pulse_flag_hist[index] == 1) {
			dfs->pulse_flag_hist[index] = 0;
			if (dfs->det_print2) {
				PHYDM_DBG(dm, DBG_DFS,
					  "Radar is masked : FA mask\n");
			}
		}
		fa_flag = 1;
	} else {
		dfs->radar_det_mask_hist[index] = 0;
	}

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS, "mask_idx: %d\n", dfs->mask_idx);
		PHYDM_DBG(dm, DBG_DFS, "radar_det_mask_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ",
				  dfs->radar_det_mask_hist[i]);
		PHYDM_DBG(dm, DBG_DFS, "pulse_flag_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ", dfs->pulse_flag_hist[i]);
		PHYDM_DBG(dm, DBG_DFS, "fa_inc_hist: ");
		for (i = 0; i < 5; i++)
			PHYDM_DBG(dm, DBG_DFS, "%d ", dfs->fa_inc_hist[i]);
		PHYDM_DBG(dm, DBG_DFS,
			  "\nfa_mask_th: %d, total_fa_in_hist: %d ",
			  fa_mask_th, total_fa_in_hist);
	}

	sum = 0;
	for (k = 0; k < 5; k++) {
		if (dfs->radar_det_mask_hist[k] == 1)
			sum++;
	}

	if (dfs->mask_hist_checked <= 5)
		dfs->mask_hist_checked++;

	if (dfs->mask_hist_checked >= 5 && dfs->pulse_flag_hist[index]) {
		if (sum <= 2) {
			if (dfs->hist_cond_on) {
				/*return the value from hist_radar_detected*/
				radar_detected = phydm_dfs_hist_log(dm, index);
			} else if (dfs->det_jar3_en) {
				if (dfs->nhm_dfs_flag || dfs->b_flag) {
					radar_detected = false;
				} else if (dfs->pw_flag && dfs->pri_flag &&
					   dfs->loct_flag) {
					PHYDM_DBG(dm, DBG_DFS, "Detected type %d radar signal!\n",
						  dfs->radar_type);
					radar_detected = true;
				}
			} else {
				if (dfs->pulse_type_hist[dfs->mask_idx] == 0)
					dfs->radar_type = 0;
				else if (dfs->pulse_type_hist[dfs->mask_idx] == 1)
					dfs->radar_type = 1;
				radar_detected = true;
				PHYDM_DBG(dm, DBG_DFS,
					  "Detected type %d radar signal!\n",
					  dfs->radar_type);
			}
		} else {
			fault_flag_det = 1;
			if (dfs->det_print2) {
				PHYDM_DBG(dm, DBG_DFS,
					  "Radar is masked : mask_hist large than thd\n");
			}
		}
	}

	dfs->mask_idx++;
	if (dfs->mask_idx == 5)
		dfs->mask_idx = 0;

	if (fault_flag_det == 0 && fault_flag_psd == 0 && fa_flag == 0) {
		if (dfs->igi_cur < 0x30) {
			st_l2h_new = dfs->st_l2h_min;
		}
	}

	if (st_l2h_new != dfs->st_l2h_cur) {
		if (st_l2h_new < dfs->st_l2h_min) {
			dfs->st_l2h_cur = dfs->st_l2h_min;
		} else if (st_l2h_new > dfs->st_l2h_max)
			dfs->st_l2h_cur = dfs->st_l2h_max;
		else
			dfs->st_l2h_cur = st_l2h_new;
		/*odm_set_bb_reg(dm, R_0x91c, 0xff, dfs->st_l2h_cur);*/

		dfs->pwdb_th_cur = ((int)dfs->st_l2h_cur - (int)dfs->igi_cur)
				    / 2 + dfs->pwdb_scalar_factor;

		/*@limit the pwdb value to absolute lower bound 8*/
		dfs->pwdb_th_cur = MAX_2(dfs->pwdb_th_cur, (int)dfs->pwdb_th);

		/*@limit the pwdb value to absolute upper bound 0x1f*/
		dfs->pwdb_th_cur = MIN_2(dfs->pwdb_th_cur, 0x1f);

		if (dm->support_ic_type & ODM_IC_JGR3_SERIES)
			odm_set_bb_reg(dm, R_0xa50, 0x000000f0,
				       dfs->pwdb_th_cur);
		#if (RTL8721D_SUPPORT)
		else if (dm->support_ic_type & ODM_RTL8721D) {
			odm_set_bb_reg(dm, R_0xf54, 0x0000001f,
				       ((dfs->st_l2h_cur & 0x0000007c) >> 2));
			odm_set_bb_reg(dm, R_0xf58, 0xc0000000,
				       (dfs->st_l2h_cur & 0x00000003));
			odm_set_bb_reg(dm, R_0xf70, 0x03c00000,
				       dfs->pwdb_th_cur);
		}
		#endif
		else
			odm_set_bb_reg(dm, R_0x918, 0x00001f00,
				       dfs->pwdb_th_cur);
	}

	if (dfs->det_print) {
		PHYDM_DBG(dm, DBG_DFS,
			  "fault_flag_det[%d], fault_flag_psd[%d], DFS_detected [%d]\n",
			  fault_flag_det, fault_flag_psd, radar_detected);
	}
	#if (RTL8721D_SUPPORT)
	if (dm->support_ic_type & (ODM_RTL8721D))
		odm_set_bb_reg(dm, R_0x908, MASKDWORD, reg908_value);
	#endif

	return radar_detected;
}
#if (RTL8733B_SUPPORT || RTL8822C_SUPPORT || RTL8822E_SUPPORT)
void phydm_dfs_rpt_distinguish(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	struct ccx_info *ccx = &dm->dm_ccx_info;
	
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;
	u8 band_width = *dm->band_width;
	
	u32 reg_2e08, reg_2e24, reg_2e28, reg_2e2c, reg_2e30, reg_2e34;
	u32 reg_2e0c, reg_2e10, reg_2e20;
	u16 pw_rpt_set[12] = {0}, pri_rpt_set[12] = {0};
	s8 loct_rpt_set[12] = {0};
	u8 pw_lth[10] = {255,255,255,255,255,255,255,255,255,255};
       u16 pw_uth[10] = {255,255,255,255,255,255,255,255,255,255};
       u8 pri_lth[10] = {255,255,255,255,255,255,255,255,255,255};
       u8 pri_uth[10] = {255,255,255,255,255,255,255,255,255,255};
	u8 pri_vd_max = 0, pri_vd_min = 255, rdr_tmp = 0;
	u8 i, j, dc_cnt = 0, rdr_num = 0, rdr_cnt[10] = {0};
	u8 short_cnt_th, long_cnt_th, loop_cnt, cnt_th, loct_diff = 0;
	u8 pri_var_up, pri_var_low, pri_cnt_tmp, pri_cnt_fnl = 0;
	u8 pri_cnt_th = dfs->pri_cnt_th;

	/* PW : Wordlength : U(8,0) , unit: 400ns */
	reg_2e08 = odm_get_bb_reg(dm, 0x2e08, 0xffffffff);
	pw_rpt_set[0]=(unsigned int)(reg_2e08 & 0x000000ff);
	pw_rpt_set[1]=(unsigned int)((reg_2e08 & 0x0000ff00)>>8);
	pw_rpt_set[2]=(unsigned int)((reg_2e08 & 0x00ff0000)>>16);
	pw_rpt_set[3]=(unsigned int)((reg_2e08 & 0xff000000)>>24);
	reg_2e24 = odm_get_bb_reg(dm, 0x2e24, 0xffffffff);
	pw_rpt_set[4]=(unsigned int)(reg_2e24 & 0x000000ff);
	pw_rpt_set[5]=(unsigned int)((reg_2e24 & 0x0000ff00)>>8);
	pw_rpt_set[6]=(unsigned int)((reg_2e24 & 0x00ff0000)>>16);
	pw_rpt_set[7]=(unsigned int)((reg_2e24 & 0xff000000)>>24);
	reg_2e28 = odm_get_bb_reg(dm, 0x2e28, 0xffffffff);
	pw_rpt_set[8]=(unsigned int)(reg_2e28 & 0x000000ff);
	pw_rpt_set[9]=(unsigned int)((reg_2e28 & 0x0000ff00)>>8);
	pw_rpt_set[10]=(unsigned int)((reg_2e28 & 0x00ff0000)>>16);
	pw_rpt_set[11]=(unsigned int)((reg_2e28 & 0xff000000)>>24);

	/* PRI : Wordlength : U(8,0) , unit: 25.6us */
	reg_2e2c = odm_get_bb_reg(dm, 0x2e2c, 0xffffffff);
	pri_rpt_set[0]=(unsigned int)(reg_2e2c & 0x000000ff);
	pri_rpt_set[1]=(unsigned int)((reg_2e2c & 0x0000ff00)>>8);
	pri_rpt_set[2]=(unsigned int)((reg_2e2c & 0x00ff0000)>>16);
	pri_rpt_set[3]=(unsigned int)((reg_2e2c & 0xff000000)>>24);
	reg_2e30 = odm_get_bb_reg(dm, 0x2e30, 0xffffffff);
	pri_rpt_set[4]=(unsigned int)(reg_2e30 & 0x000000ff);
	pri_rpt_set[5]=(unsigned int)((reg_2e30 & 0x0000ff00)>>8);
	pri_rpt_set[6]=(unsigned int)((reg_2e30 & 0x00ff0000)>>16);
	pri_rpt_set[7]=(unsigned int)((reg_2e30 & 0xff000000)>>24);
	reg_2e34 = odm_get_bb_reg(dm, 0x2e34, 0xffffffff);
	pri_rpt_set[8]=(unsigned int)(reg_2e34 & 0x000000ff);
	pri_rpt_set[9]=(unsigned int)((reg_2e34 & 0x0000ff00)>>8);
	pri_rpt_set[10]=(unsigned int)((reg_2e34 & 0x00ff0000)>>16);
	pri_rpt_set[11]=(unsigned int)((reg_2e34 & 0xff000000)>>24);
	
	/* Location : Wordlength : S(8,0) , unit: MHz */
	reg_2e0c = odm_get_bb_reg(dm, 0x2e0c, 0xffffffff);
	loct_rpt_set[0]=(signed char)(reg_2e0c & 0x000000ff);
	loct_rpt_set[1]=(signed char)((reg_2e0c & 0x0000ff00)>>8);
	loct_rpt_set[2]=(signed char)((reg_2e0c & 0x00ff0000)>>16);
	loct_rpt_set[3]=(signed char)((reg_2e0c & 0xff000000)>>24);
	reg_2e10 = odm_get_bb_reg(dm, 0x2e10, 0xffffffff);
	loct_rpt_set[4]=(signed char)(reg_2e10 & 0x000000ff);
	loct_rpt_set[5]=(signed char)((reg_2e10 & 0x0000ff00)>>8);
	loct_rpt_set[6]=(signed char)((reg_2e10 & 0x00ff0000)>>16);
	loct_rpt_set[7]=(signed char)((reg_2e10 & 0xff000000)>>24);
	reg_2e20 = odm_get_bb_reg(dm, 0x2e20, 0xffffffff);
	loct_rpt_set[8]=(signed char)(reg_2e20 & 0x000000ff);
	loct_rpt_set[9]=(signed char)((reg_2e20 & 0x0000ff00)>>8);
	loct_rpt_set[10]=(signed char)((reg_2e20 & 0x00ff0000)>>16);
	loct_rpt_set[11]=(signed char)((reg_2e20 & 0xff000000)>>24);

	dfs->pw_flag = false;
	dfs->loct_flag = false;
	dfs->pri_flag = false;
	for (i = 0; i < 12; i++) {
		dfs->pw_rslt[i] = 0;
		dfs->loct_rslt[i] = 0;
		dfs->pri_rslt[i] = 0;
	}

	// for NHMxDFS //
	dfs->nhm_dfs_cnt = 0;
	dfs->nhm_dfs_flag = false;
	dfs->b_flag = false;

	short_cnt_th = (u8)odm_get_bb_reg(dm, 0xa50, 0x00f00000);
	long_cnt_th = (u8)odm_get_bb_reg(dm, 0xa4c, 0xf0000000);

	if (dfs->radar_type) 
		loop_cnt = long_cnt_th + 2;
	else
		loop_cnt = short_cnt_th  + 2;

	if (region_domain == PHYDM_DFS_DOMAIN_ETSI)
		loop_cnt = short_cnt_th  + 2;
	
	if (loop_cnt > 12)
		loop_cnt = 12;

	cnt_th = loop_cnt / 2;
	
	/* Calculate the difference between each element (1st Not calculated) */
	for(i = 0; i < loop_cnt; i++)
		dfs->loct_rslt[i] = loct_rpt_set[i];
	for(i = 1; i < loop_cnt; i++)
		loct_diff = loct_diff + abs(dfs->loct_rslt[i] - dfs->loct_rslt[i-1]);
	
	for (i = 0; i < loop_cnt; i++) {
		if ((-2 < loct_rpt_set[i]) && (loct_rpt_set[i] < 2))
			dc_cnt++;
	}

	if (region_domain == PHYDM_DFS_DOMAIN_FCC){
		rdr_num = 7;
		
		pw_lth[0]=2;
		pw_lth[1]=2;
		pw_lth[2]=2;
		pw_lth[3]=12;
		pw_lth[4]=22;
		pw_lth[5]=120;
		pw_lth[6]=2;
		
		pw_uth[0]=3;
		pw_uth[1]=3;
		pw_uth[2]=15;
		pw_uth[3]=30;
		pw_uth[4]=60;
		pw_uth[5]=253;
		pw_uth[6]=3;
		
		pri_lth[0]=50;
		pri_lth[1]=18;
		pri_lth[2]=5;
		pri_lth[3]=7;
		pri_lth[4]=7;
		pri_lth[5]=38;
		pri_lth[6]=11;
		
		pri_uth[0]=62;
		pri_uth[1]=130;
		pri_uth[2]=10;
		pri_uth[3]=21;
		pri_uth[4]=21;
		pri_uth[5]=80;
		pri_uth[6]=15;
		if (dc_cnt > dfs->dc_cnt_th){
			pw_lth[0] = 0;
			pw_lth[1] = 0;
			pw_lth[2] = 0;
			pw_lth[3] = 0;
			pw_lth[4] = 0;
			pw_lth[5] = 80;
			pw_lth[6] = 0;
			}
		}
	else if (region_domain == PHYDM_DFS_DOMAIN_MKK){
		rdr_num = 8;
		if (c_channel >= 52 && c_channel <= 64){
			dfs->loct_diff_th = dfs->loct_diff_th + 15;
			pw_lth[0]=1;
			pw_lth[1]=1;
			pw_lth[2]=1;
			pw_lth[3]=1;
			pw_lth[4]=1;
			pw_lth[5]=1;
			pw_lth[6]=1;
			pw_lth[7]=1;

			pw_uth[0]=14;
			pw_uth[1]=40;
			pw_uth[2]=14;
			pw_uth[3]=40;
			pw_uth[4]=5;
			pw_uth[5]=5;
			pw_uth[6]=5;
			pw_uth[7]=5;

			pri_lth[0]=35;
			pri_lth[1]=20;
			pri_lth[2]=35;
			pri_lth[3]=18;
			pri_lth[4]=32;
			pri_lth[5]=38;
			pri_lth[6]=40;
			pri_lth[7]=48;

			pri_uth[0]=200;
			pri_uth[1]=200;
			pri_uth[2]=200;
			pri_uth[3]=200;
			pri_uth[4]=38;
			pri_uth[5]=44;
			pri_uth[6]=48;
			pri_uth[7]=55;	
			if (dc_cnt > dfs->dc_cnt_th){
				pw_lth[0] = 0;
				pw_lth[1] = 0;
				pw_lth[2] = 0;
				pw_lth[3] = 0;
				pw_lth[4] = 0;
				pw_lth[5] = 0;
				pw_lth[6] = 0;
				pw_lth[7] = 0;
				cnt_th--;
				}
			}
		else{
			pw_lth[0]=1;
			pw_lth[1]=2;
			pw_lth[2]=4;
			pw_lth[3]=2;
			pw_lth[4]=12;
			pw_lth[5]=22;
			pw_lth[6]=100;
			pw_lth[7]=2;

			pw_uth[0]=2;
			pw_uth[1]=3;
			pw_uth[2]=6;
			pw_uth[3]=15;
			pw_uth[4]=30;
			pw_uth[5]=60;
			pw_uth[6]=300;
			pw_uth[7]=3;

			pri_lth[0]=33;
			pri_lth[1]=35;
			pri_lth[2]=130;
			pri_lth[3]=5;
			pri_lth[4]=7;
			pri_lth[5]=7;
			pri_lth[6]=35;
			pri_lth[7]=11;

			pri_uth[0]=73;
			pri_uth[1]=75;
			pri_uth[2]=170;
			pri_uth[3]=10;
			pri_uth[4]=22;
			pri_uth[5]=22;
			pri_uth[6]=86;
			pri_uth[7]=15;
			if (dc_cnt > dfs->dc_cnt_th){
				pw_lth[0] = 0;
				pw_lth[1] = 0;
				pw_lth[2] = 0;
				pw_lth[3] = 0;
				pw_lth[4] = 0;
				pw_lth[5] = 0;
				pw_lth[6] = 80;
				pw_lth[7] = 0;
				}
			}
		}
	else if (region_domain == PHYDM_DFS_DOMAIN_ETSI){
		rdr_num = 6;
		pw_lth[0]=1;
		pw_lth[1]=1;
		pw_lth[2]=1;
		pw_lth[3]=40;
		pw_lth[4]=1;
		pw_lth[5]=1;
	
		pw_uth[0]=15;
		pw_uth[1]=45;
		pw_uth[2]=45;
		pw_uth[3]=90;
		pw_uth[4]=6;
		pw_uth[5]=6;

		pri_lth[0]=35;
		pri_lth[1]=21;
		pri_lth[2]=8;
		pri_lth[3]=8;
		pri_lth[4]=87;
		pri_lth[5]=29;

		pri_uth[0]=215;
		pri_uth[1]=215;
		pri_uth[2]=19;
		pri_uth[3]=22;
		pri_uth[4]=144;
		pri_uth[5]=108;
		if (dc_cnt > dfs->dc_cnt_th){
			pw_lth[0] = 0;
			pw_lth[1] = 0;
			pw_lth[2] = 0;
			pw_lth[3] = 0;
			pw_lth[4] = 0;
			pw_lth[5] = 0;
			}
		}	

	for (i = 0; i < loop_cnt; i++) {
		dfs->pw_rslt[i] = (u8)pw_rpt_set[i];
		dfs->pri_rslt[i] = (u8)pri_rpt_set[i];
		for (j = 0; j < rdr_num; j++) {
			if (dfs->radar_type) {
				if ((pw_lth[j] <= dfs->pw_rslt[i]) && (dfs->pw_rslt[i] <= pw_uth[j]))
					rdr_cnt[j]++;
			} else {
				if ((pw_lth[j] <= dfs->pw_rslt[i]) && (dfs->pw_rslt[i] <= pw_uth[j]) &&
				    (pri_lth[j] <= dfs->pri_rslt[i]) && (dfs->pri_rslt[i] <= pri_uth[j])) {
					rdr_cnt[j]++;
					if (dfs->pri_rslt[i] > pri_vd_max)
						pri_vd_max = dfs->pri_rslt[i];
					if (dfs->pri_rslt[i] < pri_vd_min)
						pri_vd_min = dfs->pri_rslt[i];
				}
			}
		}
	}

	/* Pattern Judgement */
	for (i = 0; i < rdr_num; i++) {
		if (rdr_cnt[i] >= cnt_th) {
			dfs->pw_flag = true;
			rdr_tmp = i;
		}
	}
	//PHYDM_DBG(dm, DBG_DFS, "rdr_tmp=%d\n", rdr_tmp);
	
	/* Add NHM to decide the validity of the three flags */
	/* Use for chaotic open space */
	if (dfs->nhm_dfs_en) {
		for (i = 0; i <= (NHM_RPT_NUM - 1); i++) {
			if (ccx->nhm_result[i] > 0)
				break;
		}
		for (j = (i + 1); j <= (NHM_RPT_NUM - 1); j++) {
			if (ccx->nhm_result[j] > 0)
				dfs->nhm_dfs_cnt++;
		}
		if (dfs->nhm_dfs_cnt > dfs->nhm_cnt_th) {
			dfs->nhm_dfs_flag = true;
			cnt_th = loop_cnt;
		}
	}
	if (dm->support_ability & ODM_BB_ENV_MONITOR) {
		if ((100-((ccx->nhm_rpt_sum * 100) >> 8)) >= dfs->nhm_dty_th) {
			dfs->b_flag = true;
			cnt_th = loop_cnt;
		}
	}  

	/* Relax */
	if (!(dfs->idle_mode)) {
		pri_cnt_th--;
		if (!(region_domain == PHYDM_DFS_DOMAIN_ETSI)){
			dfs->pri_var_offset = 15;
			if (region_domain == PHYDM_DFS_DOMAIN_MKK)
				dfs->pri_var_offset = 5 + 10*pri_vd_min/20;
			}
		else
			dfs->pri_var_offset = dfs->pri_var_tp_offset;	
	}
	
	// Mode approximation for PRI
	for (i = 0; i < (loop_cnt - 1); i++) {
		pri_cnt_tmp = 0;
		for (j = 0; j < (loop_cnt - 1); j++) {
			if ((i == j) || (dfs->pri_rslt[i] <= 4))
				continue;
			if (dfs->pri_rslt[j] > (255 - dfs->pri_var_offset))
				pri_var_up = 255;
			else
				pri_var_up = dfs->pri_rslt[j] + dfs->pri_var_offset;
			if (dfs->pri_var_offset >= dfs->pri_rslt[j])
				pri_var_low = 0;
			else
				pri_var_low = dfs->pri_rslt[j] - dfs->pri_var_offset;
			if ((dfs->pri_rslt[i] <= pri_var_up) && (dfs->pri_rslt[i] >= pri_var_low) &&
			    (dfs->pri_rslt[i] <= pri_vd_max) && (dfs->pri_rslt[i] >= pri_vd_min))
				pri_cnt_tmp++;
		}
		if (pri_cnt_tmp > pri_cnt_fnl)
			pri_cnt_fnl = pri_cnt_tmp;
	}

	// Advanced PRI Judgement (Bypass for long pulse flag)
	if (pri_cnt_fnl != 0)
		pri_cnt_fnl++;

	/* PRI */
	if (dfs->radar_type)
		dfs->pri_flag= true;
	else if (pri_cnt_fnl >= pri_cnt_th)
		dfs->pri_flag= true;
	/* For the case of SRTC */
	else if ((dfs->pri_rslt[0] == 0) && (dfs->pri_rslt[1] > 0) &&
		 (dfs->pri_rslt[2] == 0) && (dfs->pri_rslt[3] > 0))
		dfs->pri_flag= true;
	else if ((dfs->pri_rslt[0] > 0) && (dfs->pri_rslt[1] == 0) &&
		 (dfs->pri_rslt[2] > 0) && (dfs->pri_rslt[3] == 0))
		dfs->pri_flag= true;
	else if (!(dfs->pri_flag_en))
		dfs->pri_flag= true;

	if (region_domain == PHYDM_DFS_DOMAIN_MKK){
		if (c_channel >= 52 && c_channel <= 64){
			if (!(dfs->idle_mode))
				dfs->pri_flag= true;     // MKK, 52<=ch<=64 , TP mode	
			}
		}

	/* PW */
	if (!(dfs->pw_flag_en))
		dfs->pw_flag = true;

	/* LOCT */
	if (!(dfs->idle_mode))
		dfs->loct_diff_th = dfs->loct_diff_th + 15;
	if (loct_diff <= dfs->loct_diff_th)
		dfs->loct_flag = true;
	else if (dfs->radar_type)
		dfs->loct_flag = true;
	else if (!(dfs->loct_flag_en))
		dfs->loct_flag = true;

	if (dfs->det_print_jar3) {
		if (dfs->nhm_dfs_flag || dfs->b_flag)
			PHYDM_DBG(dm, DBG_DFS, "Anti-FRD Rdr Drop\n");
		if (!(dfs->pw_flag_en) || !(dfs->loct_flag_en) || !(dfs->pri_flag_en))
			PHYDM_DBG(dm, DBG_DFS, "Byps flag scenario\n");
	       if (dc_cnt > dfs->dc_cnt_th)
		   	PHYDM_DBG(dm, DBG_DFS, "Radar is near DC\n");
		if (!(dfs->pw_flag))
			PHYDM_DBG(dm, DBG_DFS, "- PW/PRI-J Failure\n");
		if (!(dfs->loct_flag))
			PHYDM_DBG(dm, DBG_DFS, "- Loct-J Failure\n");
		if (!(dfs->pri_flag))
			PHYDM_DBG(dm, DBG_DFS, "- Adv-PRI-J Failure\n");
		if (!(dfs->pw_flag) || !(dfs->pri_flag) || !(dfs->loct_flag))
			PHYDM_DBG(dm, DBG_DFS, "Suspected radar result is dropped by SW mechanism !!\n");

		PHYDM_DBG(dm, DBG_DFS, "\n");
		PHYDM_DBG(dm, DBG_DFS, "The current information: ([Lower bound / Upper bound])\n");
		PHYDM_DBG(dm, DBG_DFS, "pw_th1 : [%d, %d], pw_th2 : [%d, %d], pw_th3 : [%d, %d]\n",
			  pw_lth[0], pw_uth[0], pw_lth[1], pw_uth[1],
			  pw_lth[2], pw_uth[2]);
		PHYDM_DBG(dm, DBG_DFS, "pw_th4 : [%d, %d], pw_th5 : [%d, %d], pw_th6 : [%d, %d] \n",
			  pw_lth[3], pw_uth[3], pw_lth[4],
			  pw_uth[4], pw_lth[5], pw_uth[5]);
		PHYDM_DBG(dm, DBG_DFS, "pw_th7 : [%d, %d], pw_th8 : [%d, %d]\n",
			  pw_lth[6], pw_uth[6], pw_lth[7], pw_uth[7]);
		PHYDM_DBG(dm, DBG_DFS, "pri_th1 : [%d, %d], pri_th2 : [%d, %d], pri_th3 : [%d, %d]\n",
			  pri_lth[0], pri_uth[0], pri_lth[1], pri_uth[1],
			  pri_lth[2], pri_uth[2]);
		PHYDM_DBG(dm, DBG_DFS, "pri_th4 : [%d, %d], pri_th5 : [%d, %d], pri_th6 : [%d, %d]\n",
			  pri_lth[3], pri_uth[3], pri_lth[4], pri_uth[4],
			  pri_lth[5], pri_uth[5]);
		PHYDM_DBG(dm, DBG_DFS, "pri_th7 : [%d, %d], pri_th8 : [%d, %d]\n",
			  pri_lth[6], pri_uth[6], pri_lth[7], pri_uth[7]);

		if (!(dfs->idle_mode))
			PHYDM_DBG(dm, DBG_DFS, "loct_diff_th = [%d](Relax for TP mode)\n",
				  dfs->loct_diff_th);
		else
			PHYDM_DBG(dm, DBG_DFS, "loct_diff_th = [%d](Idle mode)\n",
				  dfs->loct_diff_th);

		if (!(dfs->idle_mode)) {
			PHYDM_DBG(dm, DBG_DFS, "pri_var_offset = [%d] (Relax for TP mode)\n",
				  dfs->pri_var_offset);
			PHYDM_DBG(dm, DBG_DFS, "pri_cnt_th = [%d] (Relax for TP mode)\n",
				  pri_cnt_th);
		} else {
			PHYDM_DBG(dm, DBG_DFS, "pri_var_offset = [%d] (Idle mode)\n",
				  dfs->pri_var_offset);
			PHYDM_DBG(dm, DBG_DFS, "pri_cnt_th = [%d] (Idle mode)\n",
				  pri_cnt_th);
		}

		PHYDM_DBG(dm, DBG_DFS, "\n");
		PHYDM_DBG(dm, DBG_DFS, "loop_cnt = [%d]\n", loop_cnt);
		if (dfs->nhm_dfs_flag) {
			PHYDM_DBG(dm, DBG_DFS, "Nmode cnt = %d!\n",
				  dfs->nhm_dfs_cnt);
		} 
		else if (dfs->b_flag) {
			PHYDM_DBG(dm, DBG_DFS, "Bmode DTY = %d\n",
				  (100-((ccx->nhm_rpt_sum * 100) >> 8)));
		} 
		else {
			PHYDM_DBG(dm, DBG_DFS, "cnt_th = [%d] (Default : Half of loop cnt)\n",
				  cnt_th);
			PHYDM_DBG(dm, DBG_DFS, "Bmode DTY = %d\n",
				  (100-((ccx->nhm_rpt_sum * 100) >> 8)));
		} 

		PHYDM_DBG(dm, DBG_DFS, "\n");
		PHYDM_DBG(dm, DBG_DFS, "The current report:\n");
		PHYDM_DBG(dm, DBG_DFS, "pw_rpt(1-12) = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
			  pw_rpt_set[0], pw_rpt_set[1], pw_rpt_set[2],
			  pw_rpt_set[3], pw_rpt_set[4], pw_rpt_set[5],
			  pw_rpt_set[6], pw_rpt_set[7], pw_rpt_set[8],
			  pw_rpt_set[9], pw_rpt_set[10], pw_rpt_set[11]);
		PHYDM_DBG(dm, DBG_DFS, "loct_rpt(1-12) = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
			  loct_rpt_set[0], loct_rpt_set[1],
			  loct_rpt_set[2], loct_rpt_set[3],
			  loct_rpt_set[4], loct_rpt_set[5],
			  loct_rpt_set[6], loct_rpt_set[7],
			  loct_rpt_set[8], loct_rpt_set[9],
			  loct_rpt_set[10], loct_rpt_set[11]);
		PHYDM_DBG(dm, DBG_DFS, "pri_rpt(1-12) = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
			  pri_rpt_set[0], pri_rpt_set[1],
			  pri_rpt_set[2], pri_rpt_set[3],
			  pri_rpt_set[4], pri_rpt_set[5],
			  pri_rpt_set[6], pri_rpt_set[7],
			  pri_rpt_set[8], pri_rpt_set[9],
			  pri_rpt_set[10], pri_rpt_set[11]);

		PHYDM_DBG(dm, DBG_DFS, "\n");
		PHYDM_DBG(dm, DBG_DFS, "dc_cnt = [%d]\n", dc_cnt);
		PHYDM_DBG(dm, DBG_DFS, "loct_diff = [%d]\n", loct_diff);

		if (pri_vd_max == 0 && pri_vd_min == 255)
			PHYDM_DBG(dm, DBG_DFS, "No vaild pri_max and pri_min");
		else
			PHYDM_DBG(dm, DBG_DFS, "pri_vaild_max = %d, pri_vaild_min = %d\n",
				  pri_vd_max, pri_vd_min);
		PHYDM_DBG(dm, DBG_DFS, "advanced_pri_cnt = %d\n",
			  pri_cnt_fnl);

		PHYDM_DBG(dm, DBG_DFS, "radar_class_cnt[1-8] = [%d, %d, %d, %d, %d, %d, %d, %d]\n",
			  rdr_cnt[0], rdr_cnt[1], rdr_cnt[2], rdr_cnt[3],
			  rdr_cnt[4], rdr_cnt[5], rdr_cnt[6], rdr_cnt[7]);
	}
}
#endif

#if (RTL8814A_SUPPORT || RTL8822B_SUPPORT || RTL8821C_SUPPORT)
void phydm_dfs_histogram_radar_distinguish(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	u8 region_domain = dm->dfs_region_domain;
	u8 c_channel = *dm->channel;
	u8 band_width = *dm->band_width;

	u8 dfs_pw_thd1 = 0, dfs_pw_thd2 = 0, dfs_pw_thd3 = 0;
	u8 dfs_pw_thd4 = 0, dfs_pw_thd5 = 0;
	u8 dfs_pri_thd1 = 0, dfs_pri_thd2 = 0, dfs_pri_thd3 = 0;
	u8 dfs_pri_thd4 = 0, dfs_pri_thd5 = 0;
	u8 pri_th = 0, i = 0;
	u8 max_pri_idx = 0, max_pw_idx = 0, max_pri_cnt_th = 0;
	u8 max_pri_cnt_fcc_g1_th = 0, max_pri_cnt_fcc_g3_th = 0;
	u8 safe_pri_pw_diff_th = 0, safe_pri_pw_diff_fcc_th = 0;
	u8 safe_pri_pw_diff_w53_th = 0, safe_pri_pw_diff_fcc_idle_th = 0;
	u16 j = 0;
	u32 dfs_hist1_pw = 0, dfs_hist2_pw = 0, g_pw[6] = {0};
	u32 dfs_hist1_pri = 0, dfs_hist2_pri = 0, g_pri[6] = {0};
	u8 pw_sum_g0g5 = 0, pw_sum_g1g2g3g4 = 0;
	u8 pri_sum_g0g5 = 0, pri_sum_g1g2g3g4 = 0;
	u16 pw_sum_ss_g1g2g3g4 = 0, pri_sum_ss_g1g2g3g4 = 0;
	u8 max_pri_cnt = 0, max_pw_cnt = 0, valid_set = 0;
	#if (RTL8721D_SUPPORT)
	if (dm->support_ic_type & (ODM_RTL8721D))
		return;
	#endif

	/*read pulse width hist report*/
	odm_set_bb_reg(dm, 0x19e4, BIT(22) | BIT(23), 0x1);
	dfs_hist1_pw = odm_get_bb_reg(dm, 0xf5c, 0xffffffff);
	dfs_hist2_pw = odm_get_bb_reg(dm, 0xf74, 0xffffffff);

	g_pw[0] = (unsigned int)((dfs_hist2_pw & 0xff000000) >> 24);
	g_pw[1] = (unsigned int)((dfs_hist2_pw & 0x00ff0000) >> 16);
	g_pw[2] = (unsigned int)((dfs_hist2_pw & 0x0000ff00) >> 8);
	g_pw[3] = (unsigned int)dfs_hist2_pw & 0x000000ff;
	g_pw[4] = (unsigned int)((dfs_hist1_pw & 0xff000000) >> 24);
	g_pw[5] = (unsigned int)((dfs_hist1_pw & 0x00ff0000) >> 16);

	/*read pulse repetition interval hist report*/
	odm_set_bb_reg(dm, 0x19e4, BIT(22) | BIT(23), 0x3);
	dfs_hist1_pri = odm_get_bb_reg(dm, 0xf5c, 0xffffffff);
	dfs_hist2_pri = odm_get_bb_reg(dm, 0xf74, 0xffffffff);
	odm_set_bb_reg(dm, 0x19b4, 0x10000000, 1); /*reset histo report*/
	odm_set_bb_reg(dm, 0x19b4, 0x10000000, 0); /*@continue histo report*/

	g_pri[0] = (unsigned int)((dfs_hist2_pri & 0xff000000) >> 24);
	g_pri[1] = (unsigned int)((dfs_hist2_pri & 0x00ff0000) >> 16);
	g_pri[2] = (unsigned int)((dfs_hist2_pri & 0x0000ff00) >> 8);
	g_pri[3] = (unsigned int)dfs_hist2_pri & 0x000000ff;
	g_pri[4] = (unsigned int)((dfs_hist1_pri & 0xff000000) >> 24);
	g_pri[5] = (unsigned int)((dfs_hist1_pri & 0x00ff0000) >> 16);

	dfs->pri_cond1 = 0;
	dfs->pri_cond2 = 0;
	dfs->pri_cond3 = 0;
	dfs->pri_cond4 = 0;
	dfs->pri_cond5 = 0;
	dfs->pri_cond6 = 0;
	dfs->pw_cond1 = 0;
	dfs->pw_cond2 = 0;
	dfs->pw_cond3 = 0;
	dfs->pri_type3_4_cond1 = 0;	/*@for ETSI*/
	dfs->pri_type3_4_cond2 = 0;	/*@for ETSI*/
	dfs->pw_long_cond1 = 0;		/*@for long radar*/
	dfs->pw_long_cond2 = 0;		/*@for long radar*/
	dfs->pri_long_cond1 = 0;	/*@for long radar*/
	dfs->pw_flag = 0;
	dfs->pri_flag = 0;
	dfs->pri_type3_4_flag = 0;	/*@for ETSI*/
	dfs->long_radar_flag = 0;
	dfs->pw_std = 0;	/*The std(var) of reasonable num of pw group*/
	dfs->pri_std = 0;	/*The std(var) of reasonable num of pri group*/

	for (i = 0; i < 6; i++) {
		dfs->pw_hold_sum[i] = 0;
		dfs->pri_hold_sum[i] = 0;
		dfs->pw_long_hold_sum[i] = 0;
		dfs->pri_long_hold_sum[i] = 0;
	}

	if (dfs->idle_mode == 1)
		pri_th = dfs->pri_hist_idle_th;
	else
		pri_th = dfs->pri_hist_th;

	for (i = 0; i < 6; i++) {
		dfs->pw_hold[dfs->hist_idx][i] = (u8)g_pw[i];
		dfs->pri_hold[dfs->hist_idx][i] = (u8)g_pri[i];
		/*@collect whole histogram report may take some time
		 *so we add the counter of 2 time slots in FCC and ETSI
		 */
		dfs->pw_hold_sum[i] = dfs->pw_hold_sum[i] +
			dfs->pw_hold[(dfs->hist_idx + 1) % 3][i] +
			dfs->pw_hold[(dfs->hist_idx + 2) % 3][i];
		dfs->pri_hold_sum[i] = dfs->pri_hold_sum[i] +
			dfs->pri_hold[(dfs->hist_idx + 1) % 3][i] +
			dfs->pri_hold[(dfs->hist_idx + 2) % 3][i];
	}
	/*@For long radar type*/
	for (i = 0; i < 6; i++) {
		dfs->pw_long_hold[dfs->hist_long_idx][i] = (u8)g_pw[i];
		dfs->pri_long_hold[dfs->hist_long_idx][i] = (u8)g_pri[i];
		/*collect whole histogram report may take some time, so we add the counter of 299 time slots for long radar*/
		for (j = 1; j < 300; j++) {
			dfs->pw_long_hold_sum[i] = dfs->pw_long_hold_sum[i] +
				dfs->pw_long_hold[(dfs->hist_long_idx + j) % 300][i];
			dfs->pri_long_hold_sum[i] = dfs->pri_long_hold_sum[i] +
				dfs->pri_long_hold[(dfs->hist_long_idx + j) % 300][i];
		}
	}

	dfs->hist_idx++;
	if (dfs->hist_idx == 3)
		dfs->hist_idx = 0;
	dfs->hist_long_idx++;
	if (dfs->hist_long_idx == 300)
		dfs->hist_long_idx = 0;

	max_pri_cnt = 0;
	max_pri_idx = 0;
	max_pw_cnt = 0;
	max_pw_idx = 0;
	max_pri_cnt_th = dfs->pri_sum_g1_th;
	max_pri_cnt_fcc_g1_th = dfs->pri_sum_g1_fcc_th;
	max_pri_cnt_fcc_g3_th = dfs->pri_sum_g3_fcc_th;
	safe_pri_pw_diff_th = dfs->pri_pw_diff_th;
	safe_pri_pw_diff_fcc_th = dfs->pri_pw_diff_fcc_th;
	safe_pri_pw_diff_fcc_idle_th = dfs->pri_pw_diff_fcc_idle_th;
	safe_pri_pw_diff_w53_th = dfs->pri_pw_diff_w53_th;

	/*@g1 to g4 is the reseasonable range of pri and pw*/
	for (i = 1; i <= 4; i++) {
		if (dfs->pri_hold_sum[i] > 0)
			valid_set++;
		if (dfs->pw_hold_sum[i]>0)
			valid_set++;
		if (dfs->pri_hold_sum[i] > max_pri_cnt) {
			max_pri_cnt = dfs->pri_hold_sum[i];
			max_pri_idx = i;
		}
		if (dfs->pw_hold_sum[i] > max_pw_cnt) {
			max_pw_cnt = dfs->pw_hold_sum[i];
			max_pw_idx = i;
		}
		if (dfs->pri_hold_sum[i] >= pri_th)
			dfs->pri_cond1 = 1;
	}

	pri_sum_g0g5 = dfs->pri_hold_sum[0];
	if (pri_sum_g0g5 == 0)
		pri_sum_g0g5 = 1;
	pri_sum_g1g2g3g4 = dfs->pri_hold_sum[1] + dfs->pri_hold_sum[2]
			 + dfs->pri_hold_sum[3] + dfs->pri_hold_sum[4];

	/*pw will reduce because of dc, so we do not treat g0 as illegal group*/
	pw_sum_g0g5 = dfs->pw_hold_sum[5];
	if (pw_sum_g0g5 == 0)
		pw_sum_g0g5 = 1;
	pw_sum_g1g2g3g4 = dfs->pw_hold_sum[1] + dfs->pw_hold_sum[2] +
				dfs->pw_hold_sum[3] + dfs->pw_hold_sum[4];

	/*@Calculate the variation from g1 to g4*/
	for (i = 1; i < 5; i++) {
		/*Sum of square*/
		pw_sum_ss_g1g2g3g4 = pw_sum_ss_g1g2g3g4 +
		(dfs->pw_hold_sum[i] - (pw_sum_g1g2g3g4 / 4)) *
		(dfs->pw_hold_sum[i] - (pw_sum_g1g2g3g4 / 4));
		pri_sum_ss_g1g2g3g4 = pri_sum_ss_g1g2g3g4 +
		(dfs->pri_hold_sum[i] - (pri_sum_g1g2g3g4 / 4)) *
		(dfs->pri_hold_sum[i] - (pri_sum_g1g2g3g4 / 4));
	}
	/*The value may less than the normal variance,
	 *since the variable type is int (not float)
	 */
		dfs->pw_std = (u8)(pw_sum_ss_g1g2g3g4 / 4);
		dfs->pri_std = (u8)(pri_sum_ss_g1g2g3g4 / 4);

	if (region_domain == 1) {
		dfs->pri_type3_4_flag = 1;	/*@ETSI flag*/

		/*(OTA) Cancel long PRI case*/
		dfs->pri_cond2 = 1;

		/*reasonable group shouldn't large*/
		if ((pri_sum_g0g5 + pri_sum_g1g2g3g4) / pri_sum_g0g5 > 2 &&
		    pri_sum_g1g2g3g4 <= dfs->pri_sum_safe_fcc_th)
			dfs->pri_cond3 = 1;

		/*@Cancel the condition that the abs between pri and pw*/
		if (dfs->pri_std >= dfs->pri_std_th)
			dfs->pri_cond4 = 1;
		else if (max_pri_idx == 1 &&
			 max_pri_cnt >= max_pri_cnt_fcc_g1_th)
			dfs->pri_cond4 = 1;

		/*(OTA) Cancel the condition (type 3,4 distinction)*/
		dfs->pri_cond5 = 1;

		if (dfs->pri_cond1 && dfs->pri_cond2 && dfs->pri_cond3 &&
		    dfs->pri_cond4 && dfs->pri_cond5)
			dfs->pri_flag = 1;

		/* PW judgment conditions for short radar type */
		/*ratio of reasonable and illegal group && g5 should be zero*/
		if (((pw_sum_g0g5 + pw_sum_g1g2g3g4) / pw_sum_g0g5 > 2) &&
		    (dfs->pw_hold_sum[5] <= 1))
			dfs->pw_cond1 = 1;
		/*unreasonable group*/
		if (dfs->pw_hold_sum[4] == 0 && dfs->pw_hold_sum[5] == 0)
			dfs->pw_cond2 = 1;
		/*pw's std (short radar) should be large(=7)*/
		if (dfs->pw_std >= dfs->pw_std_th)
			dfs->pw_cond3 = 1;
		if (dfs->pw_cond1 && dfs->pw_cond2 && dfs->pw_cond3)
			dfs->pw_flag = 1;

		/* @Judgment conditions of long radar type */
		if (band_width == CHANNEL_WIDTH_20) {
			if (dfs->pw_long_hold_sum[4] >=
			    dfs->pw_long_lower_20m_th)
				dfs->pw_long_cond1 = 1;
		} else{
			if (dfs->pw_long_hold_sum[4] >= dfs->pw_long_lower_th)
				dfs->pw_long_cond1 = 1;
		}
		/* @Disable the condition that dfs->pw_long_hold_sum[1] */
		if (dfs->pw_long_hold_sum[2] + dfs->pw_long_hold_sum[3] +
		    dfs->pw_long_hold_sum[4] <= dfs->pw_long_sum_upper_th &&
		    dfs->pw_long_hold_sum[2] <= dfs->pw_long_hold_sum[4] &&
		    dfs->pw_long_hold_sum[3] <= dfs->pw_long_hold_sum[4])
			dfs->pw_long_cond2 = 1;
		/*@g4 should be large for long radar*/
		if (dfs->pri_long_hold_sum[4] <= dfs->pri_long_upper_th)
			dfs->pri_long_cond1 = 1;
		if (dfs->pw_long_cond1 && dfs->pw_long_cond2 &&
		    dfs->pri_long_cond1)
			dfs->long_radar_flag = 1;
	} else if (region_domain == 2) {
		dfs->pri_type3_4_flag = 1;	/*@ETSI flag*/

		/*PRI judgment conditions for short radar type*/
		if ((pri_sum_g0g5 + pri_sum_g1g2g3g4) / pri_sum_g0g5 > 2)
			dfs->pri_cond2 = 1;

		/*reasonable group shouldn't too large*/
		if (pri_sum_g1g2g3g4 <= dfs->pri_sum_safe_fcc_th)
			dfs->pri_cond3 = 1;

		/*Cancel the abs diff between pri and pw for idle mode (thr=2)*/
		dfs->pri_cond4 = 1;

		if (dfs->idle_mode == 1) {
			if (dfs->pri_std >= dfs->pri_std_idle_th) {
				if (max_pw_idx == 3 &&
				    pri_sum_g1g2g3g4 <= dfs->pri_sum_type4_th){
		/*To distinguish between type 4 radar and false detection*/
					dfs->pri_cond5 = 1;
				} else if (max_pw_idx == 1 &&
					   pri_sum_g1g2g3g4 >=
					   dfs->pri_sum_type6_th) {
		/*To distinguish between type 6 radar and false detection*/
					dfs->pri_cond5 = 1;
				} else {
		/*pri variation of short radar should be large (idle mode)*/
					dfs->pri_cond5 = 1;
				}
			}
		} else {
		/*pri variation of short radar should be large (TP mode)*/
			if (dfs->pri_std >= dfs->pri_std_th)
				dfs->pri_cond5 = 1;
		}

		if (dfs->pri_cond1 && dfs->pri_cond2 && dfs->pri_cond3 &&
		    dfs->pri_cond4 && dfs->pri_cond5)
			dfs->pri_flag = 1;

		/* PW judgment conditions for short radar type */
		if (((pw_sum_g0g5 + pw_sum_g1g2g3g4) / pw_sum_g0g5 > 2) &&
		    (dfs->pw_hold_sum[5] <= 1))
		/*ratio of reasonable and illegal group && g5 should be zero*/
			dfs->pw_cond1 = 1;

		if ((c_channel >= 52) && (c_channel <= 64))
			dfs->pw_cond2 = 1;
		/*unreasonable group shouldn't too large*/
		else if (dfs->pw_hold_sum[0] <= dfs->pw_g0_th)
			dfs->pw_cond2 = 1;

		if (dfs->idle_mode == 1) {
		/*pw variation of short radar should be large (idle mode)*/
			if (dfs->pw_std >= dfs->pw_std_idle_th)
				dfs->pw_cond3 = 1;
		} else {
		/*pw variation of short radar should be large (TP mode)*/
			if (dfs->pw_std >= dfs->pw_std_th)
				dfs->pw_cond3 = 1;
		}
		if (dfs->pw_cond1 && dfs->pw_cond2 && dfs->pw_cond3)
			dfs->pw_flag = 1;

		/* @Judgment conditions of long radar type */
		if (band_width == CHANNEL_WIDTH_20) {
			if (dfs->pw_long_hold_sum[4] >=
			    dfs->pw_long_lower_20m_th)
				dfs->pw_long_cond1 = 1;
		} else{
			if (dfs->pw_long_hold_sum[4] >= dfs->pw_long_lower_th)
				dfs->pw_long_cond1 = 1;
		}
		if (dfs->pw_long_hold_sum[1] + dfs->pw_long_hold_sum[2] +
		    dfs->pw_long_hold_sum[3] + dfs->pw_long_hold_sum[4]
		    <= dfs->pw_long_sum_upper_th)
			dfs->pw_long_cond2 = 1;
		/*@g4 should be large for long radar*/
		if (dfs->pri_long_hold_sum[4] <= dfs->pri_long_upper_th)
			dfs->pri_long_cond1 = 1;
		if (dfs->pw_long_cond1 &&
		    dfs->pw_long_cond2 && dfs->pri_long_cond1)
			dfs->long_radar_flag = 1;
	} else if (region_domain == 3) {
		/*ratio of reasonable group and illegal group */
		if ((pri_sum_g0g5 + pri_sum_g1g2g3g4) / pri_sum_g0g5 > 2)
			dfs->pri_cond2 = 1;

		if (pri_sum_g1g2g3g4 <= dfs->pri_sum_safe_th)
			dfs->pri_cond3 = 1;

		/*@Cancel the condition that the abs between pri and pw*/
		if (dfs->idle_mode) {
			if (ABS_8(pri_sum_g1g2g3g4 - pw_sum_g1g2g3g4) <= dfs->pri_pw_diff_idle_th)
				dfs->pri_cond4 = 1;
			if (dfs->pri_hold_sum[5] <= dfs->pri_sum_g5_idle_th)
				dfs->pri_cond5 = 1;
			if (dfs->pri_hold_sum[0] <= dfs->pri_sum_g0_idle_th)
				dfs->pri_cond6 = 1;
		} else {
			if (ABS_8(pri_sum_g1g2g3g4 - pw_sum_g1g2g3g4) <= dfs->pri_pw_diff_th)
				dfs->pri_cond4 = 1;
			if (dfs->pri_hold_sum[5] <= dfs->pri_sum_g5_th)
				dfs->pri_cond5 = 1;
			if (dfs->pri_hold_sum[0] <= dfs->pri_sum_g0_th)
				dfs->pri_cond6 = 1;
		}

		/*if (band_width == CHANNEL_WIDTH_40) {
			if (max_pw_idx == 4) {
				if (max_pw_cnt >= dfs->type4_pw_max_cnt &&
				    pri_sum_g1g2g3g4 >=
				    dfs->type4_safe_pri_sum_th) {
					dfs->pri_cond1 = 1;
					dfs->pri_cond4 = 1;
					dfs->pri_type3_4_cond1 = 1;
				}
			}
		}*/

		if (dfs->pri_cond1 && dfs->pri_cond2 &&
		    dfs->pri_cond3 && dfs->pri_cond4 && dfs->pri_cond5 && dfs->pri_cond6)
			dfs->pri_flag = 1;

		if (((pw_sum_g0g5 + pw_sum_g1g2g3g4) / pw_sum_g0g5 > 2))
			dfs->pw_cond1 = 1;

		if (dfs->idle_mode) {
			if (dfs->pw_hold_sum[0] <= dfs->pw_sum_g0_idle_th && dfs->pw_hold_sum[5] <= dfs->pw_sum_g5_idle_th)
				dfs->pw_cond2 = 1;
			if (valid_set <= dfs->pw_pri_valid_set_idle_th)
				dfs->pw_cond3 = 1;
		} else {
			if (dfs->pw_hold_sum[0] <= dfs->pw_sum_g0_th && dfs->pw_hold_sum[5] <= dfs->pw_sum_g5_th)
				dfs->pw_cond2 = 1;
			if (valid_set <= dfs->pw_pri_valid_set_th)
				dfs->pw_cond3 = 1;
		}

		if(dfs->pw_cond1 && dfs->pw_cond2 && dfs->pw_cond3)
			dfs->pw_flag = 1;

		/*@max num pri group is g1 means radar type3 or type4*/
		if (max_pri_idx == 1) {
			if (max_pri_cnt >= max_pri_cnt_th)
				dfs->pri_type3_4_cond1 = 1;
			if (dfs->pri_hold_sum[4] <=
			    dfs->pri_sum_g5_under_g1_th &&
			    dfs->pri_hold_sum[5] <= dfs->pri_sum_g5_under_g1_th)
				dfs->pri_type3_4_cond2 = 1;
		} else {
			dfs->pri_type3_4_cond1 = 1;
			dfs->pri_type3_4_cond2 = 1;
		}
		if (dfs->pri_type3_4_cond1 && dfs->pri_type3_4_cond2)
			dfs->pri_type3_4_flag = 1;
	} else {
	}

	if (dfs->print_hist_rpt) {
		dfs_pw_thd1 = (u8)odm_get_bb_reg(dm, 0x19e4, 0xff000000);
		dfs_pw_thd2 = (u8)odm_get_bb_reg(dm, 0x19e8, 0x000000ff);
		dfs_pw_thd3 = (u8)odm_get_bb_reg(dm, 0x19e8, 0x0000ff00);
		dfs_pw_thd4 = (u8)odm_get_bb_reg(dm, 0x19e8, 0x00ff0000);
		dfs_pw_thd5 = (u8)odm_get_bb_reg(dm, 0x19e8, 0xff000000);

		dfs_pri_thd1 = (u8)odm_get_bb_reg(dm, 0x19b8, 0x7F80);
		dfs_pri_thd2 = (u8)odm_get_bb_reg(dm, 0x19ec, 0x000000ff);
		dfs_pri_thd3 = (u8)odm_get_bb_reg(dm, 0x19ec, 0x0000ff00);
		dfs_pri_thd4 = (u8)odm_get_bb_reg(dm, 0x19ec, 0x00ff0000);
		dfs_pri_thd5 = (u8)odm_get_bb_reg(dm, 0x19ec, 0xff000000);

		PHYDM_DBG(dm, DBG_DFS, "\ndfs_pw_thd=%d %d %d %d %d\n",
			  dfs_pw_thd1, dfs_pw_thd2, dfs_pw_thd3,
			  dfs_pw_thd4, dfs_pw_thd5);
		PHYDM_DBG(dm, DBG_DFS, "-----pulse width hist-----\n");
		PHYDM_DBG(dm, DBG_DFS, "dfs_hist_pw=%x %x\n",
			  dfs_hist1_pw, dfs_hist2_pw);
		PHYDM_DBG(dm, DBG_DFS, "g_pw_hist = %x %x %x %x %x %x\n",
			  g_pw[0], g_pw[1], g_pw[2], g_pw[3],
			  g_pw[4], g_pw[5]);
		PHYDM_DBG(dm, DBG_DFS, "dfs_pri_thd=%d %d %d %d %d\n",
			  dfs_pri_thd1, dfs_pri_thd2, dfs_pri_thd3,
			  dfs_pri_thd4, dfs_pri_thd5);
		PHYDM_DBG(dm, DBG_DFS, "-----pulse interval hist-----\n");
		PHYDM_DBG(dm, DBG_DFS, "dfs_hist_pri=%x %x\n",
			  dfs_hist1_pri, dfs_hist2_pri);
		PHYDM_DBG(dm, DBG_DFS,
			  "g_pri_hist = %x %x %x %x %x %x, pw_flag = %d, pri_flag = %d\n",
			  g_pri[0], g_pri[1], g_pri[2], g_pri[3], g_pri[4],
			  g_pri[5], dfs->pw_flag, dfs->pri_flag);
		if (region_domain == 1 || region_domain == 3) {
			PHYDM_DBG(dm, DBG_DFS, "hist_idx= %d\n",
				  (dfs->hist_idx + 2) % 3);
		} else {
			PHYDM_DBG(dm, DBG_DFS, "hist_idx= %d\n",
				  (dfs->hist_idx + 3) % 4);
		}
		PHYDM_DBG(dm, DBG_DFS, "hist_long_idx= %d\n",
			  (dfs->hist_long_idx + 299) % 300);
		PHYDM_DBG(dm, DBG_DFS,
			  "pw_sum_g0g5 = %d, pw_sum_g1g2g3g4 = %d\n",
			  pw_sum_g0g5, pw_sum_g1g2g3g4);
		PHYDM_DBG(dm, DBG_DFS,
			  "pri_sum_g0g5 = %d, pri_sum_g1g2g3g4 = %d\n",
			  pri_sum_g0g5, pri_sum_g1g2g3g4);
		PHYDM_DBG(dm, DBG_DFS, "pw_hold_sum = %d %d %d %d %d %d\n",
			  dfs->pw_hold_sum[0], dfs->pw_hold_sum[1],
			  dfs->pw_hold_sum[2], dfs->pw_hold_sum[3],
			  dfs->pw_hold_sum[4], dfs->pw_hold_sum[5]);
		PHYDM_DBG(dm, DBG_DFS, "pri_hold_sum = %d %d %d %d %d %d\n",
			  dfs->pri_hold_sum[0], dfs->pri_hold_sum[1],
			  dfs->pri_hold_sum[2], dfs->pri_hold_sum[3],
			  dfs->pri_hold_sum[4], dfs->pri_hold_sum[5]);
		PHYDM_DBG(dm, DBG_DFS, "pw_long_hold_sum = %d %d %d %d %d %d\n",
			  dfs->pw_long_hold_sum[0], dfs->pw_long_hold_sum[1],
			  dfs->pw_long_hold_sum[2], dfs->pw_long_hold_sum[3],
			  dfs->pw_long_hold_sum[4], dfs->pw_long_hold_sum[5]);
		PHYDM_DBG(dm, DBG_DFS,
			  "pri_long_hold_sum = %d %d %d %d %d %d\n",
			  dfs->pri_long_hold_sum[0], dfs->pri_long_hold_sum[1],
			  dfs->pri_long_hold_sum[2], dfs->pri_long_hold_sum[3],
			  dfs->pri_long_hold_sum[4], dfs->pri_long_hold_sum[5]);
		PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n", dfs->idle_mode);
		PHYDM_DBG(dm, DBG_DFS, "pw_standard = %d\n", dfs->pw_std);
		PHYDM_DBG(dm, DBG_DFS, "pri_standard = %d\n", dfs->pri_std);
		PHYDM_DBG(dm, DBG_DFS, "\n");
		PHYDM_DBG(dm, DBG_DFS,
			  "pri_cond1 = %d, pri_cond2 = %d, pri_cond3 = %d, pri_cond4 = %d, pri_cond5 = %d\n",
			  dfs->pri_cond1, dfs->pri_cond2, dfs->pri_cond3,
			  dfs->pri_cond4, dfs->pri_cond5);
		PHYDM_DBG(dm, DBG_DFS,
			  "bandwidth = %d, pri_th = %d, max_pri_cnt_th = %d, safe_pri_pw_diff_th = %d\n",
			  band_width, pri_th, max_pri_cnt_th,
			  safe_pri_pw_diff_th);
	}
}
#endif
boolean phydm_dfs_hist_log(void *dm_void, u8 index)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	struct phydm_fa_struct *fa_t = &dm->false_alm_cnt;
	u8 i = 0, j = 0;
	u32 fa_cnt = 0;
	boolean hist_radar_detected = 0;

	fa_cnt = fa_t->cnt_all;

	if (dfs->det_print2)
		PHYDM_DBG(dm, DBG_DFS, "fa_cnt = %d, fa_cnt_th = %d\n", fa_cnt, dfs->fa_cnt_th);

	if (dfs->pulse_type_hist[index] == 0) {
		dfs->radar_type = 0;
		if (dfs->pw_flag && dfs->pri_flag &&
		    dfs->pri_type3_4_flag) {
			hist_radar_detected = 1;
			PHYDM_DBG(dm, DBG_DFS,
				  "Detected type %d radar signal!\n",
				  dfs->radar_type);
			if (dfs->det_print2) {
				/*PHYDM_DBG(dm, DBG_DFS,
					  "hist_idx= %d\n",
					  (dfs->hist_idx + 3) % 4);
				for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_hold = %d ",
						  dfs->pri_hold[j][i]);
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
				for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS, "pw_hold = %d ",
						  dfs->pw_hold[j][i]);
				}
					PHYDM_DBG(dm, DBG_DFS, "\n");
				}*/
				PHYDM_DBG(dm, DBG_DFS, "\n");
				PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
					  dfs->idle_mode);
				PHYDM_DBG(dm, DBG_DFS,
					  "pw_hold_sum = %d %d %d %d %d %d\n",
					  dfs->pw_hold_sum[0],
					  dfs->pw_hold_sum[1],
					  dfs->pw_hold_sum[2],
					  dfs->pw_hold_sum[3],
					  dfs->pw_hold_sum[4],
					  dfs->pw_hold_sum[5]);
				PHYDM_DBG(dm, DBG_DFS,
					  "pri_hold_sum = %d %d %d %d %d %d\n",
					  dfs->pri_hold_sum[0],
					  dfs->pri_hold_sum[1],
					  dfs->pri_hold_sum[2],
					  dfs->pri_hold_sum[3],
					  dfs->pri_hold_sum[4],
					  dfs->pri_hold_sum[5]);
			}
		} else {
		if (dfs->det_print2) {
			if (dfs->pulse_flag_hist[index] &&
			    dfs->pri_flag == 0) {
				PHYDM_DBG(dm, DBG_DFS, "pri_variation = %d\n",
					  dfs->pri_std);
				PHYDM_DBG(dm, DBG_DFS,
					  "PRI criterion is not satisfied!\n");
				if (dfs->pri_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond1 is not satisfied!\n");
				if (dfs->pri_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond2 is not satisfied!\n");
				if (dfs->pri_cond3 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond3 is not satisfied!\n");
				if (dfs->pri_cond4 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond4 is not satisfied!\n");
				if (dfs->pri_cond5 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond5 is not satisfied!\n");
				if (dfs->pri_cond6 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_cond6 is not satisfied!\n");
			}
			if (dfs->pulse_flag_hist[index] &&
			    dfs->pw_flag == 0) {
				PHYDM_DBG(dm, DBG_DFS, "pw_variation = %d\n",
					  dfs->pw_std);
				PHYDM_DBG(dm, DBG_DFS,
					  "PW criterion is not satisfied!\n");
				if (dfs->pw_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond1 is not satisfied!\n");
				if (dfs->pw_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond2 is not satisfied!\n");
				if (dfs->pw_cond3 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_cond3 is not satisfied!\n");
			}
			if (dfs->pulse_flag_hist[index] &&
			    (dfs->pri_type3_4_flag == 0)) {
				PHYDM_DBG(dm, DBG_DFS,
					  "pri_type3_4 criterion is not satisfied!\n");
				if (dfs->pri_type3_4_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_type3_4_cond1 is not satisfied!\n");
				if (dfs->pri_type3_4_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_type3_4_cond2 is not satisfied!\n");
			}
			/*PHYDM_DBG(dm, DBG_DFS, "hist_idx= %d\n",
				  (dfs->hist_idx + 3) % 4);
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++) {
					PHYDM_DBG(dm, DBG_DFS,
						  "pri_hold = %d ",
						  dfs->pri_hold[j][i]);
				}
				PHYDM_DBG(dm, DBG_DFS, "\n");
			}
			PHYDM_DBG(dm, DBG_DFS, "\n");
			for (j = 0; j < 4; j++) {
				for (i = 0; i < 6; i++)
					PHYDM_DBG(dm, DBG_DFS,
						  "pw_hold = %d ",
						  dfs->pw_hold[j][i]);
				PHYDM_DBG(dm, DBG_DFS, "\n");
			}*/
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
				  dfs->idle_mode);
			PHYDM_DBG(dm, DBG_DFS,
				  "pw_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pw_hold_sum[0], dfs->pw_hold_sum[1],
				  dfs->pw_hold_sum[2], dfs->pw_hold_sum[3],
				  dfs->pw_hold_sum[4], dfs->pw_hold_sum[5]);
			PHYDM_DBG(dm, DBG_DFS,
				  "pri_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pri_hold_sum[0], dfs->pri_hold_sum[1],
				  dfs->pri_hold_sum[2], dfs->pri_hold_sum[3],
				  dfs->pri_hold_sum[4], dfs->pri_hold_sum[5]);
		}
		}
	} else {
		dfs->radar_type = 1;
		if (dfs->det_print2) {
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
				  dfs->idle_mode);
		}
		/* @Long radar should satisfy three conditions */
		if (dfs->long_radar_flag == 1) {
			hist_radar_detected = 1;
			PHYDM_DBG(dm, DBG_DFS,
				  "Detected type %d radar signal!\n",
				  dfs->radar_type);
		} else {
			if (dfs->det_print2) {
				if (dfs->pw_long_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pw_long_cond1 is not satisfied!--\n");
				if (dfs->pw_long_cond2 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pw_long_cond2 is not satisfied!--\n");
				if (dfs->pri_long_cond1 == 0)
					PHYDM_DBG(dm, DBG_DFS,
						  "--pri_long_cond1 is not satisfied!--\n");
			}
		}
	}

	if (hist_radar_detected) {
		if (fa_cnt > dfs->fa_cnt_th) {
		hist_radar_detected = false;
		if(dfs->det_print2 || dfs->det_print_sw) {
			PHYDM_DBG(dm, DBG_DFS, "RDR is in FA ENV !!\n");
		}
	}
	}

	if (hist_radar_detected) {
		if (dfs->det_print_sw) {
			PHYDM_DBG(dm, DBG_DFS, "\n");
			PHYDM_DBG(dm, DBG_DFS, "fa_cnt = %d, fa_cnt_th = %d\n", fa_cnt, dfs->fa_cnt_th);
			PHYDM_DBG(dm, DBG_DFS, "idle_mode = %d\n",
				  dfs->idle_mode);
			PHYDM_DBG(dm, DBG_DFS,
				  "pw_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pw_hold_sum[0],
				  dfs->pw_hold_sum[1],
				  dfs->pw_hold_sum[2],
				  dfs->pw_hold_sum[3],
				  dfs->pw_hold_sum[4],
				  dfs->pw_hold_sum[5]);
			PHYDM_DBG(dm, DBG_DFS,
				  "pri_hold_sum = %d %d %d %d %d %d\n",
				  dfs->pri_hold_sum[0],
				  dfs->pri_hold_sum[1],
				  dfs->pri_hold_sum[2],
				  dfs->pri_hold_sum[3],
				  dfs->pri_hold_sum[4],
				  dfs->pri_hold_sum[5]);
		}
	}
	return hist_radar_detected;
}

boolean phydm_radar_detect(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	boolean radar_detected = false;

	if (dm->support_ic_type & ODM_IC_JGR3_SERIES) {
		dfs->igi_cur = (u8)odm_get_bb_reg(dm, R_0x1d70, 0x0000007f);
		dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0xa40, 0x00007f00);
	#if (RTL8721D_SUPPORT)
	} else if (dm->support_ic_type & (ODM_RTL8721D)) {
		dfs->st_l2h_cur = (u8)(odm_get_bb_reg(dm, R_0xf54,
						      0x0000001f) << 2);
		dfs->st_l2h_cur += (u8)odm_get_bb_reg(dm, R_0xf58, 0xc0000000);
	#endif
	} else {
		dfs->igi_cur = (u8)odm_get_bb_reg(dm, R_0xc50, 0x0000007f);
		dfs->st_l2h_cur = (u8)odm_get_bb_reg(dm, R_0x91c, 0x000000ff);
	}

	/* @dynamic pwdb calibration */
	if (dfs->igi_pre != dfs->igi_cur) {
		dfs->pwdb_th_cur = ((int)dfs->st_l2h_cur - (int)dfs->igi_cur)
				    / 2 + dfs->pwdb_scalar_factor;

		/* @limit the pwdb value to absolute lower bound 0xa */
		dfs->pwdb_th_cur = MAX_2(dfs->pwdb_th_cur, (int)dfs->pwdb_th);
		/* @limit the pwdb value to absolute upper bound 0x1f */
		dfs->pwdb_th_cur = MIN_2(dfs->pwdb_th_cur, 0x1f);

		if (dm->support_ic_type & ODM_IC_JGR3_SERIES)
			odm_set_bb_reg(dm, R_0xa50, 0x000000f0,
				       dfs->pwdb_th_cur);
		#if (RTL8721D_SUPPORT)
		else if (dm->support_ic_type & (ODM_RTL8721D))
			odm_set_bb_reg(dm, R_0xf70, 0x03c00000,
				       dfs->pwdb_th_cur);
		#endif
		else
			odm_set_bb_reg(dm, R_0x918, 0x00001f00,
				       dfs->pwdb_th_cur);
	}
	dfs->igi_pre = dfs->igi_cur;

	phydm_dfs_dynamic_setting(dm);
	#if (RTL8814A_SUPPORT || RTL8822B_SUPPORT || RTL8821C_SUPPORT)
	if (dm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C))
		phydm_dfs_histogram_radar_distinguish(dm);
	#endif
	radar_detected = phydm_radar_detect_dm_check(dm);

	if (radar_detected) {
		/*
		PHYDM_DBG(dm, DBG_DFS,
			  "Radar detect: %d\n", radar_detected);
		*/
		phydm_radar_detect_reset(dm);
		if (dfs->dbg_mode == 1) {
			PHYDM_DBG(dm, DBG_DFS,
				  "Radar is detected in DFS dbg mode!!!!!!!!!!!\n");
			radar_detected = 0;
		}
	}

	if (dfs->sw_trigger_mode) {
		radar_detected = 1;
		PHYDM_DBG(dm, DBG_DFS,
			  "Radar is detected in DFS SW trigger mode.\n");
	}

	return radar_detected;
}

void phydm_dfs_hist_dbg(void *dm_void, char input[][16], u32 *_used,
			char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	char help[] = "-h";
	u32 argv[5] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

	if ((strcmp(input[1], help) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{0} pri_hist_th = %d\n", dfs->pri_hist_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{1} pri_sum_g1_th = %d\n", dfs->pri_sum_g1_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{2} pri_sum_g5_th = %d\n", dfs->pri_sum_g5_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{3} pri_sum_g1_fcc_th = %d\n",
			 dfs->pri_sum_g1_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{4} pri_sum_g3_fcc_th = %d\n",
			 dfs->pri_sum_g3_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{5} pri_sum_safe_fcc_th = %d\n",
			 dfs->pri_sum_safe_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{6} pri_sum_type4_th = %d\n", dfs->pri_sum_type4_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{7} pri_sum_type6_th = %d\n", dfs->pri_sum_type6_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{8} pri_sum_safe_th = %d\n", dfs->pri_sum_safe_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{9} pri_sum_g5_under_g1_th = %d\n",
			 dfs->pri_sum_g5_under_g1_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{10} pri_pw_diff_th = %d\n", dfs->pri_pw_diff_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{11} pri_pw_diff_fcc_th = %d\n",
			 dfs->pri_pw_diff_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{12} pri_pw_diff_fcc_idle_th = %d\n",
			 dfs->pri_pw_diff_fcc_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{13} pri_pw_diff_w53_th = %d\n",
			 dfs->pri_pw_diff_w53_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{14} pri_type1_low_fcc_th = %d\n",
			 dfs->pri_type1_low_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{15} pri_type1_upp_fcc_th = %d\n",
			 dfs->pri_type1_upp_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{16} pri_type1_cen_fcc_th = %d\n",
			 dfs->pri_type1_cen_fcc_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{17} pw_g0_th = %d\n", dfs->pw_g0_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{18} pw_long_lower_20m_th = %d\n",
			 dfs->pw_long_lower_20m_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{19} pw_long_lower_th = %d\n",
			 dfs->pw_long_lower_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{20} pri_long_upper_th = %d\n",
			 dfs->pri_long_upper_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{21} pw_long_sum_upper_th = %d\n",
			 dfs->pw_long_sum_upper_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{22} pw_std_th = %d\n", dfs->pw_std_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{23} pw_std_idle_th = %d\n", dfs->pw_std_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{24} pri_std_th = %d\n", dfs->pri_std_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{25} pri_std_idle_th = %d\n", dfs->pri_std_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{26} type4_pw_max_cnt = %d\n", dfs->type4_pw_max_cnt);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{27} type4_safe_pri_sum_th = %d\n",
			 dfs->type4_safe_pri_sum_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{28} fa_mask_th = %d\n",
			dfs->fa_mask_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{29} nhm_cnt_th = %d\n",
			dfs->nhm_cnt_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{30} nhm_dty_th = %d\n",
			dfs->nhm_dty_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{31} pw_flag_en = %d\n",
			dfs->pw_flag_en);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{32} loct_flag_en = %d\n",
			dfs->loct_flag_en);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{33} pri_flag_en = %d\n",
			dfs->pri_flag_en);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{34} pri_var_offset = %d\n",
			dfs->pri_var_offset);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{35} pri_var_tp_offset = %d\n",
			dfs->pri_var_tp_offset);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{36} pri_cnt_th = %d\n",
			dfs->pri_cnt_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{37} loct_diff_th = %d\n",
			dfs->loct_diff_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{38} dc_cnt_th = %d\n",
			dfs->dc_cnt_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{39} pri_sum_g0_idle_th = %d\n",
			dfs->pri_sum_g0_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{40} pri_sum_g0_th = %d\n",
			dfs->pri_sum_g0_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{41} pri_sum_g5_idle_th = %d\n",
			dfs->pri_sum_g5_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{42} pw_sum_g0_idle_th = %d\n",
			dfs->pw_sum_g0_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{43} pw_sum_g0_th = %d\n",
			dfs->pw_sum_g0_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{44} pw_sum_g5_idle_th = %d\n",
			dfs->pw_sum_g5_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{45} pw_sum_g5_th = %d\n",
			dfs->pw_sum_g5_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{46} pw_pri_valid_set_idle_th = %d\n",
			dfs->pw_pri_valid_set_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{47} pw_pri_valid_set_th = %d\n",
			dfs->pw_pri_valid_set_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{48} pri_pw_diff_idle_th = %d\n",
			dfs->pri_pw_diff_idle_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			"{49} fa_cnt_th = %d\n",
			dfs->fa_cnt_th);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{50} pri_hist_idle_th = %d\n", dfs->pri_hist_idle_th);
	} else {
		PHYDM_SSCANF(input[1], DCMD_DECIMAL, &argv[0]);

		for (i = 1; i < 5; i++) {
			PHYDM_SSCANF(input[i + 1], DCMD_DECIMAL,
				     &argv[i]);
		}
		if (argv[0] == 0) {
			dfs->pri_hist_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_hist_th = %d\n",
				 dfs->pri_hist_th);
		} else if (argv[0] == 1) {
			dfs->pri_sum_g1_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g1_th = %d\n",
				 dfs->pri_sum_g1_th);
		} else if (argv[0] == 2) {
			dfs->pri_sum_g5_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g5_th = %d\n",
				 dfs->pri_sum_g5_th);
		} else if (argv[0] == 3) {
			dfs->pri_sum_g1_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g1_fcc_th = %d\n",
				 dfs->pri_sum_g1_fcc_th);
		} else if (argv[0] == 4) {
			dfs->pri_sum_g3_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g3_fcc_th = %d\n",
				 dfs->pri_sum_g3_fcc_th);
		} else if (argv[0] == 5) {
			dfs->pri_sum_safe_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_safe_fcc_th = %d\n",
				 dfs->pri_sum_safe_fcc_th);
		} else if (argv[0] == 6) {
			dfs->pri_sum_type4_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_type4_th = %d\n",
				 dfs->pri_sum_type4_th);
		} else if (argv[0] == 7) {
			dfs->pri_sum_type6_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_type6_th = %d\n",
				 dfs->pri_sum_type6_th);
		} else if (argv[0] == 8) {
			dfs->pri_sum_safe_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_safe_th = %d\n",
				 dfs->pri_sum_safe_th);
		} else if (argv[0] == 9) {
			dfs->pri_sum_g5_under_g1_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_sum_g5_under_g1_th = %d\n",
				 dfs->pri_sum_g5_under_g1_th);
		} else if (argv[0] == 10) {
			dfs->pri_pw_diff_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_th = %d\n",
				 dfs->pri_pw_diff_th);
		} else if (argv[0] == 11) {
			dfs->pri_pw_diff_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_fcc_th = %d\n",
				 dfs->pri_pw_diff_fcc_th);
		} else if (argv[0] == 12) {
			dfs->pri_pw_diff_fcc_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_fcc_idle_th = %d\n",
				 dfs->pri_pw_diff_fcc_idle_th);
		} else if (argv[0] == 13) {
			dfs->pri_pw_diff_w53_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_pw_diff_w53_th = %d\n",
				 dfs->pri_pw_diff_w53_th);
		} else if (argv[0] == 14) {
			dfs->pri_type1_low_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_low_fcc_th = %d\n",
				 dfs->pri_type1_low_fcc_th);
		} else if (argv[0] == 15) {
			dfs->pri_type1_upp_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_upp_fcc_th = %d\n",
				 dfs->pri_type1_upp_fcc_th);
		} else if (argv[0] == 16) {
			dfs->pri_type1_cen_fcc_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_type1_cen_fcc_th = %d\n",
				 dfs->pri_type1_cen_fcc_th);
		} else if (argv[0] == 17) {
			dfs->pw_g0_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_g0_th = %d\n",
				 dfs->pw_g0_th);
		} else if (argv[0] == 18) {
			dfs->pw_long_lower_20m_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_lower_20m_th = %d\n",
				 dfs->pw_long_lower_20m_th);
		} else if (argv[0] == 19) {
			dfs->pw_long_lower_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_lower_th = %d\n",
				 dfs->pw_long_lower_th);
		} else if (argv[0] == 20) {
			dfs->pri_long_upper_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_long_upper_th = %d\n",
				 dfs->pri_long_upper_th);
		} else if (argv[0] == 21) {
			dfs->pw_long_sum_upper_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_long_sum_upper_th = %d\n",
				 dfs->pw_long_sum_upper_th);
		} else if (argv[0] == 22) {
			dfs->pw_std_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_std_th = %d\n",
				 dfs->pw_std_th);
		} else if (argv[0] == 23) {
			dfs->pw_std_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pw_std_idle_th = %d\n",
				 dfs->pw_std_idle_th);
		} else if (argv[0] == 24) {
			dfs->pri_std_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_std_th = %d\n",
				 dfs->pri_std_th);
		} else if (argv[0] == 25) {
			dfs->pri_std_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "pri_std_idle_th = %d\n",
				 dfs->pri_std_idle_th);
		} else if (argv[0] == 26) {
			dfs->type4_pw_max_cnt = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "type4_pw_max_cnt = %d\n",
				 dfs->type4_pw_max_cnt);
		} else if (argv[0] == 27) {
			dfs->type4_safe_pri_sum_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "type4_safe_pri_sum_th = %d\n",
				 dfs->type4_safe_pri_sum_th);
		} else if (argv[0] == 28) {
			dfs->fa_mask_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"fa_mask_th = %d\n",
				dfs->fa_mask_th);
		} else if (argv[0] == 29) {
			dfs->nhm_cnt_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"nhm_cnt_th = %d\n",
				dfs->nhm_cnt_th);
		} else if (argv[0] == 30) {
			dfs->nhm_dty_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"nhm_dty_th = %d\n",
				dfs->nhm_dty_th);
		} else if (argv[0] == 31) {
			dfs->pw_flag_en = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_flag_en = %d\n",
				dfs->pw_flag_en);
		} else if (argv[0] == 32) {
			dfs->loct_flag_en = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"loct_flag_en = %d\n",
				dfs->loct_flag_en);
		} else if (argv[0] == 33) {
			dfs->pri_flag_en = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_flag_en = %d\n",
				dfs->pri_flag_en);
		} else if (argv[0] == 34) {
			dfs->pri_var_offset = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_var_offset = %d\n",
				dfs->pri_var_offset);
		} else if (argv[0] == 35) {
			dfs->pri_var_tp_offset = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_var_tp_offset = %d\n",
				dfs->pri_var_tp_offset);
		} else if (argv[0] == 36) {
			dfs->pri_cnt_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_cnt_th = %d\n",
				dfs->pri_cnt_th);
		} else if (argv[0] == 37) {
			dfs->loct_diff_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"loct_diff_th = %d\n",
				dfs->loct_diff_th);
		} else if (argv[0] == 38) {
			dfs->dc_cnt_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"dc_cnt_th = %d\n",
				dfs->dc_cnt_th);
		} else if (argv[0] == 39) {
			dfs->pri_sum_g0_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_sum_g0_idle_th = %d\n",
				dfs->pri_sum_g0_idle_th);
		} else if (argv[0] == 40) {
			dfs->pri_sum_g0_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_sum_g0_th = %d\n",
				dfs->pri_sum_g0_th);
		} else if (argv[0] == 41) {
			dfs->pri_sum_g5_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_sum_g5_idle_th = %d\n",
				dfs->pri_sum_g5_idle_th);
		} else if (argv[0] == 42) {
			dfs->pw_sum_g0_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_sum_g0_idle_th = %d\n",
				dfs->pw_sum_g0_idle_th);
		} else if (argv[0] == 43) {
			dfs->pw_sum_g0_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_sum_g0_th = %d\n",
				dfs->pw_sum_g0_th);
		} else if (argv[0] == 44) {
			dfs->pw_sum_g5_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_sum_g5_idle_th = %d\n",
				dfs->pw_sum_g5_idle_th);
		} else if (argv[0] == 45) {
			dfs->pw_sum_g5_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_sum_g5_th = %d\n",
				dfs->pw_sum_g5_th);
		} else if (argv[0] == 46) {
			dfs->pw_pri_valid_set_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_pri_valid_set_idle_th = %d\n",
				dfs->pw_pri_valid_set_idle_th);
		} else if (argv[0] == 47) {
			dfs->pw_pri_valid_set_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pw_pri_valid_set_th = %d\n",
				dfs->pw_pri_valid_set_th);
		} else if (argv[0] == 48) {
			dfs->pri_pw_diff_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_pw_diff_idle_th = %d\n",
				dfs->pri_pw_diff_idle_th);
		} else if (argv[0] == 49) {
			dfs->fa_cnt_th = (u32)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"fa_cnt_th = %d\n",
				dfs->fa_cnt_th);
		} else if (argv[0] == 50) {
			dfs->pri_hist_idle_th = (u8)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				"pri_hist_idle_th = %d\n",
				dfs->pri_hist_idle_th);
		}
	}
	*_used = used;
	*_out_len = out_len;
}

void phydm_dfs_debug(void *dm_void, char input[][16], u32 *_used,
		     char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;
	char help[] = "-h";
	u32 argv[5] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

	if ((strcmp(input[1], help) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{0} dbg_mode = %d\n", dfs->dbg_mode);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{1} sw_trigger_mode = %d\n", dfs->sw_trigger_mode);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{2} force_TP_mode = %d\n", dfs->force_TP_mode);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{3} det_print = %d\n", dfs->det_print);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{4} det_print2 = %d\n", dfs->det_print2);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{5} det_print_sw = %d\n", dfs->det_print_sw);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{6} print_hist_rpt = %d\n", dfs->print_hist_rpt);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{7} hist_cond_on = %d\n", dfs->hist_cond_on);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{8} det_print_jar3 = %d\n", dfs->det_print_jar3);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{9} nhm_dfs_en = %d\n", dfs->nhm_dfs_en);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "{10} det_jar3_en = %d\n", dfs->det_jar3_en);
	} else {
		PHYDM_SSCANF(input[1], DCMD_DECIMAL, &argv[0]);

		for (i = 1; i < 5; i++) {
			PHYDM_SSCANF(input[i + 1], DCMD_DECIMAL,
				     &argv[i]);
		}
		if (argv[0] == 0) {
			dfs->dbg_mode = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "dbg_mode = %d\n", dfs->dbg_mode);
		} else if (argv[0] == 1) {
			dfs->sw_trigger_mode = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "sw_trigger_mode = %d\n", dfs->sw_trigger_mode);
		} else if (argv[0] == 2) {
			dfs->force_TP_mode = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "force_TP_mode = %d\n", dfs->force_TP_mode);
		} else if (argv[0] == 3) {
			dfs->det_print = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "det_print = %d\n", dfs->det_print);
		} else if (argv[0] == 4) {
			dfs->det_print2 = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "det_print2 = %d\n", dfs->det_print2);
		} else if (argv[0] == 5) {
			dfs->det_print_sw = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "det_print_sw = %d\n", dfs->det_print_sw);
		} else if (argv[0] == 6) {
			dfs->print_hist_rpt = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "print_hist_rpt = %d\n", dfs->print_hist_rpt);
		}  else if (argv[0] == 7) {
			dfs->hist_cond_on = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "hist_cond_on = %d\n", dfs->hist_cond_on);
		}  else if (argv[0] == 8) {
			dfs->det_print_jar3 = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "det_print_jar3 = %d\n", dfs->det_print_jar3);
		}  else if (argv[0] == 9) {
			dfs->nhm_dfs_en = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "nhm_dfs_en = %d\n", dfs->nhm_dfs_en);
		}  else if (argv[0] == 10) {
			dfs->det_jar3_en = (boolean)argv[1];
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "det_jar3_en = %d\n",  dfs->det_jar3_en);
		}
	}
	*_used = used;
	*_out_len = out_len;
}

u8 phydm_dfs_polling_time(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _DFS_STATISTICS *dfs = &dm->dfs;

	if (dm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C | ODM_RTL8822C | ODM_RTL8733B | ODM_RTL8822E))
		dfs->dfs_polling_time = 40;
	else
		dfs->dfs_polling_time = 100;

	return dfs->dfs_polling_time;
}

#endif /* @defined(CONFIG_PHYDM_DFS_MASTER) */

boolean
phydm_is_dfs_band(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (((*dm->channel >= 52) && (*dm->channel <= 64)) ||
	    ((*dm->channel >= 100) && (*dm->channel <= 144)))
		return true;
	else
		return false;
}

boolean
phydm_dfs_master_enabled(void *dm_void)
{
#ifdef CONFIG_PHYDM_DFS_MASTER
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	boolean ret_val = false;

	if (dm->dfs_master_enabled) /*pointer protection*/
		ret_val = *dm->dfs_master_enabled ? true : false;

	return ret_val;
#else
	return false;
#endif
}

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#ifdef PHYDM_IC_JGR3_SERIES_SUPPORT
void phydm_dfs_ap_reset_radar_detect_counter_and_flag(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	/* @Clear Radar Counter and Radar flag */
	odm_set_bb_reg(dm, R_0xa40, BIT(15), 0);
	odm_set_bb_reg(dm, R_0xa40, BIT(15), 1);

	/* RT_TRACE(COMP_DFS, DBG_LOUD, ("[DFS], After reset radar counter, 0xcf8 = 0x%x, 0xcf4 = 0x%x\n", */
	/* PHY_QueryBBReg(Adapter, 0xcf8, bMaskDWord), */
	/* PHY_QueryBBReg(Adapter, 0xcf4, bMaskDWord))); */
}
#endif
#endif
