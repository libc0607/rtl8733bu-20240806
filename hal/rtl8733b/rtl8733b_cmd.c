/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
#define _RTL8733B_CMD_C_

#include <hal_data.h>		/* HAL_DATA_TYPE */
#include "../hal_halmac.h"	/* HRTW_HALMAC_H2C_MAX_SIZE, CMD_ID_RSVD_PAGE and etc. */
#include "rtl8733b.h"

/*
 * Below functions are for C2H
 */
/*****************************************
 * H2C Msg format :
 *| 31 - 8		|7-5	| 4 - 0	|
 *| h2c_msg		|Class	|CMD_ID	|
 *| 31-0				|
 *| Ext msg				|
 *
 ******************************************/
s32 rtl8733b_fillh2ccmd(PADAPTER adapter, u8 id, u32 buf_len, u8 *pbuf)
{
	u8 h2c[RTW_HALMAC_H2C_MAX_SIZE] = {0};
#ifdef CONFIG_RTW_DEBUG
	u8 msg[(RTW_HALMAC_H2C_MAX_SIZE - 1) * 5 + 1] = {0};
	u8 *msg_p;
	u32 msg_size, i, n;
#endif /* CONFIG_RTW_DEBUG */
	int err;
	s32 ret = _FAIL;


	if (!pbuf)
		goto exit;

	if (buf_len > (RTW_HALMAC_H2C_MAX_SIZE - 1))
		goto exit;

	if (rtw_is_surprise_removed(adapter))
		goto exit;

#ifdef CONFIG_RTW_DEBUG
	msg_p = msg;
	msg_size = (RTW_HALMAC_H2C_MAX_SIZE - 1) * 5 + 1;
	for (i = 0; i < buf_len; i++) {
		n = rtw_sprintf(msg_p, msg_size, " 0x%02x", pbuf[i]);
		msg_p += n;
		msg_size -= n;
		if (msg_size == 0)
			break;
	}
	RTW_DBG(FUNC_ADPT_FMT ": id=0x%02x buf=%s\n",
		 FUNC_ADPT_ARG(adapter), id, msg);
#endif /* CONFIG_RTW_DEBUG */

	h2c[0] = id;
	_rtw_memcpy(h2c + 1, pbuf, buf_len);

	err = rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c);
	if (!err)
		ret = _SUCCESS;

exit:

	return ret;
}

void rtl8733b_req_txrpt_cmd(PADAPTER adapter, u8 macid)
{
	u8 h2c[RTW_HALMAC_H2C_MAX_SIZE] = {0};

	AP_REQ_TXRPT_SET_CMD_ID(h2c, CMD_ID_AP_REQ_TXRPT);
	AP_REQ_TXRPT_SET_CLASS(h2c, CLASS_AP_REQ_TXRPT);

	AP_REQ_TXRPT_SET_STA1_MACID(h2c, macid);
	AP_REQ_TXRPT_SET_STA2_MACID(h2c, 0xff);
	AP_REQ_TXRPT_SET_RTY_OK_TOTAL(h2c, 0x00);
	AP_REQ_TXRPT_SET_RTY_CNT_MACID(h2c, 0x00);
	rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c);

	AP_REQ_TXRPT_SET_RTY_CNT_MACID(h2c, 0x01);
	rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c);
}

#define SET_PWR_MODE_SET_BCN_RECEIVING_TIME(h2c_pkt, value)                    \
	SET_BITS_TO_LE_4BYTE(h2c_pkt + 0X04, 24, 5, value)
#define SET_PWR_MODE_SET_ADOPT_BCN_RECEIVING_TIME(h2c_pkt, value)              \
	SET_BITS_TO_LE_4BYTE(h2c_pkt + 0X04, 31, 1, value)

void _rtl8733b_set_FwPwrMode_cmd(PADAPTER adapter, u8 psmode, u8 rfon_ctrl)
{
	int i;
	u8 smart_ps = 0, mode = 0;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);
#ifdef CONFIG_BCN_RECV_TIME
	u8 bcn_recv_time;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
#endif
#ifdef CONFIG_WMMPS_STA
	struct mlme_priv	*pmlmepriv = &(adapter->mlmepriv);
	struct qos_priv	*pqospriv = &pmlmepriv->qospriv;
#endif /* CONFIG_WMMPS_STA */	
	u8 h2c[RTW_HALMAC_H2C_MAX_SIZE] = {0};
	u8 PowerState = 0, awake_intvl = 1, rlbm = 0;
	u8 allQueueUAPSD = 0;
	char *fw_psmode_str = "UNSPECIFIED";
#ifdef CONFIG_P2P
	struct wifidirect_info *wdinfo = &adapter->wdinfo;
#endif /* CONFIG_P2P */
	u8 hw_port = rtw_hal_get_port(adapter);

	if (pwrpriv->pwr_mode != psmode) {
		if (pwrpriv->dtim > 0)
			RTW_INFO(FUNC_ADPT_FMT ": dtim=%d, HW port id=%d\n", FUNC_ADPT_ARG(adapter),
				pwrpriv->dtim, psmode == PS_MODE_ACTIVE ? pwrpriv->current_lps_hw_port_id : hw_port);
		else
			RTW_INFO(FUNC_ADPT_FMT ": HW port id=%d\n", FUNC_ADPT_ARG(adapter),
				psmode == PS_MODE_ACTIVE ? pwrpriv->current_lps_hw_port_id : hw_port);
	}

	if (psmode == PS_MODE_MIN || psmode == PS_MODE_MAX) {
#ifdef CONFIG_WMMPS_STA	
		if (rtw_is_wmmps_mode(adapter)) {
			mode = 2;

			smart_ps = pwrpriv->wmm_smart_ps;

			/* (WMMPS) allQueueUAPSD: 0: PSPoll, 1: QosNullData (if wmm_smart_ps=1) or do nothing (if wmm_smart_ps=2) */
			if ((pqospriv->uapsd_tid & BIT_MASK_TID_TC) == ALL_TID_TC_SUPPORTED_UAPSD)
				allQueueUAPSD = 1;
		} else
#endif /* CONFIG_WMMPS_STA */
		{
			mode = 1;
#ifdef CONFIG_WMMPS_STA	
			/* For WMMPS test case, the station must retain sleep mode to capture buffered data on LPS mechanism */ 
			if ((pqospriv->uapsd_tid & BIT_MASK_TID_TC)  != 0)
				smart_ps = 0;
			else
#endif /* CONFIG_WMMPS_STA */
			{
				smart_ps = pwrpriv->smart_ps;
			}
		}

		if (psmode == PS_MODE_MIN)
			rlbm = 0;
		else
			rlbm = 1;
	} else if (psmode == PS_MODE_DTIM) {
		mode = 1;
		/* For WOWLAN LPS, DTIM = (awake_intvl - 1) */
		if (pwrpriv->dtim > 0 && pwrpriv->dtim < 16)
			/* DTIM = (awake_intvl - 1) */
			awake_intvl = pwrpriv->dtim + 1;
		else
			/* DTIM = 3 */
			awake_intvl = 4;

		rlbm = 2;
		smart_ps = pwrpriv->smart_ps;
	} else if (psmode == PS_MODE_ACTIVE) {
		mode = 0;
	} else {
		rlbm = 2;
		awake_intvl = 4;
		smart_ps = pwrpriv->smart_ps;
	}

#ifdef CONFIG_P2P
	if (!rtw_p2p_chk_state(wdinfo, P2P_STATE_NONE)) {
		awake_intvl = 2;
		rlbm = 1;
	}
#endif /* CONFIG_P2P */

	if (adapter->registrypriv.wifi_spec == 1) {
		awake_intvl = 2;
		rlbm = 1;
	}

	if (psmode > 0) {
#ifdef CONFIG_BT_COEXIST
		if (rtw_btcoex_IsBtControlLps(adapter) == _TRUE)
			PowerState = rtw_btcoex_RpwmVal(adapter);
		else
#endif /* CONFIG_BT_COEXIST */
		{
			if (rfon_ctrl == rf_on)
				PowerState = 0x04; /* AllON(0x0C), RFON(0x04), RFOFF(0x00) */
			else
				PowerState = 0x00; /* AllON(0x0C), RFON(0x04), RFOFF(0x00) */
		}
	} else
		PowerState = 0x0C; /* AllON(0x0C), RFON(0x04), RFOFF(0x00) */

	if (pwrpriv->pwr_mode != psmode) {
		if (mode == 0)
			fw_psmode_str = "ACTIVE";
		else if (mode == 1)
			fw_psmode_str = "LPS";
		else if (mode == 2)
			fw_psmode_str = "WMMPS";

		RTW_INFO(FUNC_ADPT_FMT": fw ps mode = %s, drv ps mode = %d, rlbm = %d ,"
				    "smart_ps = %d, allQueueUAPSD = %d, PowerState = %d\n",
				    FUNC_ADPT_ARG(adapter), fw_psmode_str, psmode, rlbm, smart_ps, 
				    allQueueUAPSD, PowerState);
	}

#ifdef CONFIG_LPS_1T1R
	if (psmode > PS_MODE_ACTIVE) {
		HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);

		if (hal_data->lps_1t1r != pwrpriv->lps_1t1r
			/* if rf_type already 1T1R, no need to enable LPS-1T1R */
			&& hal_data->NumTotalRFPath > 1
			/* for now, FW LPS 1T1R operates on TX:A, RX:A */
			&& GET_HAL_TX_PATH_BMP(adapter) & 0x01
			&& GET_HAL_RX_PATH_BMP(adapter) & 0x01
		) {
			/* TODO: cmd macro defined by halmac */
			#define SET_PWR_MODE_EXT_SET_1T1R_EN(h2c_pkt, value) SET_BITS_TO_LE_4BYTE(h2c_pkt + 0X00, 8, 1, value)
			u8 h2c_ext[RTW_HALMAC_H2C_MAX_SIZE] = {0};

			SET_PWR_MODE_SET_CMD_ID(h2c_ext, 0x11); /* TODO: CMD_ID defined by halmac */
			SET_PWR_MODE_SET_CLASS(h2c_ext, CLASS_SET_PWR_MODE);
			SET_PWR_MODE_EXT_SET_1T1R_EN(h2c_ext, pwrpriv->lps_1t1r);
			RTW_DBG_DUMP("H2C-PwrModeExt Parm:", h2c_ext, RTW_HALMAC_H2C_MAX_SIZE);
			if (rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c_ext) == 0)
				hal_data->lps_1t1r = pwrpriv->lps_1t1r;
		}
	}
#endif

	SET_PWR_MODE_SET_CMD_ID(h2c, CMD_ID_SET_PWR_MODE);
	SET_PWR_MODE_SET_CLASS(h2c, CLASS_SET_PWR_MODE);
	SET_PWR_MODE_SET_MODE(h2c, mode);
	SET_PWR_MODE_SET_SMART_PS(h2c, smart_ps);
	SET_PWR_MODE_SET_RLBM(h2c, rlbm);
	SET_PWR_MODE_SET_AWAKE_INTERVAL(h2c, awake_intvl);
	SET_PWR_MODE_SET_B_ALL_QUEUE_UAPSD(h2c, allQueueUAPSD);
	SET_PWR_MODE_SET_PWR_STATE(h2c, PowerState);
	if (psmode == PS_MODE_ACTIVE) {
		/* Leave LPS, set the same HW port ID */
		SET_PWR_MODE_SET_PORT_ID(h2c, pwrpriv->current_lps_hw_port_id);
	} else {
		/* Enter LPS, record HW port ID */
		SET_PWR_MODE_SET_PORT_ID(h2c, hw_port);
		pwrpriv->current_lps_hw_port_id = hw_port;
	}
#ifdef CONFIG_BCN_RECV_TIME
	if (pmlmeext->bcn_rx_time) {
		bcn_recv_time = pmlmeext->bcn_rx_time / 128; /*unit : 128 us*/
		if (pmlmeext->bcn_rx_time % 128)
			bcn_recv_time += 1;

		if (bcn_recv_time > 31)
			bcn_recv_time = 31;
		SET_PWR_MODE_SET_ADOPT_BCN_RECEIVING_TIME(h2c, 1);
		SET_PWR_MODE_SET_BCN_RECEIVING_TIME(h2c, bcn_recv_time);
	}
#endif

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_RecordPwrMode(adapter, h2c + 1, RTW_HALMAC_H2C_MAX_SIZE - 1);
#endif /* CONFIG_BT_COEXIST */

	RTW_DBG_DUMP("H2C-PwrMode Parm:", h2c, RTW_HALMAC_H2C_MAX_SIZE);
	rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c);
}

void rtl8733b_set_FwPwrMode_cmd(PADAPTER adapter, u8 psmode)
{
	return _rtl8733b_set_FwPwrMode_cmd(adapter, psmode, rf_off);
}

void rtl8733b_set_FwPwrMode_rfon_ctrl_cmd(PADAPTER adapter, u8 rfon_ctrl)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);

	return _rtl8733b_set_FwPwrMode_cmd(adapter, pwrpriv->power_mgnt, rfon_ctrl);
}

void rtl8733b_set_FwPwrModeInIPS_cmd(PADAPTER adapter, u8 cmd_param)
{

	u8 h2c[RTW_HALMAC_H2C_MAX_SIZE] = {0};

	INACTIVE_PS_SET_CMD_ID(h2c, CMD_ID_INACTIVE_PS);
	INACTIVE_PS_SET_CLASS(h2c, CLASS_INACTIVE_PS);

	if (cmd_param & BIT0)
		INACTIVE_PS_SET_ENABLE(h2c, 1);

	if (cmd_param & BIT1)
		INACTIVE_PS_SET_IGNORE_PS_CONDITION(h2c, 1);

	RTW_DBG_DUMP("H2C-FwPwrModeInIPS Parm:", h2c, RTW_HALMAC_H2C_MAX_SIZE);
	rtw_halmac_send_h2c(adapter_to_dvobj(adapter), h2c);
}

#ifdef CONFIG_WOWLAN

void rtl8733b_set_fw_pwrmode_inips_cmd_wowlan(PADAPTER padapter, u8 ps_mode)
{
	struct registry_priv  *registry_par = &padapter->registrypriv;
	u8 param[H2C_INACTIVE_PS_LEN] = {0};
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	RTW_INFO("%s, ps_mode: %d\n", __func__, ps_mode);
	if (ps_mode == PS_MODE_ACTIVE) {
		SET_H2CCMD_INACTIVE_PS_EN(param, 0);
	}
	else {
		SET_H2CCMD_INACTIVE_PS_EN(param, 1);
		if(registry_par->suspend_type == FW_IPS_DISABLE_BBRF && !check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
			SET_H2CCMD_INACTIVE_DISBBRF(param, 1);
		if(registry_par->suspend_type == FW_IPS_WRC) {
			SET_H2CCMD_INACTIVE_PERIOD_SCAN_EN(param, 1);
			SET_H2CCMD_INACTIVE_PS_FREQ(param, 3);
			SET_H2CCMD_INACTIVE_PS_DURATION(param, 1);
			SET_H2CCMD_INACTIVE_PS_PERIOD_SCAN_TIME(param, 3);
		}
	}

	rtl8733b_fillh2ccmd(padapter, H2C_INACTIVE_PS_, sizeof(param), param);
}

#endif /* CONFIG_WOWLAN */


#ifdef CONFIG_BT_COEXIST
void rtl8733b_download_BTCoex_AP_mode_rsvd_page(PADAPTER adapter)
{
	hw_var_set_dl_rsvd_page(adapter, RT_MEDIA_CONNECT);
}
#endif /* CONFIG_BT_COEXIST */

#ifdef CONFIG_P2P_PS
void rtl8733b_set_p2p_ps_offload_cmd(_adapter *padapter, u8 p2p_ps_state)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct pwrctrl_priv		*pwrpriv = adapter_to_pwrctl(padapter);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	struct P2P_PS_Offload_t	*p2p_ps_offload = (struct P2P_PS_Offload_t *)(&pHalData->p2p_ps_offload);
	u8	i;

	switch (p2p_ps_state) {
	case P2P_PS_DISABLE:
		RTW_INFO("P2P_PS_DISABLE\n");
		_rtw_memset(p2p_ps_offload, 0 , 1);
		break;
	case P2P_PS_ENABLE:
		RTW_INFO("P2P_PS_ENABLE\n");
		/* update CTWindow value. */
		if (pwdinfo->ctwindow > 0) {
			p2p_ps_offload->CTWindow_En = 1;
			rtw_write8(padapter, REG_CTWND, pwdinfo->ctwindow);
		}

		/* hw only support 2 set of NoA */
		for (i = 0 ; i < pwdinfo->noa_num ; i++) {
			/* To control the register setting for which NOA */
			rtw_write8(padapter, REG_TXCMD_NOA_SEL, (i << 4));
			if (i == 0)
				p2p_ps_offload->NoA0_En = 1;
			else
				p2p_ps_offload->NoA1_En = 1;

			/* config P2P NoA Descriptor Register */
			/* RTW_INFO("%s(): noa_duration = %x\n",__FUNCTION__,pwdinfo->noa_duration[i]); */
			rtw_write32(padapter, REG_NOA_PARAM, pwdinfo->noa_duration[i]);

			/* RTW_INFO("%s(): noa_interval = %x\n",__FUNCTION__,pwdinfo->noa_interval[i]); */
			rtw_write32(padapter, REG_NOA_PARAM_1, pwdinfo->noa_interval[i]);

			/* RTW_INFO("%s(): start_time = %x\n",__FUNCTION__,pwdinfo->noa_start_time[i]); */
			rtw_write32(padapter, REG_NOA_PARAM_2, pwdinfo->noa_start_time[i]);

			/* RTW_INFO("%s(): noa_count = %x\n",__FUNCTION__,pwdinfo->noa_count[i]); */
			rtw_write8(padapter, REG_NOA_PARAM_3, pwdinfo->noa_count[i]);
		}

		if ((pwdinfo->opp_ps == 1) || (pwdinfo->noa_num > 0)) {
			/* rst p2p circuit */
			rtw_write8(padapter, REG_DUAL_TSF_RST, BIT(4));

			p2p_ps_offload->Offload_En = 1;

			if (pwdinfo->role == P2P_ROLE_GO) {
				p2p_ps_offload->role = 1;
				p2p_ps_offload->AllStaSleep = 0;
			} else
				p2p_ps_offload->role = 0;

			p2p_ps_offload->discovery = 0;
		}
		break;
	case P2P_PS_SCAN:
		RTW_INFO("P2P_PS_SCAN\n");
		p2p_ps_offload->discovery = 1;
		break;
	case P2P_PS_SCAN_DONE:
		RTW_INFO("P2P_PS_SCAN_DONE\n");
		p2p_ps_offload->discovery = 0;
		pwdinfo->p2p_ps_state = P2P_PS_ENABLE;
		break;
	default:
		break;
	}

	rtl8733b_fillh2ccmd(padapter, SUB_CMD_ID_P2PPS, sizeof(p2p_ps_offload), (u8 *)p2p_ps_offload);
}
#endif


/*
 * Below functions are for C2H
 */
static void c2h_ccx_rpt(PADAPTER adapter, u8 *pdata)
{
#ifdef CONFIG_XMIT_ACK
	u8 tx_state;


	tx_state = CCX_RPT_GET_TX_STATE(pdata);

	/* 0 means success, 1 means retry drop */
	if (tx_state == 0)
		rtw_ack_tx_done(&adapter->xmitpriv, RTW_SCTX_DONE_SUCCESS);
	else
		rtw_ack_tx_done(&adapter->xmitpriv, RTW_SCTX_DONE_CCX_PKT_FAIL);
#endif /* CONFIG_XMIT_ACK */
}

static void
C2HTxRPTHandler_8733b(
		PADAPTER	Adapter,
		u8			*CmdBuf,
		u8			CmdLen
)
{
	_irqL	 irqL;
	u8 macid = 0, IniRate = 0;
	u16 TxOK = 0, TxFail = 0;
	struct sta_priv	*pstapriv = &(GET_PRIMARY_ADAPTER(Adapter))->stapriv, *pstapriv_original = NULL;
	u8 TxOK0 = 0, TxOK1 = 0;
	u8 TxFail0 = 0, TxFail1 = 0;
	struct sta_info *psta = NULL;
	PADAPTER	adapter_ognl = NULL;

	if(!pstapriv->gotc2h) {
		RTW_WARN("%s,%d: No gotc2h!\n", __FUNCTION__, __LINE__);
		return;
	}
	
	adapter_ognl = rtw_get_iface_by_id(GET_PRIMARY_ADAPTER(Adapter), pstapriv->c2h_adapter_id);
	if(!adapter_ognl) {
		RTW_WARN("%s: No adapter!\n", __FUNCTION__);
		return;
	}

	psta = rtw_get_stainfo(&adapter_ognl->stapriv, pstapriv->c2h_sta_mac);
	if (!psta) {
		RTW_WARN("%s: No corresponding sta_info!\n", __FUNCTION__);
		return;
	}

	macid = C2H_AP_REQ_TXRPT_GET_STA1_MACID(CmdBuf);
	TxOK0 = C2H_AP_REQ_TXRPT_GET_TX_OK1_0(CmdBuf);
	TxOK1 = C2H_AP_REQ_TXRPT_GET_TX_OK1_1(CmdBuf);
	TxOK = (TxOK1 << 8) | TxOK0;
	TxFail0 = C2H_AP_REQ_TXRPT_GET_TX_FAIL1_0(CmdBuf);
	TxFail1 = C2H_AP_REQ_TXRPT_GET_TX_FAIL1_1(CmdBuf);
	TxFail = (TxFail1 << 8) | TxFail0;
	IniRate = C2H_AP_REQ_TXRPT_GET_INITIAL_RATE1(CmdBuf);

	psta->sta_stats.tx_ok_cnt = TxOK;
	psta->sta_stats.tx_fail_cnt = TxFail;
	psta->sta_stats.tx_fail_cnt_sum += TxFail;
}

static void
C2HSPC_STAT_8733b(
		PADAPTER	Adapter,
		u8			*CmdBuf,
		u8			CmdLen
)
{
	_irqL	 irqL;
	struct sta_priv *pstapriv = &(GET_PRIMARY_ADAPTER(Adapter))->stapriv;
	struct sta_info *psta = NULL;
	struct sta_info *pbcmc_stainfo = rtw_get_bcmc_stainfo(Adapter);
	_list	*plist, *phead;
	u8 idx = C2H_SPECIAL_STATISTICS_GET_STATISTICS_IDX(CmdBuf);
	PADAPTER	adapter_ognl = NULL;

	if(!pstapriv->gotc2h) {
		RTW_WARN("%s, %d: No gotc2h!\n", __FUNCTION__, __LINE__);
		return;
	}
	
	adapter_ognl = rtw_get_iface_by_id(GET_PRIMARY_ADAPTER(Adapter), pstapriv->c2h_adapter_id);
	if(!adapter_ognl) {
		RTW_WARN("%s: No adapter!\n", __FUNCTION__);
		return;
	}

	psta = rtw_get_stainfo(&adapter_ognl->stapriv, pstapriv->c2h_sta_mac);
	if (!psta) {
		RTW_WARN("%s: No corresponding sta_info!\n", __FUNCTION__);
		return;
	}
	psta->sta_stats.tx_retry_cnt = (C2H_SPECIAL_STATISTICS_GET_DATA3(CmdBuf) << 8) | C2H_SPECIAL_STATISTICS_GET_DATA2(CmdBuf);
	psta->sta_stats.tx_retry_cnt_sum += psta->sta_stats.tx_retry_cnt;

	enter_critical_bh(&pstapriv->tx_rpt_lock);
	rtw_sctx_done(&pstapriv->gotc2h);
	exit_critical_bh(&pstapriv->tx_rpt_lock);
}
#ifdef CONFIG_FW_HANDLE_TXBCN
#define C2H_SUB_CMD_ID_FW_TBTT_RPT  0X23
#define TBTT_RPT_GET_SN(c2h_pkt)	LE_BITS_TO_4BYTE(c2h_pkt + 0X01, 0, 8)
#define TBTT_RPT_GET_PORT_ID(c2h_pkt)	LE_BITS_TO_4BYTE(c2h_pkt + 0X04, 0, 8)

#define TBTT_ROOT	0x00
#define TBTT_VAP1	0x10
#define TBTT_VAP2	0x20
#define TBTT_VAP3	0x30

static void c2h_tbtt_rpt(PADAPTER adapter, u8 *pdata)
{
	u8 ap_id, c2h_sn;

	ap_id = TBTT_RPT_GET_PORT_ID(pdata);
	c2h_sn = TBTT_RPT_GET_SN(pdata);
#ifdef DBG_FW_TBTT_RPT
	if (ap_id == TBTT_ROOT)
		RTW_INFO("== TBTT ROOT SN:%d==\n", c2h_sn);
	else if (ap_id == TBTT_VAP1)
		RTW_INFO("== TBTT_VAP1 SN:%d==\n", c2h_sn);
	else if (ap_id == TBTT_VAP2)
		RTW_INFO("== TBTT_VAP2 SN:%d==\n", c2h_sn);
	else if (ap_id == TBTT_VAP3)
		RTW_INFO("== TBTT_VAP3 SN:%d==\n", c2h_sn);
	else
		RTW_ERR("TBTT RPT INFO ERROR\n");
#endif
}
#endif

/**
 * c2h = RXDESC + c2h packet
 * size = RXDESC_SIZE + c2h packet size
 * c2h payload = c2h packet revmoe id & seq
 */
static void process_c2h_event(PADAPTER adapter, u8 *c2h, u32 size)
{
	struct mlme_ext_priv *pmlmeext;
	struct mlme_ext_info *pmlmeinfo;
	u32 desc_size;
	u8 id, seq;
	u8 c2h_len, c2h_payload_len;
	u8 *pc2h_data, *pc2h_payload;


	if (!c2h) {
		RTW_INFO("%s: c2h buffer is NULL!!\n", __FUNCTION__);
		return;
	}

	desc_size = rtl8733b_get_rx_desc_size(adapter);

	if (size < desc_size) {
		RTW_INFO("%s: c2h length(%d) is smaller than RXDESC_SIZE(%d)!!\n",
			 __FUNCTION__, size, desc_size);
		return;
	}

	pmlmeext = &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	/* shift rx desc len */
	pc2h_data = c2h + desc_size;
	c2h_len = size - desc_size;

	if (c2h_len >= 4) {
		id = C2H_GET_CMD_ID(pc2h_data);
		seq = C2H_GET_SEQ(pc2h_data);
	} else {
		id = C2H_GET_CMD_ID_1BYTE(pc2h_data);
		seq = C2H_GET_SEQ_1BYTE(pc2h_data);
	}

	/* shift 2 byte to remove cmd id & seq */
	pc2h_payload = pc2h_data + 2;
	c2h_payload_len = c2h_len - 2;

	switch (id) {
#ifdef CONFIG_BEAMFORMING
	case CMD_ID_C2H_SND_TXBF:
		RTW_INFO("%s: [CMD_ID_C2H_SND_TXBF] len=%d\n", __FUNCTION__, c2h_payload_len);
		rtw_bf_c2h_handler(adapter, id, pc2h_data, c2h_len);
		break;
#endif /* CONFIG_BEAMFORMING */

	case CMD_ID_C2H_AP_REQ_TXRPT:
		/*RTW_INFO("[C2H], C2H_AP_REQ_TXRPT!!\n");*/
		C2HTxRPTHandler_8733b(adapter, pc2h_data, c2h_len);
		break;

	case CMD_ID_C2H_SPECIAL_STATISTICS:
		/*RTW_INFO("[C2H], C2H_SPC_STAT!!\n");*/
		C2HSPC_STAT_8733b(adapter, pc2h_data, c2h_len);
		break;

	case CMD_ID_C2H_CUR_CHANNEL:
	{
		PHAL_DATA_TYPE hal = GET_HAL_DATA(adapter);
		struct submit_ctx *chsw_sctx = &hal->chsw_sctx;

		/* RTW_INFO("[C2H], CMD_ID_C2H_CUR_CHANNEL!!\n"); */
		rtw_sctx_done(&chsw_sctx);
		break;
	}

	case C2H_EXTEND:
		if (C2H_HDR_GET_C2H_SUB_CMD_ID(pc2h_data) == C2H_SUB_CMD_ID_CCX_RPT) {
			/* Shift C2H HDR 4 bytes */
			c2h_ccx_rpt(adapter, pc2h_data);
			break;
		}
#ifdef CONFIG_FW_HANDLE_TXBCN
		else if (C2H_HDR_GET_C2H_SUB_CMD_ID(pc2h_data) == C2H_SUB_CMD_ID_FW_TBTT_RPT) {
			c2h_tbtt_rpt(adapter, pc2h_data);
			break;
		}
#endif
		/* indicate c2h pkt + rx desc to halmac */
		rtw_halmac_c2h_handle(adapter_to_dvobj(adapter), c2h, size);
		break;

	/* others for c2h common code */
	default:
		c2h_handler(adapter, id, seq, c2h_payload_len, pc2h_payload);
		break;
	}
}

void rtl8733b_c2h_handler(PADAPTER adapter, u8 *pbuf, u16 length)
{
#ifdef CONFIG_WOWLAN
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);


	if (pwrpriv->wowlan_mode == _TRUE) {
#ifdef CONFIG_RTW_DEBUG
		u32 desc_size;

		desc_size = rtl8733b_get_rx_desc_size(adapter);
		RTW_INFO("%s: return because wowolan_mode==TRUE! CMDID=%d\n",
			 __FUNCTION__, C2H_GET_CMD_ID(pbuf + desc_size));
#endif /* CONFIG_RTW_DEBUG */
		return;
	}
#endif /* CONFIG_WOWLAN*/

	process_c2h_event(adapter, pbuf, length);
}

/**
 * pbuf = RXDESC + c2h packet
 * length = RXDESC_SIZE + c2h packet size
 */
void rtl8733b_c2h_handler_no_io(PADAPTER adapter, u8 *pbuf, u16 length)
{
	u32 desc_size;
	u8 id, seq;
	u8 *pc2h_content;
	u8 res;


	if ((length == 0) || (!pbuf))
		return;

	desc_size = rtl8733b_get_rx_desc_size(adapter);

	/* shift rx desc len to get c2h packet content */
	pc2h_content = pbuf + desc_size;
	id = C2H_GET_CMD_ID(pc2h_content);
	seq = C2H_GET_SEQ(pc2h_content);

	RTW_DBG("%s: C2H, ID=%d seq=%d len=%d\n",
		 __FUNCTION__, id, seq, length);

	switch (id) {
	case CMD_ID_C2H_SND_TXBF:
	case CMD_ID_C2H_CCX_RPT:
	case C2H_BT_MP_INFO:
	case C2H_FW_CHNL_SWITCH_COMPLETE:
	case C2H_IQK_FINISH:
	case C2H_MCC:
	case C2H_BCN_EARLY_RPT:
	case C2H_TX_PAUSE_RPT:
	case C2H_LPS_STATUS_RPT:	
	case C2H_EXTEND:
		/* no I/O, process directly */
		process_c2h_event(adapter, pbuf, length);
		break;

	default:
		/* Others may need I/O, run in command thread */
		res = rtw_c2h_packet_wk_cmd(adapter, pbuf, length);
		if (res == _FAIL)
			RTW_ERR("%s: C2H(%d) enqueue FAIL!\n", __FUNCTION__, id);
		break;
	}
}
