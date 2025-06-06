/******************************************************************************
 *
 * Copyright(c) 2013 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#include <rtw_odm.h>
#include <hal_data.h>

u32 rtw_phydm_ability_ops(_adapter *adapter, HAL_PHYDM_OPS ops, u32 ability)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter);
	struct dm_struct *podmpriv = &pHalData->odmpriv;
	u32 result = 0;

	switch (ops) {
	case HAL_PHYDM_DIS_ALL_FUNC:
		podmpriv->support_ability = DYNAMIC_FUNC_DISABLE;
		halrf_cmn_info_set(podmpriv, HALRF_CMNINFO_ABILITY, DYNAMIC_FUNC_DISABLE);
		break;
	case HAL_PHYDM_FUNC_SET:
		podmpriv->support_ability |= ability;
		break;
	case HAL_PHYDM_FUNC_CLR:
		podmpriv->support_ability &= ~(ability);
		break;
	case HAL_PHYDM_ABILITY_BK:
		/* dm flag backup*/
		podmpriv->bk_support_ability = podmpriv->support_ability;
		pHalData->bk_rf_ability = halrf_cmn_info_get(podmpriv, HALRF_CMNINFO_ABILITY);
		break;
	case HAL_PHYDM_ABILITY_RESTORE:
		/* restore dm flag */
		podmpriv->support_ability = podmpriv->bk_support_ability;
		halrf_cmn_info_set(podmpriv, HALRF_CMNINFO_ABILITY, pHalData->bk_rf_ability);
		break;
	case HAL_PHYDM_ABILITY_SET:
		podmpriv->support_ability = ability;
		break;
	case HAL_PHYDM_ABILITY_GET:
		result = podmpriv->support_ability;
		break;
	}
	return result;
}

/* set ODM_CMNINFO_IC_TYPE based on chip_type */
void rtw_odm_init_ic_type(_adapter *adapter)
{
	struct dm_struct *odm = adapter_to_phydm(adapter);
	u32 ic_type = chip_type_to_odm_ic_type(rtw_get_chip_type(adapter));

	rtw_warn_on(!ic_type);

	odm_cmn_info_init(odm, ODM_CMNINFO_IC_TYPE, ic_type);
}

static bool rtw_edcca_hal_mode_supported(struct dvobj_priv* dvobj, enum rtw_edcca_mode_t mode)
{
	return mode <= RTW_EDCCA_ADAPT;
}

void rtw_edcca_hal_update(struct dvobj_priv *dvobj)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	struct dm_struct *odm = dvobj_to_phydm(dvobj);
	u8 mode;
	BAND_TYPE band;
	u8 uch;

#ifndef CONFIG_DIRECT_EDCCA_MODE_SETTING
	if (rtw_mi_get_ch_setting_union(dvobj_get_primary_adapter(dvobj), &uch, NULL, NULL)) {
		band = uch > 14 ? BAND_ON_5G : BAND_ON_2_4G;
		rfctl->last_edcca_mode_op_band = band;
	} else if (rfctl->last_edcca_mode_op_band != BAND_MAX)
		band = rfctl->last_edcca_mode_op_band;
	else
#endif
		band = hal_data->current_band_type;

	mode = rtw_get_edcca_mode(dvobj, band);
	/*
	* may get band not existing in current channel plan
	* then edcca mode RTW_EDCCA_MODE_NUM is got
	* this is not a real problem because this band is not used for TX
	* change to RTW_EDCCA_NORM to avoid warning calltrace below
	*/
	if (mode == RTW_EDCCA_MODE_NUM)
		mode = RTW_EDCCA_NORM;

	if (!rtw_edcca_hal_mode_supported(dvobj, mode)) {
		RTW_WARN("%s %s edcca mode %s is not supported by HAL, set to %s\n", __func__
			, band_str(band), rtw_edcca_mode_str(mode), rtw_edcca_mode_str(RTW_EDCCA_NORM));
		mode = RTW_EDCCA_NORM;
	}

	rfctl->adaptivity_en = mode == RTW_EDCCA_NORM ? false : true;
	phydm_adaptivity_info_init(odm, PHYDM_ADAPINFO_CARRIER_SENSE_ENABLE, mode == RTW_EDCCA_CS ? true : false);
}

void rtw_odm_adaptivity_ver_msg(void *sel, _adapter *adapter)
{
	RTW_PRINT_SEL(sel, "ADAPTIVITY_VERSION "ADAPTIVITY_VERSION"\n");
}

void rtw_odm_adaptivity_parm_msg(void *sel, _adapter *adapter)
{
	struct dm_struct *odm = adapter_to_phydm(adapter);
	struct registry_priv *regsty = &adapter->registrypriv;

	rtw_cfg_adaptivity_config_msg(sel, adapter);

	RTW_PRINT_SEL(sel, "%10s %16s\n"
		, "th_l2h_ini", "th_edcca_hl_diff");
	RTW_PRINT_SEL(sel, "0x%-8x %-16d\n"
		, (u8)odm->th_l2h_ini
		, odm->th_edcca_hl_diff
	);
	RTW_PRINT_SEL(sel, "adaptivity_idle_probability = %u\n", regsty->adaptivity_idle_probability);
}

void rtw_odm_adaptivity_parm_set(_adapter *adapter, s8 th_l2h_ini, s8 th_edcca_hl_diff)
{
	struct dm_struct *odm = adapter_to_phydm(adapter);

	odm->th_l2h_ini = th_l2h_ini;
	odm->th_edcca_hl_diff = th_edcca_hl_diff;
}

void rtw_odm_get_perpkt_rssi(void *sel, _adapter *adapter)
{
	struct dm_struct *odm = adapter_to_phydm(adapter);

	RTW_PRINT_SEL(sel, "rx_rate = %s, rssi_a = %d(%%), rssi_b = %d(%%)\n",
		      HDATA_RATE(odm->rx_rate), odm->rssi_a, odm->rssi_b);
}


void rtw_odm_acquirespinlock(_adapter *adapter,	enum rt_spinlock_type type)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(adapter);
	_irqL irqL;

	switch (type) {
	case RT_IQK_SPINLOCK:
		_enter_critical_bh(&pHalData->IQKSpinLock, &irqL);
		break;
	default:
		break;
	}
}

void rtw_odm_releasespinlock(_adapter *adapter,	enum rt_spinlock_type type)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(adapter);
	_irqL irqL;

	switch (type) {
	case RT_IQK_SPINLOCK:
		_exit_critical_bh(&pHalData->IQKSpinLock, &irqL);
		break;
	default:
		break;
	}
}

s16 rtw_odm_get_tx_power_mbm(struct dm_struct *dm, u8 rfpath, u8 rate, u8 bw, u8 cch)
{
	return phy_get_txpwr_single_mbm(dm->adapter, rfpath, mgn_rate_to_rs(rate), rate, bw, cch, 0, 0, 0, NULL);
}

#if CONFIG_TXPWR_LIMIT
enum txpwr_lmt_reg_exc_match rtw_txpwr_hal_lmt_reg_exc_search(struct dvobj_priv* dvobj
	, const char *country, u8 domain, const char **reg_name)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));
	struct lmt_reg_exc *exc;

	exc = hal_txpwr_lmt_reg_exc_search(hal_data, country, domain);
	if (exc) {
		*reg_name = exc->reg_name;
		if ((exc->country[0] != '\0' || exc->country[1] != '\0')
			&& _rtw_memcmp(country, exc->country, 2) ==  _TRUE)
			return TXPWR_LMT_REG_EXC_MATCH_COUNTRY;
		else if (exc->domain != 0xFF && domain == exc->domain)
			return TXPWR_LMT_REG_EXC_MATCH_DOMAIN;
		rtw_warn_on(1);
	}
	return TXPWR_LMT_REG_EXC_MATCH_NONE;
}

#if CONFIG_IEEE80211_BAND_6GHZ
enum txpwr_lmt_reg_exc_match rtw_txpwr_hal_lmt_reg_exc_6g_search(struct dvobj_priv* dvobj
	, const char *country, u8 domain, const char **reg_name)
{
	return TXPWR_LMT_REG_EXC_MATCH_NONE;
}
#endif /* CONFIG_IEEE80211_BAND_6GHZ */

bool rtw_txpwr_hal_lmt_reg_search(struct dvobj_priv* dvobj, enum band_type band, const char *name)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));
	struct lmt_reg *ent = hal_txpwr_lmt_reg_get_by_name(hal_data, name);

	if (ent) {
		if (band == BAND_ON_24G && ent->lmt_2g)
			return true;
		#if CONFIG_IEEE80211_BAND_5GHZ
		if (band == BAND_ON_5G && ent->lmt_5g)
			return true;
		#endif
	}
	return false;
}

void rtw_txpwr_hal_set_current_lmt_regs_by_name(struct dvobj_priv* dvobj, char *names_of_band[], int names_len_of_band[])
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));
	enum band_type band;

	for (band = 0; band < BAND_MAX; band++)
		hal_txpwr_set_current_lmt_regs(hal_data, band, names_of_band[band], names_len_of_band[band]);
}

void rtw_txpwr_hal_get_current_lmt_regs_name(struct dvobj_priv* dvobj, char *names_of_band[], int names_len_of_band[])
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));
	enum band_type band;

	for (band = 0; band < BAND_MAX; band++)
		hal_txpwr_get_current_lmt_regs(hal_data, band, &names_of_band[band], &names_len_of_band[band]);
}
#endif

#ifdef CONFIG_DFS_MASTER
static const enum phydm_dfs_region_domain _rtw_dfs_regd_to_phydm[RTW_DFS_REGD_NUM] = {
	/* elements not listed here will get PHYDM_DFS_DOMAIN_UNKNOWN(0) */
	[RTW_DFS_REGD_NONE]	= PHYDM_DFS_DOMAIN_UNKNOWN,
	[RTW_DFS_REGD_FCC]	= PHYDM_DFS_DOMAIN_FCC,
	[RTW_DFS_REGD_MKK]	= PHYDM_DFS_DOMAIN_MKK,
	[RTW_DFS_REGD_ETSI]	= PHYDM_DFS_DOMAIN_ETSI,
};

#define rtw_dfs_regd_to_phydm(region) (((region) >= RTW_DFS_REGD_NUM) ? _rtw_dfs_regd_to_phydm[RTW_DFS_REGD_NONE] : _rtw_dfs_regd_to_phydm[(region)])

bool rtw_dfs_hal_region_supported(struct dvobj_priv* dvobj, enum rtw_dfs_regd domain)
{
	return domain == RTW_DFS_REGD_NONE
		|| rtw_dfs_regd_to_phydm(domain) != PHYDM_DFS_DOMAIN_UNKNOWN;
}

void rtw_dfs_hal_update_region(struct dvobj_priv *dvobj, u8 band_idx, enum rtw_dfs_regd domain)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));

	rtw_hal_dfs_change_domain(hal_data, band_idx, rtw_dfs_regd_to_phydm(domain));
}

void rtw_dfs_hal_radar_detect_disable(struct dvobj_priv *dvobj, u8 band_idx)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));

	rtw_hal_dfs_rd_disable(hal_data, band_idx);
}

void rtw_dfs_hal_radar_detect_enable(struct dvobj_priv *dvobj, u8 band_idx, bool cac, u32 rd_freq_hi, u32 rd_freq_lo)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));

	rtw_hal_dfs_rd_enable_with_sp_freq_range(hal_data, band_idx, cac, rd_freq_hi, rd_freq_lo);
}

void rtw_dfs_hal_set_cac_status(struct dvobj_priv *dvobj, u8 band_idx, bool cac)
{
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(dvobj_get_primary_adapter(dvobj));

	rtw_hal_dfs_rd_set_cac_status(hal_data, band_idx, cac);
}

void rtw_dfs_hal_radar_detect_result_polling(struct dvobj_priv *dvobj, u8 band_idx, u8 *radar_cch, u8 *radar_bw)
{
	struct rf_ctl_t *rfctl = dvobj_to_rfctl(dvobj);
	u32 cur_hi, cur_lo;

	*radar_cch = 0;
	*radar_bw = CHANNEL_WIDTH_MAX;

	if (!rfctl->radar_detect_enabled || rfctl->radar_detect_hwband != band_idx)
		return;

	if (rtw_chbw_to_freq_range(dvobj->oper_channel, dvobj->oper_bwmode, dvobj->oper_ch_offset
		, &cur_hi, &cur_lo) == _FALSE)
		return;

	if (!rtw_is_range_overlap(cur_hi, cur_lo
		, rfctl->radar_detect_freq_hi[band_idx], rfctl->radar_detect_freq_lo[band_idx])
		|| rtw_get_passing_time_ms(rtw_get_on_oper_ch_time(dvobj_get_primary_adapter(dvobj))) < 300
	) {
		/* offchannel, bypass radar detect */
		return;
	}

	if (phydm_radar_detect(dvobj_to_phydm(dvobj))) {
		/* TODO: PHYDM info for specific band and range */
		*radar_cch = rfctl->radar_detect_cch[band_idx];
		*radar_bw = rfctl->radar_detect_bw[band_idx];
	}
}

u8 rtw_dfs_hal_radar_detect_polling_int_ms(struct dvobj_priv *dvobj)
{
	return phydm_dfs_polling_time(dvobj_to_phydm(dvobj));
}
#endif /* CONFIG_DFS_MASTER */

void rtw_odm_parse_rx_phy_status_chinfo(union recv_frame *rframe, u8 *phys)
{
#ifndef DBG_RX_PHYSTATUS_CHINFO
#define DBG_RX_PHYSTATUS_CHINFO 0
#endif

#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	_adapter *adapter = rframe->u.hdr.adapter;
	struct dm_struct *phydm = adapter_to_phydm(adapter);
	struct rx_pkt_attrib *attrib = &rframe->u.hdr.attrib;
	u8 *wlanhdr = get_recvframe_data(rframe);

	if (phydm->support_ic_type & PHYSTS_2ND_TYPE_IC) {
		/*
		* 8723D:
		* type_0(CCK)
		*     l_rxsc
		*         is filled with primary channel SC, not real rxsc.
		*         0:LSC, 1:USC
		* type_1(OFDM)
		*     rf_mode
		*         RF bandwidth when RX
		*     l_rxsc(legacy), ht_rxsc
		*         see below RXSC N-series
		* type_2(Not used)
		*/
		/*
		* 8821C, 8822B:
		* type_0(CCK)
		*     l_rxsc
		*         is filled with primary channel SC, not real rxsc.
		*         0:LSC, 1:USC
		* type_1(OFDM)
		*     rf_mode
		*         RF bandwidth when RX
		*     l_rxsc(legacy), ht_rxsc
		*         see below RXSC AC-series
		* type_2(Not used)
		*/

		if ((*phys & 0xf) == 0) {
			struct phy_sts_rpt_jgr2_type0 *phys_t0 = (struct phy_sts_rpt_jgr2_type0 *)phys;

			if (DBG_RX_PHYSTATUS_CHINFO) {
				RTW_PRINT("phys_t%u ta="MAC_FMT" %s, %s(band:%u, ch:%u, l_rxsc:%u)\n"
					, *phys & 0xf
					, MAC_ARG(get_ta(wlanhdr))
					, is_broadcast_mac_addr(get_ra(wlanhdr)) ? "BC" : is_multicast_mac_addr(get_ra(wlanhdr)) ? "MC" : "UC"
					, HDATA_RATE(attrib->data_rate)
					, phys_t0->band, phys_t0->channel, phys_t0->rxsc
				);
			}

		} else if ((*phys & 0xf) == 1) {
			struct phy_sts_rpt_jgr2_type1 *phys_t1 = (struct phy_sts_rpt_jgr2_type1 *)phys;
			u8 rxsc = (attrib->data_rate > DESC_RATE11M && attrib->data_rate < DESC_RATEMCS0) ? phys_t1->l_rxsc : phys_t1->ht_rxsc;
			u8 pkt_cch = 0;
			u8 pkt_bw = CHANNEL_WIDTH_20;

			#if	ODM_IC_11N_SERIES_SUPPORT
			if (phydm->support_ic_type & ODM_IC_11N_SERIES) {
				/* RXSC N-series */
				#define RXSC_DUP	0
				#define RXSC_LSC	1
				#define RXSC_USC	2
				#define RXSC_40M	3

				static const s8 cch_offset_by_rxsc[4] = {0, -2, 2, 0};

				if (phys_t1->rf_mode == 0) {
					pkt_cch = phys_t1->channel;
					pkt_bw = CHANNEL_WIDTH_20;
				} else if (phys_t1->rf_mode == 1) {
					if (rxsc == RXSC_LSC || rxsc == RXSC_USC) {
						pkt_cch = phys_t1->channel + cch_offset_by_rxsc[rxsc];
						pkt_bw = CHANNEL_WIDTH_20;
					} else if (rxsc == RXSC_40M) {
						pkt_cch = phys_t1->channel;
						pkt_bw = CHANNEL_WIDTH_40;
					}
				} else
					rtw_warn_on(1);

				goto type1_end;
			}
			#endif /* ODM_IC_11N_SERIES_SUPPORT */

			#if	ODM_IC_11AC_SERIES_SUPPORT
			if (phydm->support_ic_type & ODM_IC_11AC_SERIES) {
				/* RXSC AC-series */
				#define RXSC_DUP			0 /* 0: RX from all SC of current rf_mode */

				#define RXSC_LL20M_OF_160M	8 /* 1~8: RX from 20MHz SC */
				#define RXSC_L20M_OF_160M	6
				#define RXSC_L20M_OF_80M	4
				#define RXSC_L20M_OF_40M	2
				#define RXSC_U20M_OF_40M	1
				#define RXSC_U20M_OF_80M	3
				#define RXSC_U20M_OF_160M	5
				#define RXSC_UU20M_OF_160M	7

				#define RXSC_L40M_OF_160M	12 /* 9~12: RX from 40MHz SC */
				#define RXSC_L40M_OF_80M	10
				#define RXSC_U40M_OF_80M	9
				#define RXSC_U40M_OF_160M	11

				#define RXSC_L80M_OF_160M	14 /* 13~14: RX from 80MHz SC */
				#define RXSC_U80M_OF_160M	13

				static const s8 cch_offset_by_rxsc[15] = {0, 2, -2, 6, -6, 10, -10, 14, -14, 4, -4, 12, -12, 8, -8};

				if (phys_t1->rf_mode == 0) {
					/* RF 20MHz */
					pkt_cch = phys_t1->channel;
					pkt_bw = CHANNEL_WIDTH_20;
					goto type1_end;
				}

				if (rxsc == 0) {
					/* RF and RX with same BW */
					if (attrib->data_rate >= DESC_RATEMCS0) {
						pkt_cch = phys_t1->channel;
						pkt_bw = phys_t1->rf_mode;
					}
					goto type1_end;
				}

				if ((phys_t1->rf_mode == 1 && rxsc >= 1 && rxsc <= 2) /* RF 40MHz, RX 20MHz */
					|| (phys_t1->rf_mode == 2 && rxsc >= 1 && rxsc <= 4) /* RF 80MHz, RX 20MHz */
					|| (phys_t1->rf_mode == 3 && rxsc >= 1 && rxsc <= 8) /* RF 160MHz, RX 20MHz */
				) {
					pkt_cch = phys_t1->channel + cch_offset_by_rxsc[rxsc];
					pkt_bw = CHANNEL_WIDTH_20;
				} else if ((phys_t1->rf_mode == 2 && rxsc >= 9 && rxsc <= 10) /* RF 80MHz, RX 40MHz */
					|| (phys_t1->rf_mode == 3 && rxsc >= 9 && rxsc <= 12) /* RF 160MHz, RX 40MHz */
				) {
					if (attrib->data_rate >= DESC_RATEMCS0) {
						pkt_cch = phys_t1->channel + cch_offset_by_rxsc[rxsc];
						pkt_bw = CHANNEL_WIDTH_40;
					}
				} else if ((phys_t1->rf_mode == 3 && rxsc >= 13 && rxsc <= 14) /* RF 160MHz, RX 80MHz */
				) {
					if (attrib->data_rate >= DESC_RATEMCS0) {
						pkt_cch = phys_t1->channel + cch_offset_by_rxsc[rxsc];
						pkt_bw = CHANNEL_WIDTH_80;
					}
				} else
					rtw_warn_on(1);

			}
			#endif /* ODM_IC_11AC_SERIES_SUPPORT */

type1_end:
			if (DBG_RX_PHYSTATUS_CHINFO) {
				RTW_PRINT("phys_t%u ta="MAC_FMT" %s, %s(band:%u, ch:%u, rf_mode:%u, l_rxsc:%u, ht_rxsc:%u) => %u,%u\n"
					, *phys & 0xf
					, MAC_ARG(get_ta(wlanhdr))
					, is_broadcast_mac_addr(get_ra(wlanhdr)) ? "BC" : is_multicast_mac_addr(get_ra(wlanhdr)) ? "MC" : "UC"
					, HDATA_RATE(attrib->data_rate)
					, phys_t1->band, phys_t1->channel, phys_t1->rf_mode, phys_t1->l_rxsc, phys_t1->ht_rxsc
					, pkt_cch, pkt_bw
				);
			}

			/* for now, only return cneter channel of 20MHz packet */
			if (pkt_cch && pkt_bw == CHANNEL_WIDTH_20)
				attrib->ch = pkt_cch;

		} else {
			struct phy_sts_rpt_jgr2_type2 *phys_t2 = (struct phy_sts_rpt_jgr2_type2 *)phys;

			if (DBG_RX_PHYSTATUS_CHINFO) {
				RTW_PRINT("phys_t%u ta="MAC_FMT" %s, %s(band:%u, ch:%u, l_rxsc:%u, ht_rxsc:%u)\n"
					, *phys & 0xf
					, MAC_ARG(get_ta(wlanhdr))
					, is_broadcast_mac_addr(get_ra(wlanhdr)) ? "BC" : is_multicast_mac_addr(get_ra(wlanhdr)) ? "MC" : "UC"
					, HDATA_RATE(attrib->data_rate)
					, phys_t2->band, phys_t2->channel, phys_t2->l_rxsc, phys_t2->ht_rxsc
				);
			}
		}
	}
#endif /* (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1) */

}

#if defined(CONFIG_RTL8822C) && defined(CONFIG_LPS_PG)
void
debug_DACK(
	struct dm_struct *dm
)
{
	//P_PHYDM_FUNC dm;
	//dm = &(SysMib.ODM.Phydm);
	//PIQK_OFFLOAD_PARM pIQK_info;
	//pIQK_info= &(SysMib.ODM.IQKParm);
	u8 i;
	u32 temp1, temp2, temp3;

	temp1 = odm_get_bb_reg(dm, 0x1860, bMaskDWord);
	temp2 = odm_get_bb_reg(dm, 0x4160, bMaskDWord);
	temp3 = odm_get_bb_reg(dm, 0x9b4, bMaskDWord);

	odm_set_bb_reg(dm, 0x9b4, bMaskDWord, 0xdb66db00);

	//pathA
	odm_set_bb_reg(dm, 0x1830, BIT(30), 0x0);
	odm_set_bb_reg(dm, 0x1860, 0xfc000000, 0x3c);

	RTW_INFO("path A i\n");
	//i
	for (i = 0; i < 0xf; i++) {
		odm_set_bb_reg(dm, 0x18b0, 0xf0000000, i);
		RTW_INFO("[0][0][%d] = 0x%08x\n", i, (u16)odm_get_bb_reg(dm,0x2810,0x7fc0000));
		//pIQK_info->msbk_d[0][0][i] = (u16)odm_get_bb_reg(dm,0x2810,0x7fc0000);
	}
	RTW_INFO("path A q\n");
	//q
	for (i = 0; i < 0xf; i++) {
		odm_set_bb_reg(dm, 0x18cc, 0xf0000000, i);
		RTW_INFO("[0][1][%d] = 0x%08x\n", i, (u16)odm_get_bb_reg(dm,0x283c,0x7fc0000));
		//pIQK_info->msbk_d[0][1][i] = (u16)odm_get_bb_reg(dm,0x283c,0x7fc0000);
	}
	//pathB
	odm_set_bb_reg(dm, 0x4130, BIT(30), 0x0);
	odm_set_bb_reg(dm, 0x4160, 0xfc000000, 0x3c);

	RTW_INFO("\npath B i\n");
	//i
	for (i = 0; i < 0xf; i++) {
		odm_set_bb_reg(dm, 0x41b0, 0xf0000000, i);
		RTW_INFO("[1][0][%d] = 0x%08x\n", i, (u16)odm_get_bb_reg(dm,0x4510,0x7fc0000));
		//pIQK_info->msbk_d[1][0][i] = (u16)odm_get_bb_reg(dm,0x2810,0x7fc0000);
	}
	RTW_INFO("path B q\n");
	//q
	for (i = 0; i < 0xf; i++) {
		odm_set_bb_reg(dm, 0x41cc, 0xf0000000, i);
		RTW_INFO("[1][1][%d] = 0x%08x\n", i, (u16)odm_get_bb_reg(dm,0x453c,0x7fc0000));
		//pIQK_info->msbk_d[1][1][i] = (u16)odm_get_bb_reg(dm,0x283c,0x7fc0000);
	}

	//restore to normal
	odm_set_bb_reg(dm, 0x1830, BIT(30), 0x1);
	odm_set_bb_reg(dm, 0x4130, BIT(30), 0x1);
	odm_set_bb_reg(dm, 0x1860, bMaskDWord, temp1);
	odm_set_bb_reg(dm, 0x4160, bMaskDWord, temp2);
	odm_set_bb_reg(dm, 0x9b4, bMaskDWord, temp3);


}

void
debug_IQK(
	struct dm_struct *dm,
	IN	u8 idx,
	IN	u8 path
)
{
	u8 i, ch;
	u32 tmp;
	u32 bit_mask_20_16 = BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16);

	RTW_INFO("idx = %d, path = %d\n", idx, path);

	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, 0x8 | path << 1);

	if (idx == TX_IQK) {//TXCFIR
		odm_set_bb_reg(dm, R_0x1b20, BIT(31) | BIT(30), 0x3);
	} else {//RXCFIR
		odm_set_bb_reg(dm, R_0x1b20, BIT(31) | BIT(30), 0x1);		
	}
	odm_set_bb_reg(dm, R_0x1bd4, BIT(21), 0x1);
	odm_set_bb_reg(dm, R_0x1bd4, bit_mask_20_16, 0x10);
	for (i = 0; i <= 16; i++) {
		odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0xe0000001 | i << 2);
		tmp = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);
		RTW_INFO("iqk_cfir_real[%d][%d][%d] = 0x%x\n", path, idx, i, ((tmp & 0x0fff0000) >> 16));
		//iqk_info->iqk_cfir_real[ch][path][idx][i] =
		//				(tmp & 0x0fff0000) >> 16;
		RTW_INFO("iqk_cfir_imag[%d][%d][%d] = 0x%x\n", path, idx, i, (tmp & 0x0fff));
		//iqk_info->iqk_cfir_imag[ch][path][idx][i] = tmp & 0x0fff;		
	}
	odm_set_bb_reg(dm, R_0x1b20, BIT(31) | BIT(30), 0x0);
	//odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0x0);
}

__odm_func__ void
debug_information_8822c(
	struct dm_struct *dm)
{
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u32  reg_rf18;

	if (odm_get_bb_reg(dm, R_0x1e7c, BIT(30)))
		dpk_info->is_tssi_mode = true;
	else
		dpk_info->is_tssi_mode = false;

	reg_rf18 = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, RFREG_MASK);

	dpk_info->dpk_band = (u8)((reg_rf18 & BIT(16)) >> 16); /*0/1:G/A*/
	dpk_info->dpk_ch = (u8)reg_rf18 & 0xff;
	dpk_info->dpk_bw = (u8)((reg_rf18 & 0x3000) >> 12); /*3/2/1:20/40/80*/

	RTW_INFO("[DPK] TSSI/ Band/ CH/ BW = %d / %s / %d / %s\n",
	       dpk_info->is_tssi_mode, dpk_info->dpk_band == 0 ? "2G" : "5G",
	       dpk_info->dpk_ch,
	       dpk_info->dpk_bw == 3 ? "20M" : (dpk_info->dpk_bw == 2 ? "40M" : "80M"));
}

extern void _dpk_get_coef_8822c(void *dm_void, u8 path);

__odm_func__ void
debug_reload_data_8822c(
	void *dm_void)
{	
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 path;
	u32 u32tmp;

	debug_information_8822c(dm);

	for (path = 0; path < DPK_RF_PATH_NUM_8822C; path++) {

		RTW_INFO("[DPK] Reload path: 0x%x\n", path);

		odm_set_bb_reg(dm, R_0x1b00, MASKDWORD, 0x8 | (path << 1));

		 /*txagc bnd*/
		if (dpk_info->dpk_band == 0x0)
			u32tmp = odm_get_bb_reg(dm, R_0x1b60, MASKDWORD);
		else
			u32tmp = odm_get_bb_reg(dm, R_0x1b60, MASKDWORD);

 		RTW_INFO("[DPK] txagc bnd = 0x%08x\n", u32tmp);

		u32tmp = odm_get_bb_reg(dm, R_0x1b64, MASKBYTE3);
		RTW_INFO("[DPK] dpk_txagc = 0x%08x\n", u32tmp);
		
		//debug_coef_write_8822c(dm, path, dpk_info->dpk_path_ok & BIT(path) >> path);
		_dpk_get_coef_8822c(dm, path);

		//debug_one_shot_8822c(dm, path, DPK_ON);

		odm_set_bb_reg(dm, R_0x1b00, 0x0000000f, 0xc);

		if (path == RF_PATH_A)
			u32tmp = odm_get_bb_reg(dm, R_0x1b04, 0x0fffffff);
		else 
			u32tmp = odm_get_bb_reg(dm, R_0x1b5c, 0x0fffffff);

		RTW_INFO("[DPK] dpk_gs = 0x%08x\n", u32tmp);
		
	}
}

void odm_lps_pg_debug_8822c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	debug_DACK(dm);
	debug_IQK(dm, TX_IQK, RF_PATH_A);
	debug_IQK(dm, RX_IQK, RF_PATH_A);
	debug_IQK(dm, TX_IQK, RF_PATH_B);
	debug_IQK(dm, RX_IQK, RF_PATH_B);	
	debug_reload_data_8822c(dm);
}
#endif /* defined(CONFIG_RTL8822C) && defined(CONFIG_LPS_PG) */

